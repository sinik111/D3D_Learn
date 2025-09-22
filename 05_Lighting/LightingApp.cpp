#include "LightingApp.h"

#include <cmath>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <directxtk/WICTextureLoader.h>
#include <directxtk/DDSTextureLoader.h>

#include "../Common/MyTime.h"

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct Vertex
{
	Vector3 position;
	Vector3 normal;
	Vector2 tex;
};

struct ConstantBuffer
{
	Matrix world;
	Matrix view;
	Matrix projection;
	Matrix normalMatrix;

	Vector4 lightDirection;
	Vector4 lightColor;
};

void LightingApp::Initialize()
{
	m_width = 1280;
	m_height = 720;

	WinApp::Initialize();

	InitializeImGui();

	InitializeScene();
}

void LightingApp::OnUpdate()
{
	m_world =
		Matrix::CreateScale(m_scale) *
		Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_rotation.x)) *
		Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rotation.y)) *
		Matrix::CreateTranslation(m_position);

	m_lightRotationMatrix =
		Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_lightRotation.x)) *
		Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_lightRotation.y));

	m_lightDirection = DirectX::XMVector2TransformNormal(m_originalLightDir, m_lightRotationMatrix);
}

bool m_show_demo_window = true;

void LightingApp::OnRender()
{
	// view, projection setting

	m_camera.GetViewMatrix(m_view);

	m_projection = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(m_camera.GetFOV()),
		(float)m_width / m_height,
		m_camera.GetNear(),
		m_camera.GetFar());

	m_graphicsDevice.BeginDraw({ 0.5f, 0.7f, 0.9f, 1.0f });

	auto deviceContext = m_graphicsDevice.GetDeviceContext();

	// common
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
	deviceContext->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

	ConstantBuffer cb{};
	cb.view = m_view.Transpose();
	cb.projection = m_projection.Transpose();
	cb.lightDirection = m_lightDirection;
	cb.lightColor = m_lightColor;


	// crystal
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	deviceContext->IASetInputLayout(m_inputLayout.Get());
	deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
	deviceContext->PSSetShaderResources(0, 1, m_crystalTextureRV.GetAddressOf());

	cb.world = m_world.Transpose();
	// 역행렬 -> 전치행렬(비균등 스케일일때 Normal을 표면과 수직으로 만들기 위함)
	// -> 전치행렬(gpu의 column-major 행렬 연산에 맞추기 위함)
	cb.normalMatrix = m_world.Invert().Transpose().Transpose();

	deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
	deviceContext->DrawIndexed(m_indexCount, 0, 0);




	// skybox
	deviceContext->IASetVertexBuffers(0, 1, m_skyboxVertexBuffer.GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_skyboxIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	deviceContext->IASetInputLayout(m_skyboxInputLayout.Get());
	deviceContext->VSSetShader(m_skyboxVertexShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_skyboxPixelShader.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
	deviceContext->PSSetShaderResources(1, 1, m_skyboxTextureRV.GetAddressOf());
	deviceContext->RSSetState(m_rasterizerState.Get());
	deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	Matrix skyboxWorld = Matrix::CreateTranslation(m_camera.GetPosition());
	cb.world = skyboxWorld.Transpose();

	deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
	deviceContext->DrawIndexed(m_skyboxIndexCount, 0, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->OMSetDepthStencilState(nullptr, 0);

	RenderImGui();

	m_graphicsDevice.EndDraw();
}

void LightingApp::OnShutdown()
{
	ShutdownImGui();
}

void LightingApp::RenderImGui()
{
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	ImGui::Begin("Controller");

	ImGui::SeparatorText("Camera");

	Vector3 cameraPosition = m_camera.GetPosition();
	float cameraPositionBuffer[3]{ cameraPosition.x, cameraPosition.y, cameraPosition.z };

	if (ImGui::DragFloat3("Position##1", cameraPositionBuffer, 0.1f))
	{
		m_camera.SetPosition({ cameraPositionBuffer[0], cameraPositionBuffer[1], cameraPositionBuffer[2] });
	}

	Vector3 cameraRotation = m_camera.GetRotation();
	float cameraRotationBuffer[2]{ DirectX::XMConvertToDegrees(cameraRotation.y), DirectX::XMConvertToDegrees(cameraRotation.x) };

	if (ImGui::DragFloat2("Yaw Pitch##1", cameraRotationBuffer))
	{
		m_camera.SetRotation({ DirectX::XMConvertToRadians(cameraRotationBuffer[1]), DirectX::XMConvertToRadians(cameraRotationBuffer[0]), cameraRotation.z });
	}

	if (ImGui::Button("Reset##1"))
	{
		m_camera.SetPosition({ 0.0f, 0.0f, -10.0f });
		m_camera.SetRotation({ 0.0f, 0.0f, 0.0f });
	}

	float cameraFov = m_camera.GetFOV();

	if (ImGui::DragFloat("FOV", &cameraFov, 0.5f, 1.0f, 120.0f))
	{
		m_camera.SetFOV(cameraFov);
	}

	if (ImGui::Button("Reset##2"))
	{
		m_camera.SetFOV(50.0f);
	}

	float cameraNear = m_camera.GetNear();
	float cameraFar = m_camera.GetFar();

	if (ImGui::DragFloat("Near", &cameraNear, 1.0f, 0.01f, cameraFar - 10) ||
		ImGui::DragFloat("Far", &cameraFar, 1.0f, cameraNear + 10, 1000.f))
	{
		m_camera.SetNear(cameraNear);
		m_camera.SetFar(cameraFar);
	}

	if (ImGui::Button("Reset##3"))
	{
		m_camera.SetNear(0.01f);
		m_camera.SetFar(100.0f);
	}

	ImGui::NewLine();

	ImGui::SeparatorText("Object");

	float objectScaleBuffer[3]{ m_scale.x, m_scale.y, m_scale.z };

	if (ImGui::DragFloat3("Scale", objectScaleBuffer, 0.1f))
	{
		m_scale = { objectScaleBuffer[0], objectScaleBuffer[1], objectScaleBuffer[2] };
	}
	
	if (ImGui::Button("Reset##4"))
	{
		m_scale = { 3.0f, 6.0f, 3.0f };
	}

	float objectRotationBuffer[2]{ m_rotation.x, m_rotation.y };

	if (ImGui::DragFloat2("Rotation(x, y)##1", objectRotationBuffer, 0.1f))
	{
		m_rotation = { objectRotationBuffer[0], objectRotationBuffer[1], m_rotation.z };
	}
	
	if (ImGui::Button("Reset##5"))
	{
		m_rotation = { 30.0f, 60.0f, 0.0f };
	}

	float positionBuffer[3]{ m_position.x, m_position.y, m_position.z };

	if (ImGui::DragFloat3("Position##2", positionBuffer, 0.1f))
	{
		m_position = { positionBuffer[0], positionBuffer[1], positionBuffer[2] };
	}

	if (ImGui::Button("Reset##6"))
	{
		m_position = { 0.0f, 0.0f, 0.0f };
	}

	ImGui::NewLine();

	ImGui::SeparatorText("Light");

	float lightRotationBuffer[2]{ m_lightRotation.x, m_lightRotation.y };
	if (ImGui::DragFloat2("Rotation(x, y)##2", lightRotationBuffer, 0.5f))
	{
		m_lightRotation = { lightRotationBuffer[0], lightRotationBuffer[1], m_lightRotation.z };
	}

	float lightDirectionBuffer[3]{ m_lightDirection.x, m_lightDirection.y, m_lightDirection.z };
	ImGui::InputFloat3("Direction", lightDirectionBuffer);

	if (ImGui::Button("Reset##7"))
	{
		m_lightRotation = { -35.0f, 145.0f, 0.0f };
	}

	float lightColorBuffer[3]{ m_lightColor.x, m_lightColor.y, m_lightColor.z };
	if (ImGui::ColorEdit3("Color", lightColorBuffer))
	{
		m_lightColor = { lightColorBuffer[0], lightColorBuffer[1], lightColorBuffer[2], 1.0f };
	}

	if (ImGui::Button("Reset##8"))
	{
		m_lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	ImGui::End();

	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void LightingApp::InitializeImGui()
{
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_graphicsDevice.GetDevice().Get(), m_graphicsDevice.GetDeviceContext().Get());
}

void LightingApp::InitializeScene()
{
	auto device = m_graphicsDevice.GetDevice();

	m_vertexBufferStride = sizeof(Vertex);

	CreateCrystal();
	CreateSkyBox();
	

	// constant buffer
	D3D11_BUFFER_DESC constantBufferDesc{};
	constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer);

	// sampler
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &m_samplerState);

	m_world = Matrix::Identity;

	m_camera.GetViewMatrix(m_view);

	m_projection = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(m_camera.GetFOV()),
		(float)m_width / m_height,
		m_camera.GetNear(),
		m_camera.GetFar());
}

