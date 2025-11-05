#include "ShadowMappingApp.h"

#include <cmath>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <directxtk/DDSTextureLoader.h>

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

struct WorldTransformBuffer
{
	Matrix world;
	unsigned int refBoneIndex;
	float __pad1[3];
};

struct TransformBuffer
{
	Matrix view;
	Matrix projection;
	Matrix lightView;
	Matrix lightProjection;
};

struct EnvironmentBuffer
{
	Vector3 cameraPos;
	float __pad1;
	Vector3 lightDirection;
	float __pad2;
	Vector4 lightColor;
	Vector4 ambientLightColor;
	int shadowMapSize;
	int useShadowPCF;
	float __pad3[2];
};

struct MaterialBuffer
{
	Vector4 materialAmbient;
	Vector4 materialSpecular;
	float shininess;
	float __pad1[3];
};

void ShadowMappingApp::Initialize()
{
	m_width = 1600;
	m_height = 900;

	WinApp::Initialize();

	m_camera.SetSpeed(100.0f);
	m_camera.SetPosition({ 0.0f, 40.0f, -500.0f });
	m_camera.SetFar(3000.0f);

	Material::CreateDefaultTextureSRV(m_graphicsDevice.GetDevice());

	InitializeImGui();

	InitializeScene();
}

void ShadowMappingApp::OnUpdate()
{
	m_lightRotationMatrix =
		Matrix::CreateRotationX(ToRadian(m_lightRotation.x)) *
		Matrix::CreateRotationY(ToRadian(m_lightRotation.y));

	m_lightDirection = DirectX::XMVector3TransformNormal(m_originalLightDir, m_lightRotationMatrix);
	m_lightDirection.Normalize();
	Vector3 lightUp;
	float angleX = std::fmod(m_lightRotation.x, 360.0f);
	if (angleX < 0)
	{
		lightUp = (angleX < -180.0f) ? Vector3(0.0f, 0.0f, -1.0f) : Vector3(0.0f, 0.0f, 1.0f);
	}
	else
	{
		lightUp = (angleX < 180.0f) ? Vector3(0.0f, 0.0f, -1.0f) : Vector3(0.0f, 0.0f, 1.0f);
	}

	lightUp = DirectX::XMVector3TransformNormal(lightUp, m_lightRotationMatrix);
	lightUp.Normalize();

	if (m_lightFOV == 0.0f)
	{
		m_lightFOV = 0.01f;
	}

	if (m_lightNear == 0.0f)
	{
		m_lightNear = 1.0f;
	}

	if (m_lightFar == 0.0f || m_lightFar <= m_lightNear)
	{
		m_lightFar = 1000.0f;
	}

	Vector3 focusPosition = m_camera.GetPosition() + m_camera.GetForward() * m_lightForwardDistFromCam;
	m_lightPosition = focusPosition + -m_lightDirection * m_lightFar * 0.95f;
	m_lightView = DirectX::XMMatrixLookAtLH(m_lightPosition, focusPosition, lightUp);

	m_lightProjection = DirectX::XMMatrixPerspectiveFovLH(
		ToRadian(m_lightFOV),
		static_cast<float>(m_shadowMapWidth) / m_shadowMapHeight,
		m_lightNear,
		m_lightFar);

	for (auto& mesh : m_skinningAnimMeshes)
	{
		mesh.Update(MyTime::DeltaTime());
	}

	for (auto& mesh : m_rigidAnimMeshes)
	{
		mesh.Update(MyTime::DeltaTime());
	}
}

