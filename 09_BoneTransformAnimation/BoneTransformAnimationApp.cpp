#include "BoneTransformAnimationApp.h"

#include <cmath>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <directxtk/WICTextureLoader.h>
#include <directxtk/DDSTextureLoader.h>

#include "../Common/MyTime.h"
#include "../Common/Helper.h"

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
	Matrix normalMatrix;

	Vector4 materialAmbient;
	Vector4 materialSpecular;

	Vector4 cameraPos;
	Vector4 lightDirection;
	Vector4 lightColor;
	Vector4 ambientLightColor;
	Vector4 shininess;
};

void BoneTransformAnimationApp::Initialize()
{
	m_width = 1600;
	m_height = 900;

	WinApp::Initialize();

	m_camera.SetSpeed(100.0f);
	m_camera.SetPosition({ 0.0f, 70.0f, -200.0f });
	m_camera.SetFar(3000.0f);

	Material::CreateDefaultTextureSRV(m_graphicsDevice.GetDevice());

	InitializeImGui();

	InitializeScene();
}

void BoneTransformAnimationApp::OnUpdate()
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

void BoneTransformAnimationApp::OnRender()
{
	// camera
	m_camera.GetViewMatrix(m_view);

	m_projection = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(m_camera.GetFOV()),
		static_cast<float>(m_width) / m_height,
		m_camera.GetNear(),
		m_camera.GetFar());

	// draw
	m_graphicsDevice.BeginDraw({ 0.65f, 0.90f, 0.85f, 1.0f });
	//{ 0.85f, 0.82f, 0.95f, 1.0f }
	//{ 0.65f, 0.90f, 0.85f, 1.0f }
	auto deviceContext = m_graphicsDevice.GetDeviceContext();

	// common
	ConstantBuffer cb{};
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

	// model
	for (const auto& model : m_staticMeshes)
	{
		const auto& meshes = model.GetMeshes();
		const auto& materials = model.GetMaterials();

		deviceContext->IASetInputLayout(m_blinnPhongInputLayout.Get());
		deviceContext->VSSetShader(m_blinnPhongVertexShader.Get(), nullptr, 0);
		deviceContext->PSSetShader(m_blinnPhongPixelShader.Get(), nullptr, 0);

		//for (const auto& mesh : meshes)
		//{
		//	cb.world = model.GetWorld().Transpose();
		//	cb.normalMatrix = model.GetWorld().Invert().Transpose().Transpose();

		//	auto textureSRVs = materials[mesh.GetMaterialIndex()].GetTextureSRVs().AsRawArray();

		//	deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

		//	deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
		//	deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
		//	deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());

		//	deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
		//}
	}

	RenderImGui();

	m_graphicsDevice.EndDraw();
}

void BoneTransformAnimationApp::OnShutdown()
{
	ShutdownImGui();

	Material::DestroyDefaultTextureSRV();
}

