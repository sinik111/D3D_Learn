#include "ToonShadingWithOutlineApp.h"

#include <cmath>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "../Common/MyTime.h"
#include "../Common/Helper.h"

#include "StaticMesh.h"
#include "StaticMeshSection.h"
#include "Material.h"

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

const float spaceX = 10.0f;
const float spaceY = 10.0f;
const float randMin = 0.1f;
const float randMax = 0.3f;

struct ConstantBuffer
{
	Matrix world;
	Matrix view;
	Matrix projection;

	Vector4 materialAmbient;
	Vector4 materialSpecular;

	Vector4 cameraPos;
	Vector4 lightDirection;
	Vector4 lightColor;
	Vector4 ambientLightColor;
	float shininess;
	float __pad[3];
};

void ToonShadingWithOutlineApp::Initialize()
{
	m_width = 800;
	m_height = 600;

	WinApp::Initialize();

	m_camera.SetSpeed(100.0f);
	m_camera.SetPosition({ 0.0f, 0.0f, -200.0f });
	m_camera.SetFar(1000.0f);

	Material::CreateDefaultTextureSRV(m_graphicsDevice.GetDevice());

	InitializeImGui();
	InitializeScene();
}

void ToonShadingWithOutlineApp::OnUpdate()
{
	m_world =
		Matrix::CreateScale(m_scale) *
		Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_rotation.x)) *
		Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rotation.y)) *
		Matrix::CreateTranslation(m_position);

	m_lightRotationMatrix =
		Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_lightRotation.x)) *
		Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_lightRotation.y));

	m_lightDirection = DirectX::XMVector3TransformNormal(m_originalLightDir, m_lightRotationMatrix);
}

void ToonShadingWithOutlineApp::OnRender()
{
	// camera
	m_camera.GetViewMatrix(m_view);

	m_projection = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(m_camera.GetFOV()),
		static_cast<float>(m_width) / m_height,
		m_camera.GetNear(),
		m_camera.GetFar());

	// draw
	auto deviceContext = m_graphicsDevice.GetDeviceContext();
	auto renderTargetView = m_graphicsDevice.GetRenderTargetView();
	auto depthStencilView = m_graphicsDevice.GetDepthStencilView();

	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

	deviceContext->ClearRenderTargetView(renderTargetView.Get(), DirectX::SimpleMath::Color{ 0.0f, 0.0f, 0.0f, 0.0f });
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);


	// common
	ConstantBuffer cb{};
	cb.world = m_world.Transpose();
	cb.view = m_view.Transpose();
	cb.projection = m_projection.Transpose();
	cb.cameraPos = Vector4(m_camera.GetPosition());
	cb.lightDirection = m_lightDirection;
	cb.lightColor = m_lightColor;
	cb.ambientLightColor = m_ambientLightColor;
	cb.materialAmbient = m_materialAmbient;
	cb.materialSpecular = m_materialSpecular;
	cb.shininess = m_shininess;

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
	deviceContext->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
	deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

	// teapot
	const auto& meshes = m_teapot->GetMeshes();
	const auto& materials = m_teapot->GetMaterials();

	deviceContext->IASetInputLayout(m_vertexInputLayout.Get());
	deviceContext->VSSetShader(m_phongToonVertexShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_phongToonPixelShader.Get(), nullptr, 0);

	for (const auto& mesh : meshes)
	{
		deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

		deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
		deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

		deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
	}

	RenderImGui();

	m_graphicsDevice.GetSwapChain()->Present(1, 0);
}

void ToonShadingWithOutlineApp::OnShutdown()
{
	ShutdownImGui();

	Material::DestroyDefaultTextureSRV();
}