void LightingApp::CreateCrystal()
{
	auto device = m_graphicsDevice.GetDevice();

	const Vector3 top{ 0.0f, 0.5f, 0.0f };
	const Vector3 bottom{ 0.0f, -0.5f, 0.0f };
	const float PI_ONE_SIXTH = DirectX::XM_2PI / 6;
	const int HEXAGON_POINTS_COUNT = 6;
	const int FACES_COUNT = 12;

	Vector3 hexagonPoints[HEXAGON_POINTS_COUNT];

	for (int i = 0; i < HEXAGON_POINTS_COUNT; ++i)
	{
		hexagonPoints[i] = Vector3{ std::cos(PI_ONE_SIXTH * i) * 0.5f, 0.0f, std::sin(PI_ONE_SIXTH * i) * 0.5f };
	}

	Vector2 texUVs[14]{
		{ 0.279f, 1.0f - 0.678f }, // top

		{ 0.279f, 1.0f - 0.985f }, // upper right // index 1
		{ 0.013f, 1.0f - 0.832f }, // upper right up
		{ 0.013f, 1.0f - 0.525f }, // upper left up
		{ 0.279f, 1.0f - 0.371f }, // upper left
		{ 0.545f, 1.0f - 0.525f }, // upper left down
		{ 0.545f, 1.0f - 0.832f }, // upper right down

		{ 0.457f, 1.0f - 0.466f }, // lower right // index 7
		{ 0.723f, 1.0f - 0.620f }, // lower right up
		{ 0.989f, 1.0f - 0.466f }, // lower left up
		{ 0.989f, 1.0f - 0.159f }, // lower left
		{ 0.723f, 1.0f - 0.005f }, // lower left down
		{ 0.457f, 1.0f - 0.159f }, // lower right down

		{ 0.723f, 1.0f - 0.313f } // bottom
	};

	Vector3 crystalFaceNormals[FACES_COUNT];

	for (int i = 0; i < FACES_COUNT; ++i)
	{
		Vector3 first{ i < 6 ? hexagonPoints[(i + 1) % 6] - top : hexagonPoints[i % 6] - bottom };
		Vector3 second{ i < 6 ? hexagonPoints[i] - top : hexagonPoints[(i + 1) % 6] - bottom };

		Vector3 normal = DirectX::XMVector3Cross(first, second);
		normal.Normalize();

		crystalFaceNormals[i] = normal;
	}

	Vertex vertices[FACES_COUNT * 3]{};

	WORD indices[FACES_COUNT * 3]{};

	for (int i = 0; i < FACES_COUNT * 3; i += 3)
	{
		vertices[i] = {
			i / 3 < 6 ? top : bottom,
			crystalFaceNormals[i / 3],
			i / 3 < 6 ? texUVs[0] : texUVs[13]
		};

		vertices[i + 1] = {
			i / 3 < 6 ? hexagonPoints[(i / 3 + 1) % 6] : hexagonPoints[(i / 3) % 6],
			crystalFaceNormals[i / 3],
			i / 3 < 6 ? texUVs[(i / 3 + 1) % 6 + 1] : texUVs[(i / 3) % 6 + 7]
		};

		vertices[i + 2] = {
			i / 3 < 6 ? hexagonPoints[(i / 3) % 6] : hexagonPoints[(i / 3 + 1) % 6],
			crystalFaceNormals[i / 3],
			i / 3 < 6 ? texUVs[(i / 3) % 6 + 1] : texUVs[(i / 3 + 1) % 6 + 7]
		};

		indices[i] = i;
		indices[i + 1] = i + 1;
		indices[i + 2] = i + 2;
	}

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vertexBufferData{};
	vertexBufferData.pSysMem = vertices;

	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer);

	D3D11_INPUT_ELEMENT_DESC layout[]{
		// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ComPtr<ID3DBlob> vertexShaderBuffer;
	GraphicsDevice::CompileShaderFromFile(L"LightingVertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer);

	device->CreateInputLayout(
		layout,
		ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		&m_inputLayout
	);

	device->CreateVertexShader(
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		nullptr,
		&m_vertexShader
	);

	m_indexCount = ARRAYSIZE(indices);

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth = sizeof(WORD) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA indexBufferData{};
	indexBufferData.pSysMem = indices;
	device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer);


	ComPtr<ID3DBlob> pixelShaderBuffer;
	GraphicsDevice::CompileShaderFromFile(L"LightingPixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer);

	device->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(),
		nullptr,
		&m_pixelShader
	);

	// texture
	DirectX::CreateWICTextureFromFile(device.Get(), L"crystal_texture.png", nullptr, &m_crystalTextureRV);
}

