#include "TransformApp.h"

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
	Vector4 color;
};

struct ConstantBuffer
{
	Matrix world;
	Matrix view;
	Matrix projection;
};

void TransformApp::Initialize()
{
	m_width = 1280;
	m_height = 720;

	WinApp::Initialize();

	InitializeImGui();

	InitializeScene();
}

void TransformApp::OnUpdate()
{
	m_firstRotationY += DirectX::XM_2PI * 0.2f * MyTime::DeltaTime();
	m_secondRotationY += DirectX::XM_2PI * 0.5f * MyTime::DeltaTime();
	m_thirdRotationY += DirectX::XM_2PI * 1.0f * MyTime::DeltaTime();

	m_world1 =
	{
		std::cos(m_firstRotationY), 0.0f, std::sin(m_firstRotationY), 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-std::sin(m_firstRotationY), 0.0f, std::cos(m_firstRotationY), 0.0f,
		m_firstPosition.x, m_firstPosition.y, m_firstPosition.z, 1.0f
	};

	m_world2 =
	Matrix{
		std::cos(m_secondRotationY) * 0.8f, 0.0f, std::sin(m_secondRotationY) * 0.8f, 0.0f,
		0.0f, 0.8f, 0.0f, 0.0f,
		-std::sin(m_secondRotationY) * 0.8f, 0.0f, std::cos(m_secondRotationY) * 0.8f, 0.0f,
		m_secondPosition.x, m_secondPosition.y, m_secondPosition.z, 1.0f
	} * m_world1;

	m_world3 =
	Matrix{
		std::cos(m_thirdRotationY) * 0.8f, 0.0f, std::sin(m_thirdRotationY) * 0.8f, 0.0f,
		0.0f, 0.8f, 0.0f, 0.0f,
		-std::sin(m_thirdRotationY) * 0.8f, 0.0f, std::cos(m_thirdRotationY) * 0.8f, 0.0f,
		m_thirdPosition.x, m_thirdPosition.y, m_thirdPosition.z, 1.0f
	} * m_world2;
}

bool m_show_demo_window = true;

void TransformApp::OnRender()
{
	// view, projection setting

	//m_camera.GetViewMatrix(m_view);

	//m_projection = DirectX::XMMatrixPerspectiveFovLH(
	//	DirectX::XMConvertToRadians(m_camera.GetFOV()),
	//	(float)m_width / m_height,
	//	m_camera.GetNear(),
	//	m_camera.GetFar());
	
	// view matrix
	Matrix cameraWorld = m_camera.GetWorldMatrix();

	Vector3 eye = m_camera.GetPosition();

	Vector3 up = cameraWorld.Up();
	up.Normalize();
	Vector3 viewAxisZ = -cameraWorld.Forward(); // viewAxisZ
	viewAxisZ.Normalize();

	//up cross viewAxisZ
	Vector3 viewAxisX = Vector3(
		up.y * viewAxisZ.z - up.z * viewAxisZ.y,
		up.z * viewAxisZ.x - up.x * viewAxisZ.z,
		up.x * viewAxisZ.y - up.y * viewAxisZ.x
	);

	// viewAxisZ cross viewAxisX 
	Vector3 viewAxisY = Vector3(
		viewAxisZ.y * viewAxisX.z - viewAxisZ.z * viewAxisX.y,
		viewAxisZ.z * viewAxisX.x - viewAxisZ.x * viewAxisX.z,
		viewAxisZ.x * viewAxisX.y - viewAxisZ.y * viewAxisX.x
	);

	m_view = Matrix{
		viewAxisX.x, viewAxisX.y, viewAxisX.z, 0.0f,
		viewAxisY.x, viewAxisY.y, viewAxisY.z, 0.0f,
		viewAxisZ.x, viewAxisZ.y, viewAxisZ.z, 0.0f,
		-eye.x, -eye.y, -eye.z, 1.0f
	};


	// projection matrix

	float aspectRatio = (float)m_width / m_height;
	float nearz = m_camera.GetNear();
	float farz = m_camera.GetFar();
	float fovY = m_camera.GetFOV() / 360 * DirectX::XM_2PI;

	m_projection = Matrix{
		1 / (aspectRatio * std::tan(fovY / 2)), 0.0f, 0.0f, 0.0f,
		0.0f, 1 / std::tan(fovY / 2), 0.0f, 0.0f,
		0.0f, 0.0f, farz / (farz - nearz), 1.0f,
		0.0f, 0.0f, -farz * nearz / (farz - nearz), 0.0f
	};


	m_graphicsDevice.BeginDraw({ 0.5f, 0.7f, 0.9f, 1.0f });

	auto deviceContext = m_graphicsDevice.GetDeviceContext();

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	deviceContext->IASetInputLayout(m_inputLayout.Get());
	deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
	deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	ConstantBuffer cb{};
	cb.world = m_world1.Transpose();
	cb.view = m_view.Transpose();
	cb.projection = m_projection.Transpose();

	deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	cb.world = m_world2.Transpose();

	deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	cb.world = m_world3.Transpose();

	deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	RenderImGui();

	m_graphicsDevice.EndDraw();
}

