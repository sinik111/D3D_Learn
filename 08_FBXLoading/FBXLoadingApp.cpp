#include "FBXLoadingApp.h"

#include <cmath>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <directxtk/WICTextureLoader.h>
#include <directxtk/DDSTextureLoader.h>

#include "../Common/MyTime.h"
#include "../Common/Helper.h"

#include "Model.h"
#include "Mesh.h"
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

void FBXLoadingApp::Initialize()
{
	m_width = 1600;
	m_height = 900;

	WinApp::Initialize();

	m_camera.SetSpeed(100.0f);
	m_camera.SetPosition({ 0.0f, 70.0f, -200.0f });
	m_camera.SetFar(1000000.0f);

	Material::CreateDefaultTextureSRV(m_graphicsDevice.GetDevice());

	InitializeImGui();

	InitializeScene();
}

void FBXLoadingApp::OnUpdate()
{
	m_cubeWorld =
		Matrix::CreateScale(m_scale) *
		Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_rotation.x)) *
		Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_rotation.y)) *
		Matrix::CreateTranslation(m_position);

	m_lightRotationMatrix =
		Matrix::CreateRotationX(DirectX::XMConvertToRadians(m_lightRotation.x)) *
		Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_lightRotation.y));

	m_lightDirection = DirectX::XMVector3TransformNormal(m_originalLightDir, m_lightRotationMatrix);

	m_crystalDegree += 45.0f * MyTime::DeltaTime();
	if (m_crystalDegree > 360.0f)
	{
		m_crystalDegree -= 360.0f;
	}

	m_crystalModel->SetWorld(
		Matrix::CreateScale(m_crystalScale) *
		Matrix::CreateRotationY(DirectX::XMConvertToRadians(m_crystalDegree)) * 
		Matrix::CreateTranslation(m_crystalPosition));
}