void ShadowMappingApp::OnRender()
{
	// camera
	m_camera.GetViewMatrix(m_view);

	m_projection = DirectX::XMMatrixPerspectiveFovLH(
		ToRadian(m_camera.GetFOV()),
		static_cast<float>(m_width) / m_height,
		m_camera.GetNear(),
		m_camera.GetFar());

	// draw
	auto deviceContext = m_graphicsDevice.GetDeviceContext();
	auto renderTargetView = m_graphicsDevice.GetRenderTargetView();
	auto depthStencilView = m_graphicsDevice.GetDepthStencilView();

	// constant buffers
	TransformBuffer transformBuffer{};
	transformBuffer.view = m_view.Transpose();
	transformBuffer.projection = m_projection.Transpose();
	transformBuffer.lightView = m_lightView.Transpose();
	transformBuffer.lightProjection = m_lightProjection.Transpose();

	deviceContext->VSSetConstantBuffers(0, 1, m_transformCB.GetAddressOf());
	deviceContext->UpdateSubresource(m_transformCB.Get(), 0, nullptr, &transformBuffer, 0, 0);

	EnvironmentBuffer environmentBuffer{};
	environmentBuffer.cameraPos = m_camera.GetPosition();
	environmentBuffer.lightDirection = m_lightDirection;
	environmentBuffer.lightColor = m_lightColor;
	environmentBuffer.ambientLightColor = m_ambientLightColor;
	environmentBuffer.shadowMapSize = m_shadowMapWidth;// * m_shadowMapHeight;
	environmentBuffer.useShadowPCF = m_useShadowPCF;

	deviceContext->VSSetConstantBuffers(1, 1, m_environmentCB.GetAddressOf());
	deviceContext->PSSetConstantBuffers(1, 1, m_environmentCB.GetAddressOf());
	deviceContext->UpdateSubresource(m_environmentCB.Get(), 0, nullptr, &environmentBuffer, 0, 0);

	MaterialBuffer materialBuffer{};
	materialBuffer.materialAmbient = m_materialAmbient;
	materialBuffer.materialSpecular = m_materialSpecular;
	materialBuffer.shininess = m_shininess;

	deviceContext->PSSetConstantBuffers(2, 1, m_materialCB.GetAddressOf());
	deviceContext->UpdateSubresource(m_materialCB.Get(), 0, nullptr, &materialBuffer, 0, 0);

	deviceContext->VSSetConstantBuffers(3, 1, m_bonePoseCB.GetAddressOf());
	deviceContext->VSSetConstantBuffers(4, 1, m_boneOffsetCB.GetAddressOf());

	// common
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
	deviceContext->PSSetSamplers(1, 1, m_samplerComparisonState.GetAddressOf());
	
	D3D11_VIEWPORT shadowViewport{};
	shadowViewport.TopLeftX = 0;
	shadowViewport.TopLeftY = 0;
	shadowViewport.Width = static_cast<float>(m_shadowMapWidth);
	shadowViewport.Height = static_cast<float>(m_shadowMapHeight);
	shadowViewport.MinDepth = 0.0f;
	shadowViewport.MaxDepth = 1.0f;

	deviceContext->RSSetViewports(1, &shadowViewport);

	deviceContext->OMSetRenderTargets(0, nullptr, m_shadowMapDSV.Get());
	deviceContext->ClearDepthStencilView(m_shadowMapDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	RenderShadowMap();

	deviceContext->RSSetViewports(1, &m_graphicsDevice.GetViewport());

	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
	deviceContext->ClearRenderTargetView(renderTargetView.Get(), Color{ 1.0f, 0.0f, 1.0f, 1.0f });
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	//RenderShadowMap();
	RenderFinal();

	RenderImGui();

	m_graphicsDevice.EndDraw();
}

void ShadowMappingApp::OnShutdown()
{
	ShutdownImGui();

	Material::DestroyDefaultTextureSRV();
}

void ShadowMappingApp::RenderShadowMap()
{
	auto deviceContext = m_graphicsDevice.GetDeviceContext();

	WorldTransformBuffer worldtransformBuffer{};
	deviceContext->VSSetConstantBuffers(5, 1, m_worldTransformCB.GetAddressOf());
	deviceContext->OMSetDepthStencilState(m_shadowMapDSState.Get(), 0);

	// common pixel shader
	deviceContext->PSSetShader(m_lightViewPS.Get(), nullptr, 0);

	// staticMesh
	deviceContext->IASetInputLayout(m_commonInputLayout.Get());
	deviceContext->VSSetShader(m_basicLightViewVS.Get(), nullptr, 0);

	for (const auto& staticMesh : m_staticMeshes)
	{
		const auto& meshes = staticMesh.GetMeshes();
		const auto& materials = staticMesh.GetMaterials();

		worldtransformBuffer.world = staticMesh.GetWorld().Transpose();
		deviceContext->UpdateSubresource(m_worldTransformCB.Get(), 0, nullptr, &worldtransformBuffer, 0, 0);

		for (const auto& mesh : meshes)
		{
			auto textureSRV = materials[mesh.GetMaterialIndex()].GetTextureSRVs().opacityTextureSRV;

			deviceContext->PSSetShaderResources(0, 1, textureSRV.GetAddressOf());

			deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_commonVertexBufferStride, &m_vertexBufferOffset);
			deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

			deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
		}
	}

	// skeletalMesh (rigid)
	deviceContext->IASetInputLayout(m_commonInputLayout.Get());
	deviceContext->VSSetShader(m_rigidAnimLightViewVS.Get(), nullptr, 0);

	for (const auto& skeletalMesh : m_rigidAnimMeshes)
	{
		const auto& meshes = skeletalMesh.GetMeshes();
		const auto& materials = skeletalMesh.GetMaterials();

		deviceContext->UpdateSubresource(m_bonePoseCB.Get(), 0, nullptr, skeletalMesh.GetSkeletonPose().data(), 0, 0);

		worldtransformBuffer.world = skeletalMesh.GetWorld().Transpose();

		for (const auto& mesh : meshes)
		{
			worldtransformBuffer.refBoneIndex = mesh.GetBoneReference();
			deviceContext->UpdateSubresource(m_worldTransformCB.Get(), 0, nullptr, &worldtransformBuffer, 0, 0);

			auto textureSRV = materials[mesh.GetMaterialIndex()].GetTextureSRVs().opacityTextureSRV;

			deviceContext->PSSetShaderResources(0, 1, textureSRV.GetAddressOf());

			deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_commonVertexBufferStride, &m_vertexBufferOffset);
			deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

			deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
		}
	}

	// skeletalMesh (skinning)
	deviceContext->IASetInputLayout(m_skinningInputLayout.Get());
	deviceContext->VSSetShader(m_skinningAnimLightViewVS.Get(), nullptr, 0);

	for (const auto& skeletalMesh : m_skinningAnimMeshes)
	{
		const auto& meshes = skeletalMesh.GetMeshes();
		const auto& materials = skeletalMesh.GetMaterials();

		deviceContext->UpdateSubresource(m_bonePoseCB.Get(), 0, nullptr, skeletalMesh.GetSkeletonPose().data(), 0, 0);
		deviceContext->UpdateSubresource(m_boneOffsetCB.Get(), 0, nullptr, skeletalMesh.GetBoneOffsets().data(), 0, 0);

		worldtransformBuffer.world = skeletalMesh.GetWorld().Transpose();
		deviceContext->UpdateSubresource(m_worldTransformCB.Get(), 0, nullptr, &worldtransformBuffer, 0, 0);

		for (const auto& mesh : meshes)
		{
			auto textureSRV = materials[mesh.GetMaterialIndex()].GetTextureSRVs().opacityTextureSRV;

			deviceContext->PSSetShaderResources(0, 1, textureSRV.GetAddressOf());

			deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_boneWeightVertexBufferStride, &m_vertexBufferOffset);
			deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

			deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
		}
	}
	deviceContext->RSSetState(nullptr);
	deviceContext->OMSetDepthStencilState(nullptr, 0);
}

