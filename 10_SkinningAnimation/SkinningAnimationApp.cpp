#include "SkinningAnimationApp.h"

#include <cmath>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "../Common/MyTime.h"
#include "../Common/Helper.h"
#include "../Common/Vertex.h"

#include "Material.h"

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

const float spaceX = 10.0f;
const float spaceY = 10.0f;
const float randMin = 0.1f;
const float randMax = 0.3f;

struct TransformBuffer
{
	Matrix world;
	Matrix view;
	Matrix projection;
};

struct EnvironmentBuffer
{
	Vector4 cameraPos;
	Vector4 lightDirection;
	Vector4 lightColor;
	Vector4 ambientLightColor;
};

struct MaterialBuffer
{
	Vector4 materialAmbient;
	Vector4 materialSpecular;
	float shininess;
	float __pad1[3];
};

void SkinningAnimationApp::Initialize()
{
	m_width = 1600;
	m_height = 900;

	WinApp::Initialize();

	m_camera.SetSpeed(100.0f);
	m_camera.SetPosition({ 0.0f, 100.0f, -400.0f });
	m_camera.SetFar(3000.0f);

	Material::CreateDefaultTextureSRV(m_graphicsDevice.GetDevice());

	InitializeImGui();

	InitializeScene();
}

void SkinningAnimationApp::OnUpdate()
{
	m_world =
		Matrix::CreateScale(m_scale) *
		Matrix::CreateRotationX(ToRadian(m_rotation.x)) *
		Matrix::CreateRotationY(ToRadian(m_rotation.y)) *
		Matrix::CreateTranslation(m_position);

	m_lightRotationMatrix =
		Matrix::CreateRotationX(ToRadian(m_lightRotation.x)) *
		Matrix::CreateRotationY(ToRadian(m_lightRotation.y));

	m_lightDirection = DirectX::XMVector3TransformNormal(m_originalLightDir, m_lightRotationMatrix);

	for (auto& skeletalMesh : m_skeletalMeshes)
	{
		skeletalMesh.Update(MyTime::DeltaTime());
	}
}

void SkinningAnimationApp::OnRender()
{
	// camera
	m_camera.GetViewMatrix(m_view);

	m_projection = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(m_camera.GetFOV()),
		static_cast<float>(m_width) / m_height,
		m_camera.GetNear(),
		m_camera.GetFar());

	// draw
	m_graphicsDevice.BeginDraw({ 0.85f, 0.82f, 0.95f, 1.0f });
	//{ 0.85f, 0.82f, 0.95f, 1.0f }
	//{ 0.65f, 0.90f, 0.85f, 1.0f }
	auto deviceContext = m_graphicsDevice.GetDeviceContext();

	// common
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

	// todo: dirty flag
	TransformBuffer transformBuffer{};
	transformBuffer.view = m_view.Transpose();
	transformBuffer.projection = m_projection.Transpose();

	deviceContext->VSSetConstantBuffers(0, 1, m_transformConstantBuffer.GetAddressOf());

	// todo: dirty flag
	EnvironmentBuffer environmentBuffer{};
	environmentBuffer.cameraPos = Vector4(m_camera.GetPosition());
	environmentBuffer.lightDirection = m_lightDirection;
	environmentBuffer.lightColor = m_lightColor;
	environmentBuffer.ambientLightColor = m_ambientLightColor;

	deviceContext->VSSetConstantBuffers(1, 1, m_environmentConstantBuffer.GetAddressOf());
	deviceContext->PSSetConstantBuffers(1, 1, m_environmentConstantBuffer.GetAddressOf());
	deviceContext->UpdateSubresource(m_environmentConstantBuffer.Get(), 0, nullptr, &environmentBuffer, 0, 0);

	// todo: dirty flag
	MaterialBuffer materialBuffer{};
	materialBuffer.materialAmbient = m_materialAmbient;
	materialBuffer.materialSpecular = m_materialSpecular;
	materialBuffer.shininess = m_shininess;

	deviceContext->PSSetConstantBuffers(2, 1, m_materialConstantBuffer.GetAddressOf());
	deviceContext->UpdateSubresource(m_materialConstantBuffer.Get(), 0, nullptr, &materialBuffer, 0, 0);

	deviceContext->VSSetConstantBuffers(3, 1, m_bonePoseConstantBuffer.GetAddressOf());
	deviceContext->VSSetConstantBuffers(4, 1, m_boneOffsetConstantBuffer.GetAddressOf());

	// skeletalMesh
	for (const auto& skeletalMesh : m_skeletalMeshes)
	{
		const auto& meshes = skeletalMesh.GetMeshes();
		const auto& materials = skeletalMesh.GetMaterials();

		deviceContext->UpdateSubresource(m_bonePoseConstantBuffer.Get(), 0, nullptr, skeletalMesh.GetSkeletonPose().data(), 0, 0);
		deviceContext->UpdateSubresource(m_boneOffsetConstantBuffer.Get(), 0, nullptr, skeletalMesh.GetBoneOffsets().data(), 0, 0);

		deviceContext->IASetInputLayout(m_blinnPhongInputLayout.Get());
		deviceContext->VSSetShader(m_blinnPhongVertexShader.Get(), nullptr, 0);
		deviceContext->PSSetShader(m_blinnPhongPixelShader.Get(), nullptr, 0);

		transformBuffer.world = skeletalMesh.GetWorld().Transpose();

		for (const auto& mesh : meshes)
		{
			deviceContext->UpdateSubresource(m_transformConstantBuffer.Get(), 0, nullptr, &transformBuffer, 0, 0);

			auto textureSRVs = materials[mesh.GetMaterialIndex()].GetTextureSRVs().AsRawArray();

			deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
			deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
			deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());

			deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
		}
	}

	RenderImGui();

	m_graphicsDevice.EndDraw();
}

