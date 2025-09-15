#include "LightingApp.h"

#include <cmath>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "../Common/MyTime.h"

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct Vertex
{
	Vector3 position;
	Vector3 normal;
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

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	deviceContext->IASetInputLayout(m_inputLayout.Get());
	deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
	deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	deviceContext->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

	ConstantBuffer cb{};
	cb.world = m_world.Transpose();
	cb.view = m_view.Transpose();
	cb.projection = m_projection.Transpose();

	// 역행렬 -> 전치행렬(비균등 스케일일때 Normal을 표면과 수직으로 만들기 위함)
	// -> 전치행렬(gpu의 column-major 행렬 연산에 맞추기 위함)
	cb.normalMatrix = m_world.Invert().Transpose().Transpose();
	
	cb.lightDirection = DirectX::XMVector2TransformNormal(m_originalLightDir, m_lightRotationMatrix);
	cb.lightColor = m_lightColor;

	deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

	deviceContext->DrawIndexed(m_indexCount, 0, 0);

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

	ImGui::PushID(0); // 0
	ImGui::Text("Camera");

	ImGui::PushID(1); // 1
	Vector3 cameraPosition = m_camera.GetPosition();
	float cameraPositionBuffer[3]{ cameraPosition.x, cameraPosition.y, cameraPosition.z };

	ImGui::DragFloat3("Position", cameraPositionBuffer, 0.1f);

	m_camera.SetPosition({ cameraPositionBuffer[0], cameraPositionBuffer[1], cameraPositionBuffer[2] });


	if (ImGui::Button("Reset"))
	{
		m_camera.SetPosition({ 0.0f, 0.0f, -10.0f });
	}

	ImGui::PopID(); // 1

	ImGui::PushID(2); // 2
	float cameraFov = m_camera.GetFOV();

	ImGui::DragFloat("FOV", &cameraFov, 0.5f, 1.0f, 120.0f);

	m_camera.SetFOV(cameraFov);


	if (ImGui::Button("Reset"))
	{
		m_camera.SetFOV(50.0f);
	}

	ImGui::PopID(); // 2

	ImGui::PushID(3); // 3
	float cameraNear = m_camera.GetNear();
	float cameraFar = m_camera.GetFar();

	ImGui::DragFloat("Near", &cameraNear, 1.0f, 0.01f, cameraFar - 10);
	ImGui::DragFloat("Far", &cameraFar, 1.0f, cameraNear + 10, 1000.f);

	m_camera.SetNear(cameraNear);
	m_camera.SetFar(cameraFar);


	if (ImGui::Button("Reset"))
	{
		m_camera.SetNear(0.01f);
		m_camera.SetFar(100.0f);
	}

	ImGui::PopID(); // 3

	ImGui::PopID(); // 0

	ImGui::NewLine();

	ImGui::PushID(4); // 4
	ImGui::Text("Object");

	ImGui::PushID(7); // 7
	float objectScaleBuffer[3]{ m_scale.x, m_scale.y, m_scale.z };

	ImGui::DragFloat3("Scale", objectScaleBuffer, 0.1f);

	m_scale = { objectScaleBuffer[0], objectScaleBuffer[1], objectScaleBuffer[2] };

	if (ImGui::Button("Reset"))
	{
		m_scale = { 3.0f, 6.0f, 3.0f };
	}

	ImGui::PopID(); // 7

	ImGui::PushID(6); // 6
	float objectRotationBuffer[2]{ m_rotation.x, m_rotation.y };

	ImGui::DragFloat2("Rotation(x, y)", objectRotationBuffer, 0.1f);

	m_rotation = { objectRotationBuffer[0], objectRotationBuffer[1], m_rotation.z };


	if (ImGui::Button("Reset"))
	{
		m_rotation = { 30.0f, 60.0f, 0.0f };
	}

	ImGui::PopID(); // 6

	ImGui::PushID(5); // 5
	float positionBuffer[3]{ m_position.x, m_position.y, m_position.z };

	ImGui::DragFloat3("Position", positionBuffer, 0.1f);

	m_position = { positionBuffer[0], positionBuffer[1], positionBuffer[2] };


	if (ImGui::Button("Reset"))
	{
		m_position = { 0.0f, 0.0f, 0.0f };
	}

	ImGui::PopID(); // 5

	ImGui::PopID(); // 4

	ImGui::NewLine();

	ImGui::PushID(8); // 8
	ImGui::Text("Light");

	ImGui::PushID(9); // 9
	float lightRotationBuffer[2]{ m_lightRotation.x, m_lightRotation.y };
	ImGui::DragFloat2("Rotation(x, y)", lightRotationBuffer, 0.5f);

	m_lightRotation = { lightRotationBuffer[0], lightRotationBuffer[1], m_lightRotation.z };

	if (ImGui::Button("Reset"))
	{
		m_lightRotation = { -45.0f, 45.0f, 0.0f };
	}

	ImGui::PopID(); // 9

	ImGui::PushID(10); // 10
	float lightColorBuffer[3]{ m_lightColor.x, m_lightColor.y, m_lightColor.z };
	ImGui::ColorEdit3("Color", lightColorBuffer);

	m_lightColor = { lightColorBuffer[0], lightColorBuffer[1], lightColorBuffer[2], 1.0f };

	if (ImGui::Button("Reset"))
	{
		m_lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	ImGui::PopID(); // 10

	ImGui::PopID(); // 8

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
			crystalFaceNormals[i / 3]
		};

		vertices[i + 1] = {
			i / 3 < 6 ? hexagonPoints[(i / 3 + 1) % 6] : hexagonPoints[(i / 3) % 6],
			crystalFaceNormals[i / 3]
		};

		vertices[i + 2] = {
			i / 3 < 6 ? hexagonPoints[(i / 3) % 6] : hexagonPoints[(i / 3 + 1) % 6],
			crystalFaceNormals[i / 3]
		};

		indices[i] = i;
		indices[i + 1] = i + 1;
		indices[i + 2] = i + 2;
	}

	// Cube

	//Vertex vertices[] =
	//{
	//	{ Vector3{ -1.0f, 1.0f, -1.0f },	Vector3{ 0.0f, 1.0f, 0.0f } }, // Normal Y +	 
	//	{ Vector3{ 1.0f, 1.0f, -1.0f },		Vector3{ 0.0f, 1.0f, 0.0f } },
	//	{ Vector3{ 1.0f, 1.0f, 1.0f },		Vector3{ 0.0f, 1.0f, 0.0f } },
	//	{ Vector3{ -1.0f, 1.0f, 1.0f },		Vector3{ 0.0f, 1.0f, 0.0f } },

	//	{ Vector3{ -1.0f, -1.0f, -1.0f },	Vector3{ 0.0f, -1.0f, 0.0f } }, // Normal Y -		
	//	{ Vector3{ 1.0f, -1.0f, -1.0f },	Vector3{ 0.0f, -1.0f, 0.0f } },
	//	{ Vector3{ 1.0f, -1.0f, 1.0f },		Vector3{ 0.0f, -1.0f, 0.0f } },
	//	{ Vector3{ -1.0f, -1.0f, 1.0f },	Vector3{ 0.0f, -1.0f, 0.0f } },

	//	{ Vector3{ -1.0f, -1.0f, 1.0f },	Vector3{ -1.0f, 0.0f, 0.0f } }, // Normal X -
	//	{ Vector3{ -1.0f, -1.0f, -1.0f },	Vector3{ -1.0f, 0.0f, 0.0f } },
	//	{ Vector3{ -1.0f, 1.0f, -1.0f },	Vector3{ -1.0f, 0.0f, 0.0f } },
	//	{ Vector3{ -1.0f, 1.0f, 1.0f },		Vector3{ -1.0f, 0.0f, 0.0f } },

	//	{ Vector3{ 1.0f, -1.0f, 1.0f },		Vector3{ 1.0f, 0.0f, 0.0f } }, // Normal X +
	//	{ Vector3{ 1.0f, -1.0f, -1.0f },	Vector3{ 1.0f, 0.0f, 0.0f } },
	//	{ Vector3{ 1.0f, 1.0f, -1.0f },		Vector3{ 1.0f, 0.0f, 0.0f } },
	//	{ Vector3{ 1.0f, 1.0f, 1.0f },		Vector3{ 1.0f, 0.0f, 0.0f } },

	//	{ Vector3{ -1.0f, -1.0f, -1.0f },	Vector3{ 0.0f, 0.0f, -1.0f } }, // Normal Z -
	//	{ Vector3{ 1.0f, -1.0f, -1.0f },	Vector3{ 0.0f, 0.0f, -1.0f } },
	//	{ Vector3{ 1.0f, 1.0f, -1.0f },		Vector3{ 0.0f, 0.0f, -1.0f } },
	//	{ Vector3{ -1.0f, 1.0f, -1.0f },	Vector3{ 0.0f, 0.0f, -1.0f } },

	//	{ Vector3{ -1.0f, -1.0f, 1.0f },	Vector3{ 0.0f, 0.0f, 1.0f } }, // Normal Z +
	//	{ Vector3{ 1.0f, -1.0f, 1.0f },		Vector3{ 0.0f, 0.0f, 1.0f } },
	//	{ Vector3{ 1.0f, 1.0f, 1.0f },		Vector3{ 0.0f, 0.0f, 1.0f } },
	//	{ Vector3{ -1.0f, 1.0f, 1.0f },		Vector3{ 0.0f, 0.0f, 1.0f } },
	//};



	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vertexBufferData{};
	vertexBufferData.pSysMem = vertices;

	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer);

	m_vertexBufferStride = sizeof(Vertex);
	m_vertexBufferOffset = 0;

	D3D11_INPUT_ELEMENT_DESC layout[]{
		// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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


	// Cube

	//WORD indices[] =
	//{
	//	3, 1, 0,
	//	2, 1, 3,
	//	6, 4, 5,
	//	7, 4, 6,
	//	11, 9, 8,
	//	10, 9, 11,
	//	14, 12, 13,
	//	15, 12, 14,
	//	19, 17, 16,
	//	18, 17, 19,
	//	22, 20, 21,
	//	23, 20, 22
	//};

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

	// constant buffer
	D3D11_BUFFER_DESC constantBufferDesc{};
	constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer);


	m_world = Matrix::Identity;

	m_camera.GetViewMatrix(m_view);

	m_projection = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(m_camera.GetFOV()),
		(float)m_width / m_height,
		m_camera.GetNear(),
		m_camera.GetFar());
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