void ShadowMappingApp::RenderFinal()
{
	auto deviceContext = m_graphicsDevice.GetDeviceContext();

	WorldTransformBuffer worldtransformBuffer{};
	deviceContext->VSSetConstantBuffers(5, 1, m_worldTransformCB.GetAddressOf());

	// skybox
	deviceContext->IASetVertexBuffers(0, 1, m_cubeVertexBuffer.GetAddressOf(), &m_cubeVertexBufferStride, &m_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_cubeIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	deviceContext->IASetInputLayout(m_posInputLayout.Get());
	deviceContext->VSSetShader(m_skyboxVS.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_skyboxPS.Get(), nullptr, 0);
	deviceContext->PSSetShaderResources(0, 1, m_skyboxTextureSRV.GetAddressOf());
	deviceContext->RSSetState(m_skyboxRSState.Get());
	deviceContext->OMSetDepthStencilState(m_skyboxDSState.Get(), 0);

	worldtransformBuffer.world = Matrix::CreateRotationY(ToRadian(90.0f));
	deviceContext->UpdateSubresource(m_worldTransformCB.Get(), 0, nullptr, &worldtransformBuffer, 0, 0);
	deviceContext->DrawIndexed(m_cubeIndexCount, 0, 0);

	deviceContext->RSSetState(nullptr);
	deviceContext->OMSetDepthStencilState(nullptr, 0);

	// common pixel shader
	deviceContext->PSSetShader(m_blinnPhongPS.Get(), nullptr, 0);
	deviceContext->PSSetShaderResources(5, 1, m_shadowMapSRV.GetAddressOf());

	// staticMesh
	deviceContext->IASetInputLayout(m_commonInputLayout.Get());
	deviceContext->VSSetShader(m_basicVS.Get(), nullptr, 0);

	for (const auto& staticMesh : m_staticMeshes)
	{
		const auto& meshes = staticMesh.GetMeshes();
		const auto& materials = staticMesh.GetMaterials();

		worldtransformBuffer.world = staticMesh.GetWorld().Transpose();
		deviceContext->UpdateSubresource(m_worldTransformCB.Get(), 0, nullptr, &worldtransformBuffer, 0, 0);

		for (const auto& mesh : meshes)
		{
			auto textureSRVs = materials[mesh.GetMaterialIndex()].GetTextureSRVs().AsRawArray();

			deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_commonVertexBufferStride, &m_vertexBufferOffset);
			deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
			deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());

			deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
		}
	}

	// skeletalMesh (rigid)
	deviceContext->IASetInputLayout(m_commonInputLayout.Get());
	deviceContext->VSSetShader(m_rigidAnimVS.Get(), nullptr, 0);

	for (const auto& skeletalMesh : m_rigidAnimMeshes)
	{
		const auto& meshes = skeletalMesh.GetMeshes();
		const auto& materials = skeletalMesh.GetMaterials();

		deviceContext->UpdateSubresource(m_bonePoseCB.Get(), 0, nullptr, skeletalMesh.GetSkeletonPose().data(), 0, 0);

		worldtransformBuffer.world = skeletalMesh.GetWorld().Transpose();

		for (const auto& mesh : meshes)
		{
			worldtransformBuffer.refBoneIndex = mesh.GetBoneReference();
			deviceContext->UpdateSubresource(m_worldTransformCB.Get(), 0, nullptr, &worldtransformBuffer, 0, 0);

			auto textureSRVs = materials[mesh.GetMaterialIndex()].GetTextureSRVs().AsRawArray();

			deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_commonVertexBufferStride, &m_vertexBufferOffset);
			deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
			deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());

			deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
		}
	}

	// skeletalMesh (skinning)
	deviceContext->IASetInputLayout(m_skinningInputLayout.Get());
	deviceContext->VSSetShader(m_skinningAnimVS.Get(), nullptr, 0);

	for (const auto& skeletalMesh : m_skinningAnimMeshes)
	{
		const auto& meshes = skeletalMesh.GetMeshes();
		const auto& materials = skeletalMesh.GetMaterials();

		deviceContext->UpdateSubresource(m_bonePoseCB.Get(), 0, nullptr, skeletalMesh.GetSkeletonPose().data(), 0, 0);
		deviceContext->UpdateSubresource(m_boneOffsetCB.Get(), 0, nullptr, skeletalMesh.GetBoneOffsets().data(), 0, 0);

		worldtransformBuffer.world = skeletalMesh.GetWorld().Transpose();
		deviceContext->UpdateSubresource(m_worldTransformCB.Get(), 0, nullptr, &worldtransformBuffer, 0, 0);

		for (const auto& mesh : meshes)
		{
			auto textureSRVs = materials[mesh.GetMaterialIndex()].GetTextureSRVs().AsRawArray();

			deviceContext->IASetVertexBuffers(0, 1, mesh.GetVertexBuffer().GetAddressOf(), &m_boneWeightVertexBufferStride, &m_vertexBufferOffset);
			deviceContext->IASetIndexBuffer(mesh.GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
			deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());

			deviceContext->DrawIndexed(mesh.GetIndexCount(), 0, 0);
		}
	}
	ID3D11ShaderResourceView* nullSRV[]{ nullptr };
	deviceContext->PSSetShaderResources(5, 1, nullSRV);
}

