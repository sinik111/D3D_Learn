#include "FBXLoadingApp.h"

#include <cmath>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <directxtk/WICTextureLoader.h>
#include <directxtk/DDSTextureLoader.h>

#include "../Common/MyTime.h"

#include "Model.h"
#include "Mesh.h"
#include "Material.h"

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

void FBXLoadingApp::Initialize()
{
	m_width = 1280;
	m_height = 720;

	WinApp::Initialize();

	m_camera.SetSpeed(100.0f);
	m_camera.SetPosition({ 0.0f, 0.0f, -100.0f });

	Material::CreateDefaultTextureSRV(m_graphicsDevice.GetDevice());

	InitializeImGui();

	InitializeScene();
}

void FBXLoadingApp::OnUpdate()
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

void FBXLoadingApp::OnRender()
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

	cb.cameraPos = Vector4(m_camera.GetPosition());
	cb.lightDirection = m_lightDirection;
	cb.lightColor = m_lightColor;
	cb.ambientLightColor = m_ambientLightColor;
	cb.materialAmbient = m_materialAmbient;
	cb.materialSpecular = m_materialSpecular;
	cb.shininess = Vector4(m_shininess);

	// skybox
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	deviceContext->IASetInputLayout(m_skyboxInputLayout.Get());
	deviceContext->VSSetShader(m_skyboxVertexShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_skyboxPixelShader.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
	deviceContext->PSSetShaderResources(0, 1, m_skyboxTextureRV.GetAddressOf());
	deviceContext->RSSetState(m_rasterizerState.Get());
	deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	Matrix skyboxWorld = Matrix::CreateRotationY(DirectX::XMConvertToRadians(-90)) *
		Matrix::CreateTranslation(m_camera.GetPosition());
	cb.world = skyboxWorld.Transpose();

	deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
	deviceContext->DrawIndexed(m_indexCount, 0, 0);
	deviceContext->RSSetState(nullptr);
	deviceContext->OMSetDepthStencilState(nullptr, 0);

	// model
	for (const auto& model : m_models)
	{
		if (model.GetName() == L"zeldaPosed001")
		{
			for (size_t i = 0; i < m_instanceDatas.size(); ++i)
			{
				const auto& meshes = model.GetMeshes();
				const auto& materials = model.GetMaterials();

				for (const auto& mesh : meshes)
				{
					const auto& material = materials[mesh.GetMaterialIndex()];

					deviceContext->IASetInputLayout(m_inputLayout.Get());
					deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
					deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

					deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
					deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

					const auto& textureSRVs = material.GetTextureSRVs();

					deviceContext->PSSetShaderResources(0, 1, textureSRVs.diffuseTextureSRV.GetAddressOf());
					deviceContext->PSSetShaderResources(1, 1, textureSRVs.normalTextureSRV.GetAddressOf());
					deviceContext->PSSetShaderResources(2, 1, textureSRVs.specularTextureSRV.GetAddressOf());
					deviceContext->PSSetShaderResources(3, 1, textureSRVs.emissiveTextureSRV.GetAddressOf());
					deviceContext->PSSetShaderResources(4, 1, textureSRVs.opacityTextureSRV.GetAddressOf());

					cb.world = m_instanceDatas[i].world.Transpose();
					cb.normalMatrix = model.GetWorld().Invert().Transpose().Transpose();

					float blendFactor[4]{ 0.0f, 0.0f, 0.0f, 0.0f };

					deviceContext->OMSetBlendState(m_blendState.Get(), blendFactor, 0xFFFFFFFF);

					deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
					deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
					//deviceContext->DrawIndexedInstanced()

					deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
				}
			}
		}

		const auto& meshes = model.GetMeshes();
		const auto& materials = model.GetMaterials();

		for (const auto& mesh : meshes)
		{
			const auto& material = materials[mesh.GetMaterialIndex()];

			deviceContext->IASetInputLayout(m_inputLayout.Get());
			deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
			deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R16_UINT, 0);

			deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
			deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

			const auto& textureSRVs = material.GetTextureSRVs();

			deviceContext->PSSetShaderResources(0, 1, textureSRVs.diffuseTextureSRV.GetAddressOf());
			deviceContext->PSSetShaderResources(1, 1, textureSRVs.normalTextureSRV.GetAddressOf());
			deviceContext->PSSetShaderResources(2, 1, textureSRVs.specularTextureSRV.GetAddressOf());
			deviceContext->PSSetShaderResources(3, 1, textureSRVs.emissiveTextureSRV.GetAddressOf());
			deviceContext->PSSetShaderResources(4, 1, textureSRVs.opacityTextureSRV.GetAddressOf());

			cb.world = model.GetWorld().Transpose();
			cb.normalMatrix = model.GetWorld().Invert().Transpose().Transpose();

			float blendFactor[4]{ 0.0f, 0.0f, 0.0f, 0.0f };

			deviceContext->OMSetBlendState(m_blendState.Get(), blendFactor, 0xFFFFFFFF);

			deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
			deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
			//deviceContext->DrawIndexedInstanced()

			deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
		}
	}

	// cube
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	deviceContext->IASetInputLayout(m_inputLayout.Get());
	deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);
	deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());

	const auto& defaultTextureSRVs = Material::GetDefaultTextureSRVs();

	deviceContext->PSSetShaderResources(0, 1, m_cubeDiffuseRV.GetAddressOf());
	deviceContext->PSSetShaderResources(1, 1, m_cubeNormalRV.GetAddressOf());
	deviceContext->PSSetShaderResources(2, 1, m_cubeSpecularRV.GetAddressOf());
	deviceContext->PSSetShaderResources(3, 1, defaultTextureSRVs.emissiveTextureSRV.GetAddressOf());
	deviceContext->PSSetShaderResources(4, 1, defaultTextureSRVs.opacityTextureSRV.GetAddressOf());

	cb.world = m_world.Transpose();
	cb.normalMatrix = m_world.Invert().Transpose().Transpose();

	deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	RenderImGui();

	m_graphicsDevice.EndDraw();
}