void SkinningAnimationApp::OnShutdown()
{
	ShutdownImGui();

	Material::DestroyDefaultTextureSRV();
}

void SkinningAnimationApp::RenderImGui()
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
	float cameraRotationBuffer[2]{ ToDegree(cameraRotation.y), ToDegree(cameraRotation.x) };

	if (ImGui::DragFloat2("Yaw Pitch##1", cameraRotationBuffer))
	{
		m_camera.SetRotation({ ToRadian(cameraRotationBuffer[1]), ToRadian(cameraRotationBuffer[0]), cameraRotation.z });
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

	float scale = m_scale.x;
	//ImGui::DragFloat("Scale", &scale, 0.1f);
	m_scale = Vector3(scale, scale, scale);
	//ImGui::DragFloat2("Rotation(x, y)##1", &m_rotation.x, 0.1f);
	//ImGui::DragFloat3("Position##2", &m_position.x, 0.1f);
	ImGui::ColorEdit3("Ambient", &m_materialAmbient.x);
	ImGui::ColorEdit3("Specular", &m_materialSpecular.x);
	ImGui::DragFloat("Shininess", &m_shininess, 5.0f, 1.0f, 10000.0f);

	if (ImGui::Button("Reset##2"))
	{
		m_scale = { 1.0f, 1.0f, 1.0f };
		m_rotation = { 0.0f, 0.0f, 0.0f };
		m_position = { 0.0f, 0.0f, 0.0f };
		m_materialAmbient = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_materialSpecular = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_shininess = 64.0f;
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

void SkinningAnimationApp::InitializeImGui()
{
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_graphicsDevice.GetDevice().Get(), m_graphicsDevice.GetDeviceContext().Get());
}

void SkinningAnimationApp::InitializeScene()
{
	auto device = m_graphicsDevice.GetDevice();

	const std::string fbxFileNames[]{
		"SkinningTest.fbx",
		"Zombie_Run.fbx"
	};

	const size_t numFBXs = ARRAYSIZE(fbxFileNames);
	const float radian = DirectX::XM_2PI / numFBXs;
	const float radius = 100.0f;

	m_skeletalMeshes.reserve(numFBXs);

	for (size_t i = 0; i < numFBXs; ++i)
	{
		Vector3 position{ radius * std::cos(radian * i), 0.0f, radius * std::sin(radian * i) };
		//Vector3 position{ 0.0f, 0.0f, 0.0f };

		m_skeletalMeshes.emplace_back(device, fbxFileNames[i], Matrix::CreateTranslation(position));
		m_skeletalMeshes.back().PlayAnimation(0);
	}

	// object
	{
		m_vertexBufferStride = sizeof(BoneWeightVertex3D);

		D3D11_INPUT_ELEMENT_DESC layout[]{
			// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		ComPtr<ID3DBlob> vertexShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"BlinnPhongVertexShader.hlsl", "main", "vs_5_0", vertexShaderBuffer);

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
		GraphicsDevice::CompileShaderFromFile(L"BlinnPhongPixelShader.hlsl", "main", "ps_5_0", pixelShaderBuffer);

		device->CreatePixelShader(
			pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(),
			nullptr,
			&m_blinnPhongPixelShader
		);
	}

	// constant buffer
	D3D11_BUFFER_DESC constantBufferDesc{};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	constantBufferDesc.ByteWidth = sizeof(TransformBuffer);
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_transformConstantBuffer);

	constantBufferDesc.ByteWidth = sizeof(EnvironmentBuffer);
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_environmentConstantBuffer);

	constantBufferDesc.ByteWidth = sizeof(MaterialBuffer);
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_materialConstantBuffer);

	constantBufferDesc.ByteWidth = sizeof(Matrix) * MAX_BONE_NUM;
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_bonePoseConstantBuffer);

	constantBufferDesc.ByteWidth = sizeof(Matrix) * MAX_BONE_NUM;
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_boneOffsetConstantBuffer);
	

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
		ToRadian(m_camera.GetFOV()),
		static_cast<float>(m_width) / m_height,
		m_camera.GetNear(),
		m_camera.GetFar());
}

void SkinningAnimationApp::ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

LRESULT SkinningAnimationApp::MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	return WinApp::MessageProc(hWnd, uMsg, wParam, lParam);
}