void TransformApp::OnShutdown()
{
	ShutdownImGui();
}

void TransformApp::RenderImGui()
{
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	ImGui::Begin("Controller");

	ImGui::PushID(0);
	ImGui::Text("Camera");

	ImGui::PushID(1);
	Vector3 cameraPosition = m_camera.GetPosition();
	float cameraPositionBuffer[3]{ cameraPosition.x, cameraPosition.y, cameraPosition.z };

	ImGui::DragFloat3("Position", cameraPositionBuffer, 0.1f);

	m_camera.SetPosition({ cameraPositionBuffer[0], cameraPositionBuffer[1], cameraPositionBuffer[2] });


	if (ImGui::Button("Reset"))
	{
		m_camera.SetPosition({ 0.0f, 0.0f, -10.0f });
	}

	ImGui::PopID(); // 1

	ImGui::PushID(2);
	float cameraFov = m_camera.GetFOV();

	ImGui::DragFloat("FOV", &cameraFov, 0.5f, 1.0f, 120.0f);

	m_camera.SetFOV(cameraFov);


	if (ImGui::Button("Reset"))
	{
		m_camera.SetFOV(90.0f);
	}

	ImGui::PopID(); // 2

	ImGui::PushID(3);
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

	ImGui::PushID(4);
	ImGui::Text("Object");

	ImGui::PushID(5);
	ImGui::Text("First");
	float firstPositionBuffer[3]{ m_firstPosition.x, m_firstPosition.y, m_firstPosition.z };

	ImGui::DragFloat3("Position", firstPositionBuffer, 0.1f);

	m_firstPosition = { firstPositionBuffer[0], firstPositionBuffer[1], firstPositionBuffer[2] };


	if (ImGui::Button("Reset"))
	{
		m_firstPosition = { 0.0f, 0.0f, 0.0f };
	}

	ImGui::PopID(); // 5

	ImGui::PushID(6);
	ImGui::Text("Second");
	float secondPositionBuffer[3]{ m_secondPosition.x, m_secondPosition.y, m_secondPosition.z };

	ImGui::DragFloat3("Position", secondPositionBuffer, 0.1f);

	m_secondPosition = { secondPositionBuffer[0], secondPositionBuffer[1], secondPositionBuffer[2] };


	if (ImGui::Button("Reset"))
	{
		m_secondPosition = { 5.0f, 0.0f, 0.0f };
	}

	ImGui::PopID(); // 6

	ImGui::PushID(7);
	ImGui::Text("Third");
	float thirdPositionBuffer[3]{ m_thirdPosition.x, m_thirdPosition.y, m_thirdPosition.z };

	ImGui::DragFloat3("Position", thirdPositionBuffer, 0.1f);

	m_thirdPosition = { thirdPositionBuffer[0], thirdPositionBuffer[1], thirdPositionBuffer[2] };


	if (ImGui::Button("Reset"))
	{
		m_thirdPosition = { 2.0f, 0.0f, 0.0f };
	}

	ImGui::PopID(); // 7

	ImGui::PopID(); // 4

	ImGui::End();

	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void TransformApp::InitializeImGui()
{
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_graphicsDevice.GetDevice().Get(), m_graphicsDevice.GetDeviceContext().Get());
}