void ShadowMappingApp::RenderImGui()
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

	ImGui::ColorEdit3("Ambient", &m_materialAmbient.x);
	ImGui::ColorEdit3("Specular", &m_materialSpecular.x);
	ImGui::DragFloat("Shininess", &m_shininess, 5.0f, 1.0f, 10000.0f);

	if (ImGui::Button("Reset##2"))
	{
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
	ImGui::DragFloat("LightNear", &m_lightNear);
	ImGui::DragFloat("LightFar", &m_lightFar);
	ImGui::DragFloat("LightFOV", &m_lightFOV, 0.0001f);
	ImGui::DragFloat("ForwardFromCam", &m_lightForwardDistFromCam, 0.1f);
	
	if (ImGui::Button("Reset##3"))
	{
		m_lightRotation = { -40.0f, 25.0f, 0.0f };
		m_lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		m_ambientLightColor = { 0.1f, 0.1f, 0.1f, 1.0f };
	}

	ImGui::NewLine();
	ImGui::SeparatorText("Info");
	ImGui::Text("%d FPS", GetLastFPS());
	ImGui::Checkbox("Use Shadow PCF", &m_useShadowPCF);
	ImGui::Image((ImTextureID)(intptr_t)m_shadowMapSRV.Get(), ImVec2(300.0f, 300.0f));
	ImGui::End();

	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ShadowMappingApp::InitializeImGui()
{
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_graphicsDevice.GetDevice().Get(), m_graphicsDevice.GetDeviceContext().Get());
}