void FBXLoadingApp::OnShutdown()
{
	ShutdownImGui();

	Material::DestroyDefaultTextureSRV();
}

void FBXLoadingApp::RenderImGui()
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
	ImGui::DragFloat("Shininess", &m_shininess, 5.0f, 1.0f, 10000.0f);

	if (ImGui::Button("Reset##2"))
	{
		m_scale = { 50.0f, 50.0f, 50.0f };
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
		m_lightRotation = { -75.0f, 25.0f, 0.0f };
		m_lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_ambientLightColor = { 0.1f, 0.1f, 0.1f, 1.0f };
	}

	ImGui::Text("%d", ImGui::GetFrameHeight());

	ImGui::End();

	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void FBXLoadingApp::InitializeImGui()
{
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_graphicsDevice.GetDevice().Get(), m_graphicsDevice.GetDeviceContext().Get());
}

void FBXLoadingApp::InitializeScene()
{
	auto device = m_graphicsDevice.GetDevice();

	const char* fbxFileNames[]{
		"Tree.fbx",
		"zeldaPosed001.fbx",
		"Character.fbx",
		"Arissa.fbx",
		"box.fbx",
		"cerberus.fbx",
		"Monkey.fbx",
		"Vampire_SkinningTest.fbx"
	};

	const size_t numFBXs = ARRAYSIZE(fbxFileNames);
	const float radian = DirectX::XM_2PI / numFBXs;
	const float radius = 300.0f;

	m_models.reserve(numFBXs);

	for (size_t i = 0; i < numFBXs; ++i)
	{
		Vector3 position{ radius * std::cos(radian * i), 0.0f, radius * std::sin(radian * i) };
		m_models.emplace_back(device, fbxFileNames[i], Matrix::CreateTranslation(position));
	}

	UINT maxInstances = (m_maxShells * 2 + 1) * (m_maxShells * 2 + 1);

	m_instanceDatas.reserve(maxInstances);

	const float x = 100.0f;
	const float y = 100.0f;

	int index = 3;
	// (n + 1) * 4
	// (n + 1) * 100, (n + 1) * 100
	// i / (n * 2 - 1)

	for (UINT i = m_startShell; i <= m_currentShells; ++i)
	{
		const int numDivide = i * 2 - 1;

		const Vector3 rightFront{ (i + 1) * x, 0.0f, (i + 1) * y };
		const Vector3 rightBack{ (i + 1) * x, 0.0f, (i + 1) * -y };
		const Vector3 leftBack{ (i + 1) * -x, 0.0f, (i + 1) * -y };
		const Vector3 leftFront{ (i + 1) * -x, 0.0f, (i + 1) * y };

		const Vector3 verticalDelta = (rightBack - rightFront) / (float)numDivide;
		const Vector3 horizontalDelta = (leftFront - rightFront) / (float)numDivide;

		Vector3 p1 = rightFront;
		Vector3 p2 = rightBack;
		Vector3 p3 = leftBack;
		Vector3 p4 = leftFront;

		for (int j = 0; j < numDivide; ++j)
		{
			p1 += verticalDelta;
			p2 += horizontalDelta;
			p3 += -verticalDelta;
			p4 += -horizontalDelta;

			m_instanceDatas.emplace_back(Matrix::CreateTranslation(p1));
			m_instanceDatas.emplace_back(Matrix::CreateTranslation(p2));
			m_instanceDatas.emplace_back(Matrix::CreateTranslation(p3));
			m_instanceDatas.emplace_back(Matrix::CreateTranslation(p4));
		}
	}

	m_vertexBufferStride = sizeof(Vertex);

	// Cube

	Vertex vertices[] =
	{
		// Normal Z -
		// Normal Z +
		// Normal X -
		// Normal X +
		// Normal Y -
		// Normal Y +
		
		// position, uv, normal, tangent, binormal
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f }, {  0.0f,  0.0f, -1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } }, // 0 - 0
		{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f }, {  0.0f,  0.0f, -1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } }, // 1 - 1
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, {  0.0f,  0.0f, -1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } }, // 2 - 2
		{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f }, {  0.0f,  0.0f, -1.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } }, // 3 - 3
													 
		{ {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f }, {  0.0f,  0.0f,  1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } }, // 4 - 4
		{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, {  0.0f,  0.0f,  1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } }, // 5 - 5
		{ {  0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } }, // 6 - 6
		{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, { -1.0f, 0.0f,  0.0f }, { 0.0f, -1.0f,  0.0f } }, // 7 - 7
													 
		{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f } }, // 5 - 8
		{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f } }, // 0 - 9
		{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f } }, // 7 - 10
		{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f,  0.0f } }, // 2 - 11
													 
		{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f }, {  1.0f,  0.0f,  0.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f } }, // 1 - 12
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, {  1.0f,  0.0f,  0.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f } }, // 4 - 13
		{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f }, {  1.0f,  0.0f,  0.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f } }, // 3 - 14
		{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f }, {  1.0f,  0.0f,  0.0f }, {  0.0f, 0.0f,  1.0f }, { 0.0f, -1.0f,  0.0f } }, // 6 - 15
													 
		{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f }, {  0.0f, -1.0f,  0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } }, // 2 - 16
		{ {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f }, {  0.0f, -1.0f,  0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } }, // 3 - 17
		{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f }, {  0.0f, -1.0f,  0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } }, // 7 - 18
		{ {  0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f }, {  0.0f, -1.0f,  0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f,  1.0f } }, // 6 - 19
													 
		{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f }, {  0.0f,  1.0f,  0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } }, // 5 - 20
		{ {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f }, {  0.0f,  1.0f,  0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } }, // 4 - 21
		{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } }, // 0 - 22
		{ {  0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }, {  1.0f, 0.0f,  0.0f }, { 0.0f,  0.0f, -1.0f } }, // 1 - 23
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

	WORD indices[] =
	{
		0, 1, 2,
		2, 1, 3,

		4, 5, 6,
		6, 5, 7,

		8, 9, 10,
		10, 9, 11,

		12, 13, 14,
		14, 13, 15,

		16, 17, 18,
		18, 17, 19,

		20, 21, 22,
		22, 21, 23,
	};

	m_indexCount = ARRAYSIZE(indices);

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth = sizeof(WORD) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA indexBufferData{};
	indexBufferData.pSysMem = indices;
	device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer);

	// Skybox
	{
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

		ComPtr<ID3DBlob> pixelShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"SkyboxPixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer);

		device->CreatePixelShader(
			pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(),
			nullptr,
			&m_skyboxPixelShader
		);

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

	// object
	{
		D3D11_INPUT_ELEMENT_DESC layout[]{
			// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		ComPtr<ID3DBlob> vertexShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"BlinnPhongVertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer);

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

		ComPtr<ID3DBlob> pixelShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"BlinnPhongPixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer);

		device->CreatePixelShader(
			pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(),
			nullptr,
			&m_pixelShader
		);

		DirectX::CreateWICTextureFromFile(device.Get(), L"Bricks059_1K-JPG_Color.jpg", nullptr, &m_cubeDiffuseRV);
		DirectX::CreateWICTextureFromFile(device.Get(), L"Bricks059_1K-JPG_NormalDX.jpg", nullptr, &m_cubeNormalRV);
		DirectX::CreateWICTextureFromFile(device.Get(), L"Bricks059_Specular.png", nullptr, &m_cubeSpecularRV);
	}

	// instancing
	{
		D3D11_INPUT_ELEMENT_DESC layout[]{
			// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 56, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 56, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 56, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 56, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};

		ComPtr<ID3DBlob> vertexShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"InstancingVertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer);

		device->CreateInputLayout(
			layout,
			ARRAYSIZE(layout),
			vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(),
			&m_instancingInputLayout
		);

		device->CreateVertexShader(
			vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(),
			nullptr,
			&m_instancingVertexShader
		);
	}

	// constant buffer
	D3D11_BUFFER_DESC constantBufferDesc{};
	constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer);

	// instance buffer
	D3D11_BUFFER_DESC instanceBufferDesc{};
	instanceBufferDesc.ByteWidth = maxInstances * sizeof(InstanceData);
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	device->CreateBuffer(&instanceBufferDesc, nullptr, &m_instanceBuffer);

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

	// blend
	D3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = TRUE;

	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	device->CreateBlendState(&blendDesc, &m_blendState);

	// 


	m_world = Matrix::Identity;

	m_camera.GetViewMatrix(m_view);

	m_projection = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(m_camera.GetFOV()),
		(float)m_width / m_height,
		m_camera.GetNear(),
		m_camera.GetFar());
}

void FBXLoadingApp::ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

LRESULT FBXLoadingApp::MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	return WinApp::MessageProc(hWnd, uMsg, wParam, lParam);
}