void TransformApp::InitializeScene()
{
	auto device = m_graphicsDevice.GetDevice();


	Vertex vertices[]{
		{
			Vector3{ 0.0f, 1.0f, 0.0f },
			Vector4{ 1.0f, 1.0f, 1.0f, 1.0f }
		},
		{
			Vector3{ 0.5f, 0.0f, 0.0f },
			Vector4{ 1.0f, 0.0f, 0.0f, 1.0f }
		},
		{
			Vector3{ std::cos(DirectX::XM_2PI * 1 / 6) * 0.5f, 0.0f, std::sin(DirectX::XM_2PI * 1 / 6) * 0.5f},
			Vector4{ 1.0f, 0.5f, 0.0f, 1.0f }
		},
		{
			Vector3{ std::cos(DirectX::XM_2PI * 2 / 6) * 0.5f, 0.0f, std::sin(DirectX::XM_2PI * 2 / 6) * 0.5f },
			Vector4{ 1.0f, 1.0f, 0.0f, 1.0f }
		},
		{
			Vector3{ -0.5f, 0.0f, 0.0f },
			Vector4{ 0.0f, 1.0f, 0.0f, 1.0f }
		},
		{
			Vector3{ std::cos(DirectX::XM_2PI * 4 / 6) * 0.5f, 0.0f, std::sin(DirectX::XM_2PI * 4 / 6) * 0.5f },
			Vector4{ 0.0f, 0.0f, 1.0f, 1.0f }
		},
		{
			Vector3{ std::cos(DirectX::XM_2PI * 5 / 6) * 0.5f, 0.0f, std::sin(DirectX::XM_2PI * 5 / 6) * 0.5f },
			Vector4{ 0.5f, 0.0f, 1.0f, 1.0f }
		},
		{
			Vector3{ 0.0f, -1.0f, 0.0f },
			Vector4{ 0.0f, 0.0f, 0.0f, 1.0f }
		}
	};

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
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};


	ComPtr<ID3DBlob> vertexShaderBuffer;
	GraphicsDevice::CompileShaderFromFile(L"BasicVertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer);

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
	
	WORD indices[]{
		0, 2, 1,
		0, 3, 2,
		0, 4, 3,
		0, 5, 4,
		0, 6, 5,
		0, 1, 6,
		7, 1, 2,
		7, 2, 3,
		7, 3, 4,
		7, 4, 5,
		7, 5, 6,
		7, 6, 1,
	};

	m_indexCount = ARRAYSIZE(indices);

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth = sizeof(WORD) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA indexBufferData{};
	indexBufferData.pSysMem = indices;
	device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer);


	ComPtr<ID3DBlob> pixelShaderBuffer;
	GraphicsDevice::CompileShaderFromFile(L"BasicPixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer);

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


	m_world1 = Matrix::Identity;
	m_world2 = Matrix::Identity;
	m_world3 = Matrix::Identity;

	m_camera.GetViewMatrix(m_view);

	m_projection = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(m_camera.GetFOV()),
		(float)m_width / m_height,
		m_camera.GetNear(),
		m_camera.GetFar());
}

void TransformApp::ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

LRESULT TransformApp::MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	return WinApp::MessageProc(hWnd, uMsg, wParam, lParam);
}