void BoneTransformAnimationApp::RenderImGui()
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

	float cameraFov = m_camera.GetFOV();

	if (ImGui::DragFloat("FOV", &cameraFov, 0.5f, 1.0f, 120.0f))
	{
		m_camera.SetFOV(cameraFov);
	}

	float cameraNear = m_camera.GetNear();
	float cameraFar = m_camera.GetFar();

	if (ImGui::DragFloat("Near", &cameraNear, 1.0f, 0.01f, cameraFar - 10) ||
		ImGui::DragFloat("Far", &cameraFar, 1.0f, cameraNear + 10, 100000.f))
	{
		m_camera.SetNear(cameraNear);
		m_camera.SetFar(cameraFar);
	}

	float cameraSpeed = m_camera.GetSpeed();
	if (ImGui::DragFloat("Speed", &cameraSpeed, 0.1f, 2.0f, 200.0f))
	{
		m_camera.SetSpeed(cameraSpeed);
	}

	if (ImGui::Button("Reset##1"))
	{
		m_camera.SetPosition({ 0.0f, 0.0f, -100.0f });
		m_camera.SetRotation({ 0.0f, 0.0f, 0.0f });
		m_camera.SetFOV(50.0f);
		m_camera.SetNear(1.0f);
		m_camera.SetFar(1000.0f);
	}

	ImGui::NewLine();

	ImGui::SeparatorText("Object");

	ImGui::DragFloat3("Scale", &m_scale.x, 0.1f);
	ImGui::DragFloat2("Rotation(x, y)##1", &m_rotation.x, 0.1f);
	ImGui::DragFloat3("Position##2", &m_position.x, 0.1f);
	ImGui::ColorEdit3("Ambient", &m_materialAmbient.x);
	ImGui::ColorEdit3("Specular", &m_materialSpecular.x);
	ImGui::DragFloat("Shininess", &m_shininess.x, 5.0f, 1.0f, 10000.0f);

	if (ImGui::Button("Reset##2"))
	{
		m_scale = { 50.0f, 50.0f, 50.0f };
		m_rotation = { 0.0f, 0.0f, 0.0f };
		m_position = { 0.0f, 0.0f, 0.0f };
		m_materialAmbient = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_materialSpecular = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_shininess = { 64.0f, 0.0f, 0.0f, 0.0f };
	}

	ImGui::NewLine();

	ImGui::SeparatorText("Light");

	ImGui::DragFloat2("Rotation(x, y)##2", &m_lightRotation.x, 0.5f);
	ImGui::InputFloat3("Direction", &m_lightDirection.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::ColorEdit3("DirectLightColor", &m_lightColor.x);
	ImGui::ColorEdit3("AmbientLightColor", &m_ambientLightColor.x);

	if (ImGui::Button("Reset##3"))
	{
		m_lightRotation = { -40.0f, 25.0f, 0.0f };
		m_lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_ambientLightColor = { 0.1f, 0.1f, 0.1f, 1.0f };
	}

	ImGui::NewLine();
	ImGui::SeparatorText("Info");
	ImGui::Text("%d FPS", GetLastFPS());
	ImGui::End();

	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void BoneTransformAnimationApp::InitializeImGui()
{
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_graphicsDevice.GetDevice().Get(), m_graphicsDevice.GetDeviceContext().Get());
}

void BoneTransformAnimationApp::InitializeScene()
{
	auto device = m_graphicsDevice.GetDevice();

	const char* fbxFileNames[]{
		//"1CubeAnim.fbx",
		"BoxHuman.fbx",
	};

	const size_t numFBXs = ARRAYSIZE(fbxFileNames);
	const float radian = DirectX::XM_2PI / numFBXs;
	const float radius = 300.0f;

	m_staticMeshes.reserve(numFBXs);

	for (size_t i = 0; i < numFBXs; ++i)
	{
		Vector3 position{ radius * std::cos(radian * i), 0.0f, radius * std::sin(radian * i) };

		m_staticMeshes.emplace_back(device, fbxFileNames[i], Matrix::CreateTranslation(position));
	}

	// object
	{
		m_vertexBufferStride = sizeof(Vertex);

		D3D11_INPUT_ELEMENT_DESC layout[]{
			// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		ComPtr<ID3DBlob> vertexShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"BlinnPhongVertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer);

		device->CreateInputLayout(
			layout,
			ARRAYSIZE(layout),
			vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(),
			&m_blinnPhongInputLayout
		);

		device->CreateVertexShader(
			vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(),
			nullptr,
			&m_blinnPhongVertexShader
		);

		ComPtr<ID3DBlob> pixelShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"BlinnPhongPixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer);

		device->CreatePixelShader(
			pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(),
			nullptr,
			&m_blinnPhongPixelShader
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

void BoneTransformAnimationApp::ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

LRESULT BoneTransformAnimationApp::MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	return WinApp::MessageProc(hWnd, uMsg, wParam, lParam);
}