void ToonShadingWithOutlineApp::RenderImGui()
{
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	ImGui::Begin("Controller");

	ImGui::SeparatorText("Camera");

	Vector3 cameraPosition = m_camera.GetPosition();

	ImGui::DragFloat3("Position##1", &cameraPosition.x, 0.1f);

	m_camera.SetPosition(cameraPosition);

	Vector3 cameraRotation = m_camera.GetRotation();
	float cameraRotationBuffer[2]{ DirectX::XMConvertToDegrees(cameraRotation.y), DirectX::XMConvertToDegrees(cameraRotation.x) };

	if (ImGui::DragFloat2("Yaw Pitch##1", cameraRotationBuffer))
	{
		m_camera.SetRotation({ DirectX::XMConvertToRadians(cameraRotationBuffer[1]), DirectX::XMConvertToRadians(cameraRotationBuffer[0]), cameraRotation.z });
	}

	if (ImGui::Button("Reset##1"))
	{
		m_camera.SetPosition({ 0.0f, 0.0f, -100.0f });
		m_camera.SetRotation({ 0.0f, 0.0f, 0.0f });
	}

	ImGui::NewLine();

	ImGui::SeparatorText("Object");

	ImGui::DragFloat3("Scale", &m_scale.x, 0.1f);
	ImGui::DragFloat2("Rotation(x, y)##1", &m_rotation.x, 0.1f);
	ImGui::DragFloat3("Position##2", &m_position.x, 0.1f);
	ImGui::ColorEdit3("Ambient", &m_materialAmbient.x);
	ImGui::ColorEdit3("Specular", &m_materialSpecular.x);
	ImGui::DragFloat("Shininess", &m_shininess, 5.0f, 1.0f, 10000.0f);

	if (ImGui::Button("Reset##2"))
	{
		m_scale = { 0.01f, 0.01f, 0.01f };
		m_rotation = { 0.0f, 0.0f, 0.0f };
		m_position = { 0.0f, 0.0f, 0.0f };
		m_materialDiffuse = { 1.0f, 0.0f, 1.0f, 1.0f };
		m_materialAmbient = { 0.1f, 0.1f, 0.1f, 1.0f };
		m_materialSpecular = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_shininess = 64.0f;
	}

	ImGui::NewLine();

	ImGui::SeparatorText("Light");

	ImGui::DragFloat2("Rotation(x, y)##2", &m_lightRotation.x, 0.5f);
	ImGui::InputFloat3("Direction", &m_lightDirection.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::ColorEdit3("DirectColor", &m_lightColor.x);
	ImGui::ColorEdit3("AmbientColor", &m_ambientLightColor.x);

	if (ImGui::Button("Reset##3"))
	{
		m_lightRotation = { 0.0f, 0.0f, 0.0f };
		m_lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_ambientLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	}

	ImGui::End();

	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ToonShadingWithOutlineApp::InitializeImGui()
{
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_graphicsDevice.GetDevice().Get(), m_graphicsDevice.GetDeviceContext().Get());
}

void ToonShadingWithOutlineApp::InitializeScene()
{
	auto device = m_graphicsDevice.GetDevice();

	m_teapot = std::make_unique<StaticMesh>(device, "Teapot.fbx", Matrix::Identity);

	m_vertexBufferStride = sizeof(Vertex);
	
	// SimpleVertex input layout
	{
		D3D11_INPUT_ELEMENT_DESC layout[]{
			// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ComPtr<ID3DBlob> vertexShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"OutlineVertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer);

		device->CreateInputLayout(
			layout,
			ARRAYSIZE(layout),
			vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(),
			&m_simpleVertexInputLayout
		);

		device->CreateVertexShader(
			vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(),
			nullptr,
			&m_outlineVertexShader
		);

		ComPtr<ID3DBlob> pixelShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"SkyboxPixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer);

		device->CreatePixelShader(
			pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(),
			nullptr,
			&m_outlinePixelShader
		);

		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.FrontCounterClockwise = TRUE;

		device->CreateRasterizerState(&rasterizerDesc, &m_outlineRasterizerState);


		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		device->CreateDepthStencilState(&depthStencilDesc, &m_outlineDepthStencilState);
	}

	// object
	{
		D3D11_INPUT_ELEMENT_DESC layout[]{
			// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		ComPtr<ID3DBlob> vertexShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"PhongToonVertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer);

		device->CreateInputLayout(
			layout,
			ARRAYSIZE(layout),
			vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(),
			&m_vertexInputLayout
		);

		device->CreateVertexShader(
			vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(),
			nullptr,
			&m_phongToonVertexShader
		);

		ComPtr<ID3DBlob> pixelShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"PhongToonPixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer);

		device->CreatePixelShader(
			pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(),
			nullptr,
			&m_phongToonPixelShader
		);
	}

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
		static_cast<float>(m_width) / m_height,
		m_camera.GetNear(),
		m_camera.GetFar());
}

void ToonShadingWithOutlineApp::ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

LRESULT ToonShadingWithOutlineApp::MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	return WinApp::MessageProc(hWnd, uMsg, wParam, lParam);
}