void FBXLoadingApp::OnRender()
{
	// view, projection setting

	m_camera.GetViewMatrix(m_view);

	m_projection = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(m_camera.GetFOV()),
		static_cast<float>(m_width) / m_height,
		m_camera.GetNear(),
		m_camera.GetFar());

	m_graphicsDevice.BeginDraw({ 1.0f, 0.0f, 1.0f, 1.0f });

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

	// skybox
	Matrix skyboxWorld =
		Matrix::CreateRotationY(DirectX::XMConvertToRadians(-90.0f)) *
		Matrix::CreateTranslation(m_camera.GetPosition());
	cb.world = skyboxWorld.Transpose();

	deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

	deviceContext->IASetVertexBuffers(0, 1, m_cubeVertexBuffer.GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_cubeIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetInputLayout(m_skyboxInputLayout.Get());
	deviceContext->VSSetShader(m_skyboxVertexShader.Get(), nullptr, 0);
	deviceContext->RSSetState(m_rasterizerState.Get());
	deviceContext->PSSetShader(m_skyboxPixelShader.Get(), nullptr, 0);
	deviceContext->PSSetShaderResources(0, 1, m_skyboxTextureRV.GetAddressOf());
	deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 0);

	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	deviceContext->RSSetState(nullptr);
	deviceContext->OMSetDepthStencilState(nullptr, 0);

	// model
	for (const auto& model : m_models)
	{
		const auto& meshes = model.GetMeshes();
		const auto& materials = model.GetMaterials();

		if (model.GetName() == L"Grass") // Grass.fbx 로 인스턴싱 테스트
		{
			if (m_useInstancing)
			{
				if (m_changed)
				{
					D3D11_MAPPED_SUBRESOURCE mappedResource;
					deviceContext->Map(m_instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
					memcpy(mappedResource.pData, m_instanceDatas.data(), m_instanceDatas.size() * sizeof(InstanceData));
					deviceContext->Unmap(m_instanceBuffer.Get(), 0);

					m_changed = false;
				}

				deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

				deviceContext->IASetInputLayout(m_instancingInputLayout.Get());
				deviceContext->VSSetShader(m_instancingVertexShader.Get(), nullptr, 0);
				deviceContext->PSSetShader(m_instancingPixelShader.Get(), nullptr, 0);

				for (const auto& mesh : meshes)
				{
					ID3D11Buffer* buffers[]{ mesh.GetVertexBuffer().Get(), m_instanceBuffer.Get() };

					auto textureSRVs = materials[mesh.GetMaterialIndex()].GetTextureSRVs().AsRawArray();

					deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
					deviceContext->IASetVertexBuffers(0, 2, buffers, m_instanceBufferStrides, m_instanceBufferOffsets);
					deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());

					deviceContext->DrawIndexedInstanced(mesh.GetIndexCount(), static_cast<UINT>(m_instanceDatas.size()), 0, 0, 0);
				}
			}
			else
			{
				for (const auto& mesh : meshes)
				{
					auto textureSRVs = materials[mesh.GetMaterialIndex()].GetTextureSRVs().AsRawArray();

					deviceContext->IASetInputLayout(m_blinnPhongInputLayout.Get());
					deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
					deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
					deviceContext->VSSetShader(m_blinnPhongVertexShader.Get(), nullptr, 0);
					deviceContext->PSSetShader(m_instancingPixelShader.Get(), nullptr, 0);
					deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());

					for (size_t i = 0; i < m_instanceDatas.size(); ++i)
					{
						cb.world = m_instanceDatas[i].world.Transpose();
						cb.normalMatrix = m_instanceDatas[i].world./*Invert().Transpose().*/Transpose();

						deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

						deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
					}
				}
			}
		}

		for (const auto& mesh : meshes)
		{
			cb.world = model.GetWorld().Transpose();
			cb.normalMatrix = model.GetWorld().Invert().Transpose().Transpose();

			auto textureSRVs = materials[mesh.GetMaterialIndex()].GetTextureSRVs().AsRawArray();

			deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

			deviceContext->IASetInputLayout(m_blinnPhongInputLayout.Get());
			deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
			deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
			deviceContext->VSSetShader(m_blinnPhongVertexShader.Get(), nullptr, 0);
			deviceContext->PSSetShader(m_blinnPhongPixelShader.Get(), nullptr, 0);
			deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());

			deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
		}
	}

	// cube
	cb.world = m_cubeWorld.Transpose();
	cb.normalMatrix = m_cubeWorld.Invert().Transpose().Transpose();

	deviceContext->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

	deviceContext->IASetVertexBuffers(0, 1, m_cubeVertexBuffer.GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_cubeIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetInputLayout(m_blinnPhongInputLayout.Get());
	deviceContext->VSSetShader(m_blinnPhongVertexShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_blinnPhongPixelShader.Get(), nullptr, 0);
	deviceContext->PSSetShaderResources(0, 5, m_cubeTextureSRVs.AsRawArray().data());

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

	ImGui::Text("%d FPS", GetLastFPS());
	
	ImGui::Checkbox("Use Instancing", &m_useInstancing);
	
	ImGui::Text("Instance Count: %d", m_instanceDatas.size());

	ImGui::Text("Shell Count: %d\t", m_currentShells);
	ImGui::SameLine();
	if (ImGui::ArrowButton("Sub Shell", ImGuiDir_Down))
	{
		SubShell();
	}
	ImGui::SameLine();
	if (ImGui::ArrowButton("Add Shell", ImGuiDir_Up))
	{
		AddShell();
	}

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
		"zeldaPosed001.fbx",
		"Character.fbx",
		"Arissa.fbx",
		"box.fbx",
		"Crystal.fbx",
		"cerberus.fbx",
		"Monkey.fbx",
		"Vampire_SkinningTest.fbx",
		"IcoSphere.fbx",
		"turtle.fbx",
		"Grass.fbx",
		"Tree.fbx"
	};

	const size_t numFBXs = ARRAYSIZE(fbxFileNames);
	const float radian = DirectX::XM_2PI / numFBXs;
	const float radius = 300.0f;

	m_models.reserve(numFBXs);

	for (size_t i = 0; i < numFBXs; ++i)
	{
		
		Vector3 position{ radius * std::cos(radian * i), 0.0f, radius * std::sin(radian * i) };

		if (strcmp(fbxFileNames[i], "Crystal.fbx") == 0)
		{
			position.y = 120.0f;
			m_crystalPosition = position;

			m_models.emplace_back(device, fbxFileNames[i], Matrix::CreateTranslation(position));

			m_crystalModel = &m_models.back();

			continue;
		}

		else if (strcmp(fbxFileNames[i], "turtle.fbx") == 0)
		{
			m_models.emplace_back(device, fbxFileNames[i], Matrix::CreateScale(0.5f) * Matrix::CreateTranslation({ 0.0f, 50.0f, 0.0f}));

			continue;
		}
		else if (strcmp(fbxFileNames[i], "Monkey.fbx") == 0)
		{
			position.y = 100.0f;

			m_models.emplace_back(device, fbxFileNames[i], Matrix::CreateScale(0.5f) * Matrix::CreateTranslation(position));

			continue;
		}
		else if (strcmp(fbxFileNames[i], "box.fbx") == 0)
		{
			position.y = 35.0f;

			m_models.emplace_back(device, fbxFileNames[i], Matrix::CreateTranslation(position));

			continue;
		}
		else if (strcmp(fbxFileNames[i], "IcoSphere.fbx") == 0)
		{
			position.y = 50.0f;

			m_models.emplace_back(device, fbxFileNames[i], Matrix::CreateScale(0.5f) * Matrix::CreateTranslation(position));

			continue;
		}
		else if (strcmp(fbxFileNames[i], "cerberus.fbx") == 0)
		{
			position.y = 100.0f;

			m_models.emplace_back(device, fbxFileNames[i], Matrix::CreateTranslation(position));

			continue;
		}

		m_models.emplace_back(device, fbxFileNames[i], Matrix::CreateTranslation(position));
	}

	const size_t maxInstances = (m_maxShells * 2) * (m_maxShells * 2);

	m_instanceDatas.reserve(maxInstances);

	for (size_t i = m_minShell; i <= m_currentShells; ++i)
	{
		const size_t numDivide = i * 2 - 1;

		const Vector3 rightFront{ i * spaceX, 0.0f, i * spaceY };
		const Vector3 rightBack{ i * spaceX, 0.0f, i * -spaceY };
		const Vector3 leftBack{ i * -spaceX, 0.0f, i * -spaceY };
		const Vector3 leftFront{ i * -spaceX, 0.0f, i * spaceY };

		const Vector3 verticalDelta = (rightBack - rightFront) / static_cast<float>(numDivide);
		const Vector3 horizontalDelta = (leftFront - rightFront) / static_cast<float>(numDivide);

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

			m_instanceDatas.emplace_back(Matrix::CreateScale(RandomFloat(randMin, randMax)) * Matrix::CreateRotationY(DirectX::XMConvertToRadians(RandomFloat(0.0f, 360.0f))) * Matrix::CreateTranslation(p1));
			m_instanceDatas.emplace_back(Matrix::CreateScale(RandomFloat(randMin, randMax)) * Matrix::CreateRotationY(DirectX::XMConvertToRadians(RandomFloat(0.0f, 360.0f))) * Matrix::CreateTranslation(p2));
			m_instanceDatas.emplace_back(Matrix::CreateScale(RandomFloat(randMin, randMax)) * Matrix::CreateRotationY(DirectX::XMConvertToRadians(RandomFloat(0.0f, 360.0f))) * Matrix::CreateTranslation(p3));
			m_instanceDatas.emplace_back(Matrix::CreateScale(RandomFloat(randMin, randMax)) * Matrix::CreateRotationY(DirectX::XMConvertToRadians(RandomFloat(0.0f, 360.0f))) * Matrix::CreateTranslation(p4));
		}
	}

	m_vertexBufferStride = sizeof(Vertex);
	m_instanceBufferStrides[0] = m_vertexBufferStride;
	m_instanceBufferStrides[1] = sizeof(InstanceData);
	m_instanceBufferOffsets[0] = m_vertexBufferOffset;
	m_instanceBufferOffsets[1] = 0;

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

	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_cubeVertexBuffer);

	DWORD indices[] =
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
	indexBufferDesc.ByteWidth = sizeof(DWORD) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA indexBufferData{};
	indexBufferData.pSysMem = indices;
	device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_cubeIndexBuffer);

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
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.FrontCounterClockwise = TRUE;

		device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);


		// DepthStencil State - skybox를 가장 깊이 그리고 깊이 값을 기록을 안하기 위함
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
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

		DirectX::CreateWICTextureFromFile(device.Get(), L"Bricks059_1K-JPG_Color.jpg", nullptr, &m_cubeTextureSRVs.diffuseTextureSRV);
		DirectX::CreateWICTextureFromFile(device.Get(), L"Bricks059_1K-JPG_NormalDX.jpg", nullptr, &m_cubeTextureSRVs.normalTextureSRV);
		DirectX::CreateWICTextureFromFile(device.Get(), L"Bricks059_Specular.png", nullptr, &m_cubeTextureSRVs.specularTextureSRV);
		m_cubeTextureSRVs.opacityTextureSRV = Material::GetDefaultTextureSRVs().opacityTextureSRV;
		m_cubeTextureSRVs.emissiveTextureSRV = Material::GetDefaultTextureSRVs().emissiveTextureSRV;
	}

	// instancing
	{
		D3D11_INPUT_ELEMENT_DESC layout[]{
			// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",	  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },

			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
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

		ComPtr<ID3DBlob> pixelShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"InstancingPixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer);

		device->CreatePixelShader(
			pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(),
			nullptr,
			&m_instancingPixelShader
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
	instanceBufferDesc.ByteWidth = static_cast<UINT>(maxInstances * sizeof(InstanceData));
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

	m_cubeWorld = Matrix::Identity;

	m_camera.GetViewMatrix(m_view);

	m_projection = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XMConvertToRadians(m_camera.GetFOV()),
		static_cast<float>(m_width) / m_height,
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

void FBXLoadingApp::AddShell()
{
	if (m_currentShells >= m_maxShells)
	{
		return;
	}

	++m_currentShells;

	const size_t i = m_currentShells;

	const size_t numDivide = i * 2 - 1;

	const Vector3 rightFront{ i * spaceX, 0.0f, i * spaceY };
	const Vector3 rightBack{ i * spaceX, 0.0f, i * -spaceY };
	const Vector3 leftBack{ i * -spaceX, 0.0f, i * -spaceY };
	const Vector3 leftFront{ i * -spaceX, 0.0f, i * spaceY };

	const Vector3 verticalDelta = (rightBack - rightFront) / static_cast<float>(numDivide);
	const Vector3 horizontalDelta = (leftFront - rightFront) / static_cast<float>(numDivide);

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

		m_instanceDatas.emplace_back(Matrix::CreateScale(RandomFloat(randMin, randMax)) * Matrix::CreateRotationY(DirectX::XMConvertToRadians(RandomFloat(0.0f, 360.0f))) * Matrix::CreateTranslation(p1));
		m_instanceDatas.emplace_back(Matrix::CreateScale(RandomFloat(randMin, randMax)) * Matrix::CreateRotationY(DirectX::XMConvertToRadians(RandomFloat(0.0f, 360.0f))) * Matrix::CreateTranslation(p2));
		m_instanceDatas.emplace_back(Matrix::CreateScale(RandomFloat(randMin, randMax)) * Matrix::CreateRotationY(DirectX::XMConvertToRadians(RandomFloat(0.0f, 360.0f))) * Matrix::CreateTranslation(p3));
		m_instanceDatas.emplace_back(Matrix::CreateScale(RandomFloat(randMin, randMax)) * Matrix::CreateRotationY(DirectX::XMConvertToRadians(RandomFloat(0.0f, 360.0f))) * Matrix::CreateTranslation(p4));
	}

	m_changed = true;
}

void FBXLoadingApp::SubShell()
{
	if (m_currentShells <= m_minShell)
	{
		return;
	}

	const size_t shellIndex = m_currentShells;
	size_t elementsToRemove = (shellIndex * 2 - 1) * 4;

	for (size_t k = 0; k < elementsToRemove; ++k)
	{
		if (m_instanceDatas.empty())
		{
			break;
		}

		m_instanceDatas.pop_back();
	}

	--m_currentShells;

	m_changed = true;
}