void LightingApp::CreateSkyBox()
{
	auto device = m_graphicsDevice.GetDevice();

	// Cube

	Vertex vertices[] =
	{
		{ Vector3{ -0.5f, 0.5f, -0.5f } },
		{ Vector3{ 0.5f, 0.5f, -0.5f } },
		{ Vector3{ -0.5f, -0.5f, -0.5f } },
		{ Vector3{ 0.5f, -0.5f, -0.5f } },
		{ Vector3{ 0.5f, 0.5f, 0.5f } },
		{ Vector3{ -0.5f, 0.5f, 0.5f } },
		{ Vector3{ 0.5f, -0.5f, 0.5f } },
		{ Vector3{ -0.5f, -0.5f, 0.5f } },
	};

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vertexBufferData{};
	vertexBufferData.pSysMem = vertices;

	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_skyboxVertexBuffer);


	D3D11_INPUT_ELEMENT_DESC layout[]{
		// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ComPtr<ID3DBlob> vertexShaderBuffer;
	GraphicsDevice::CompileShaderFromFile(L"SkyboxVertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer);

	device->CreateInputLayout(
		layout,
		ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		&m_skyboxInputLayout
	);

	device->CreateVertexShader(
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		nullptr,
		&m_skyboxVertexShader
	);

	// Cube

	WORD indices[] =
	{
		0, 1, 2,
		2, 1, 3,
		1, 4, 3,
		3, 4, 6,
		4, 5, 6,
		6, 5, 7,
		5, 0, 7,
		7, 0, 2,
		5, 4, 0,
		0, 4, 1,
		2, 3, 7,
		7, 3, 6
	};

	m_skyboxIndexCount = ARRAYSIZE(indices);

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth = sizeof(WORD) * m_skyboxIndexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA indexBufferData{};
	indexBufferData.pSysMem = indices;
	device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_skyboxIndexBuffer);


	ComPtr<ID3DBlob> pixelShaderBuffer;
	GraphicsDevice::CompileShaderFromFile(L"SkyboxPixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer);

	device->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(),
		nullptr,
		&m_skyboxPixelShader
	);

	// texture
	DirectX::CreateDDSTextureFromFile(device.Get(), L"cloudy_skybox.dds", nullptr, &m_skyboxTextureRV);


	// Rasterizer state - CW 에서 CCW 로 전환해주기 위함
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.FrontCounterClockwise = true;

	device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);


	// DepthStencil State - skybox를 가장 깊이 그리고 깊이 갚을 기록을 안하기 위함
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
}

void LightingApp::ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

LRESULT LightingApp::MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	return WinApp::MessageProc(hWnd, uMsg, wParam, lParam);
}