void ShadowMappingApp::InitializeScene()
{
	auto device = m_graphicsDevice.GetDevice();

	const std::string staticFbxFileNames[]{
		"Teapot.fbx",
		"zeldaPosed001.fbx",
		"Floor.fbx",
		"Tree.fbx",
	};

	const std::string skeletalFbxFileNames[]{
		"SkinningTest.fbx",
		"Zombie_Run.fbx",
		"1CubeAnim.fbx",
		"BouncingBall.fbx",
		"BoxHuman.fbx",
	};

	const size_t numFBXs = ARRAYSIZE(skeletalFbxFileNames) + ARRAYSIZE(staticFbxFileNames) - 1 /* ¹Ù´Ú -1 */;
	const float radian = DirectX::XM_2PI / numFBXs;
	const float radius = 300.0f;

	size_t index = 0;

	// create static mesh
	for (size_t i = 0; i < ARRAYSIZE(staticFbxFileNames); ++i)
	{
		Vector3 position{ radius * std::cos(radian * index), 0.0f, radius * std::sin(radian * index) };

		Matrix world;

		if (staticFbxFileNames[i] == "Teapot.fbx")
		{
			world = Matrix::CreateTranslation(position + Vector3(0.0f, 50.0f, 0.0f));
		}
		else if (staticFbxFileNames[i] == "Floor.fbx")
		{
			world = Matrix::CreateTranslation(0.0f, 0.0f, 0.0f);
			--index;
		}
		else if (staticFbxFileNames[i] == "zeldaPosed001.fbx")
		{
			world = Matrix::CreateTranslation(position + Vector3(0.0f, 10.0f, 0.0f));
		}
		else
		{
			world = Matrix::CreateTranslation(position);
		}

		m_staticMeshes.emplace_back(device, staticFbxFileNames[i], world);

		++index;
	}

	// create skeletal mesh
	for (size_t i = 0; i < ARRAYSIZE(skeletalFbxFileNames); ++i)
	{
		Vector3 position{ radius * std::cos(radian * index), 0.0f, radius * std::sin(radian * index) };

		Matrix world;

		if (skeletalFbxFileNames[i] == "BoxHuman.fbx")
		{
			world = Matrix::CreateScale(0.25f) * Matrix::CreateRotationY(ToRadian(180.0f)) * Matrix::CreateTranslation(position);
		}
		else if (skeletalFbxFileNames[i] == "BouncingBall.fbx")
		{
			world = Matrix::CreateScale(10.0f) * Matrix::CreateTranslation(position);
		}
		else if (skeletalFbxFileNames[i] == "1CubeAnim.fbx")
		{
			world = Matrix::CreateScale(0.25f) * Matrix::CreateTranslation(position + Vector3(0.0f, 40.0f, 0.0f));
		}
		else if (skeletalFbxFileNames[i] == "BouncingBall.fbx")
		{
			world = Matrix::CreateScale(5.0f) * Matrix::CreateTranslation(position);
		}
		else
		{
			world = Matrix::CreateTranslation(position);
		}

		SkeletalMesh mesh{ device, skeletalFbxFileNames[i], world };
		if (mesh.IsRigid())
		{
			m_rigidAnimMeshes.push_back(std::move(mesh));
			m_rigidAnimMeshes.back().PlayAnimation(0);
		}
		else
		{
			m_skinningAnimMeshes.push_back(std::move(mesh));
			m_skinningAnimMeshes.back().PlayAnimation(0);
		}

		++index;
	}

	// skinning
	{
		m_boneWeightVertexBufferStride = sizeof(BoneWeightVertex);

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

		{
			ComPtr<ID3DBlob> vertexShaderBuffer;
			GraphicsDevice::CompileShaderFromFile(L"SkinningAnimVS.hlsl", "main", "vs_5_0", vertexShaderBuffer);

			device->CreateInputLayout(
				layout,
				ARRAYSIZE(layout),
				vertexShaderBuffer->GetBufferPointer(),
				vertexShaderBuffer->GetBufferSize(),
				&m_skinningInputLayout
			);

			device->CreateVertexShader(
				vertexShaderBuffer->GetBufferPointer(),
				vertexShaderBuffer->GetBufferSize(),
				nullptr,
				&m_skinningAnimVS
			);
		}

		{
			// lightView
			ComPtr<ID3DBlob> vertexShaderBuffer;
			GraphicsDevice::CompileShaderFromFile(L"SkinningAnimLightViewVS.hlsl", "main", "vs_5_0", vertexShaderBuffer);

			device->CreateVertexShader(
				vertexShaderBuffer->GetBufferPointer(),
				vertexShaderBuffer->GetBufferSize(),
				nullptr,
				&m_skinningAnimLightViewVS
			);
		}
	}

	// common
	{
		m_commonVertexBufferStride = sizeof(Vertex);

		D3D11_INPUT_ELEMENT_DESC layout[]{
			// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		{
			ComPtr<ID3DBlob> vertexShaderBuffer;
			GraphicsDevice::CompileShaderFromFile(L"RigidAnimVS.hlsl", "main", "vs_5_0", vertexShaderBuffer);

			device->CreateInputLayout(
				layout,
				ARRAYSIZE(layout),
				vertexShaderBuffer->GetBufferPointer(),
				vertexShaderBuffer->GetBufferSize(),
				&m_commonInputLayout
			);

			device->CreateVertexShader(
				vertexShaderBuffer->GetBufferPointer(),
				vertexShaderBuffer->GetBufferSize(),
				nullptr,
				&m_rigidAnimVS
			);
		}

		{
			ComPtr<ID3DBlob> vertexShaderBuffer;
			GraphicsDevice::CompileShaderFromFile(L"BasicVS.hlsl", "main", "vs_5_0", vertexShaderBuffer);

			device->CreateVertexShader(
				vertexShaderBuffer->GetBufferPointer(),
				vertexShaderBuffer->GetBufferSize(),
				nullptr,
				&m_basicVS
			);
		}

		{
			ComPtr<ID3DBlob> vertexShaderBuffer;
			GraphicsDevice::CompileShaderFromFile(L"BasicLightViewVS.hlsl", "main", "vs_5_0", vertexShaderBuffer);

			device->CreateVertexShader(
				vertexShaderBuffer->GetBufferPointer(),
				vertexShaderBuffer->GetBufferSize(),
				nullptr,
				&m_basicLightViewVS
			);
		}

		{
			ComPtr<ID3DBlob> vertexShaderBuffer;
			GraphicsDevice::CompileShaderFromFile(L"RigidAnimLightViewVS.hlsl", "main", "vs_5_0", vertexShaderBuffer);

			device->CreateVertexShader(
				vertexShaderBuffer->GetBufferPointer(),
				vertexShaderBuffer->GetBufferSize(),
				nullptr,
				&m_rigidAnimLightViewVS
			);
		}
	}

	// cube
	{
		m_cubeVertexBufferStride = sizeof(PosVertex);

		PosVertex vertices[] =
		{
			// position
			{ { -0.5f,  0.5f, -0.5f } }, // 0 - 0
			{ {  0.5f,  0.5f, -0.5f } }, // 1 - 1
			{ { -0.5f, -0.5f, -0.5f } }, // 2 - 2
			{ {  0.5f, -0.5f, -0.5f } }, // 3 - 3

			{ {  0.5f,  0.5f,  0.5f } }, // 4 - 4
			{ { -0.5f,  0.5f,  0.5f } }, // 5 - 5
			{ {  0.5f, -0.5f,  0.5f } }, // 6 - 6
			{ { -0.5f, -0.5f,  0.5f } }, // 7 - 7

			{ { -0.5f,  0.5f,  0.5f } }, // 5 - 8
			{ { -0.5f,  0.5f, -0.5f } }, // 0 - 9
			{ { -0.5f, -0.5f,  0.5f } }, // 7 - 10
			{ { -0.5f, -0.5f, -0.5f } }, // 2 - 11

			{ {  0.5f,  0.5f, -0.5f } }, // 1 - 12
			{ {  0.5f,  0.5f,  0.5f } }, // 4 - 13
			{ {  0.5f, -0.5f, -0.5f } }, // 3 - 14
			{ {  0.5f, -0.5f,  0.5f } }, // 6 - 15

			{ { -0.5f, -0.5f, -0.5f } }, // 2 - 16
			{ {  0.5f, -0.5f, -0.5f } }, // 3 - 17
			{ { -0.5f, -0.5f,  0.5f } }, // 7 - 18
			{ {  0.5f, -0.5f,  0.5f } }, // 6 - 19

			{ { -0.5f,  0.5f,  0.5f } }, // 5 - 20
			{ {  0.5f,  0.5f,  0.5f } }, // 4 - 21
			{ { -0.5f,  0.5f, -0.5f } }, // 0 - 22
			{ {  0.5f,  0.5f, -0.5f } }, // 1 - 23
		};

		D3D11_BUFFER_DESC vertexBufferDesc{};
		vertexBufferDesc.ByteWidth = sizeof(PosVertex) * ARRAYSIZE(vertices);
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA vertexBufferData{};
		vertexBufferData.pSysMem = vertices;

		device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_cubeVertexBuffer);

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

		m_cubeIndexCount = ARRAYSIZE(indices);

		D3D11_BUFFER_DESC indexBufferDesc{};
		indexBufferDesc.ByteWidth = sizeof(WORD) * m_cubeIndexCount;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA indexBufferData{};
		indexBufferData.pSysMem = indices;
		device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_cubeIndexBuffer);


		D3D11_INPUT_ELEMENT_DESC layout[]{
			// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ComPtr<ID3DBlob> vertexShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"SkyboxVS.hlsl", "main", "vs_5_0", vertexShaderBuffer);

		device->CreateInputLayout(
			layout,
			ARRAYSIZE(layout),
			vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(),
			&m_posInputLayout
		);

		device->CreateVertexShader(
			vertexShaderBuffer->GetBufferPointer(),
			vertexShaderBuffer->GetBufferSize(),
			nullptr,
			&m_skyboxVS
		);

		ComPtr<ID3DBlob> pixelShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"SkyboxPS.hlsl", "main", "ps_5_0", pixelShaderBuffer);

		device->CreatePixelShader(
			pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(),
			nullptr,
			&m_skyboxPS
		);

		DirectX::CreateDDSTextureFromFile(device.Get(), L"cloudy_skybox.dds", nullptr, &m_skyboxTextureSRV);

		D3D11_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.DepthClipEnable = TRUE;
		rasterizerDesc.FrontCounterClockwise = TRUE;

		device->CreateRasterizerState(&rasterizerDesc, &m_skyboxRSState);


		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		device->CreateDepthStencilState(&depthStencilDesc, &m_skyboxDSState);
	}

	// blinn-phong pixel shader
	{
		ComPtr<ID3DBlob> pixelShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"BlinnPhongPS.hlsl", "main", "ps_5_0", pixelShaderBuffer);

		device->CreatePixelShader(
			pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(),
			nullptr,
			&m_blinnPhongPS
		);
	}

	// skybox pixel shader
	{
		ComPtr<ID3DBlob> pixelShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"SkyboxPS.hlsl", "main", "ps_5_0", pixelShaderBuffer);

		device->CreatePixelShader(
			pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(),
			nullptr,
			&m_skyboxPS
		);
	}

	// lightview pixel shader
	{
		ComPtr<ID3DBlob> pixelShaderBuffer;
		GraphicsDevice::CompileShaderFromFile(L"LightViewPS.hlsl", "main", "ps_5_0", pixelShaderBuffer);

		device->CreatePixelShader(
			pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(),
			nullptr,
			&m_lightViewPS
		);
	}

	// shadow mapping
	{
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = static_cast<UINT>(m_shadowMapWidth);
		texDesc.Height = static_cast<UINT>(m_shadowMapWidth);
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;

		device->CreateTexture2D(&texDesc, nullptr, &m_shadowMap);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		device->CreateDepthStencilView(m_shadowMap.Get(), &dsvDesc, &m_shadowMapDSV);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		device->CreateShaderResourceView(m_shadowMap.Get(), &srvDesc, &m_shadowMapSRV);

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

		device->CreateDepthStencilState(&depthStencilDesc, &m_shadowMapDSState);
	}

	// constant buffer
	D3D11_BUFFER_DESC constantBufferDesc{};
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	constantBufferDesc.ByteWidth = sizeof(WorldTransformBuffer);
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_worldTransformCB);

	constantBufferDesc.ByteWidth = sizeof(TransformBuffer);
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_transformCB);

	constantBufferDesc.ByteWidth = sizeof(EnvironmentBuffer);
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_environmentCB);

	constantBufferDesc.ByteWidth = sizeof(MaterialBuffer);
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_materialCB);

	constantBufferDesc.ByteWidth = sizeof(Matrix) * MAX_BONE_NUM;
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_bonePoseCB);

	constantBufferDesc.ByteWidth = sizeof(Matrix) * MAX_BONE_NUM;
	device->CreateBuffer(&constantBufferDesc, nullptr, &m_boneOffsetCB);

	// linear sampler
	{
		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		device->CreateSamplerState(&samplerDesc, &m_samplerState);
	}

	// comparison sampler
	{
		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

		device->CreateSamplerState(&samplerDesc, &m_samplerComparisonState);
	}
}

void ShadowMappingApp::ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

LRESULT ShadowMappingApp::MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	return WinApp::MessageProc(hWnd, uMsg, wParam, lParam);
}