#include "PBRApp.h"

#include <cmath>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <directxtk/DDSTextureLoader.h>
#include <sstream>
#include <iomanip>

#include <psapi.h>                // GetProcessMemoryInfo, PROCESS_MEMORY_COUNTERS_EX
#pragma comment(lib, "psapi.lib")

#include "../Common/MyTime.h"
#include "../Common/Helper.h"
#include "../Common/Vertex.h"
#include "../Common/D3DResourceManager.h"
#include "../Common/Input.h"
#include "../Common/Texture2D.h"
#include "../Common/DepthStencilView.h"
#include "../Common/ShaderResourceView.h"
#include "../Common/RasterizerState.h"
#include "../Common/DepthStencilState.h"
#include "../Common/DebugDraw.h"

#include "StaticMesh.h"

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::string FormatBytes(UINT64 bytes) {
	constexpr double KB = 1024.0;
	constexpr double MB = KB * 1024.0;
	constexpr double GB = MB * 1024.0;

	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2);

	if (bytes >= static_cast<UINT64>(GB))
		oss << (bytes / GB) << " GB";
	else if (bytes >= static_cast<UINT64>(MB))
		oss << (bytes / MB) << " MB";
	else if (bytes >= static_cast<UINT64>(KB))
		oss << (bytes / KB) << " KB";
	else
		oss << bytes << " B";

	return oss.str();
}

void PBRApp::Initialize()
{
	m_width = 1600;
	m_height = 900;

	WinApp::Initialize();

	D3DResourceManager::Get().SetGraphicsDevice(&m_graphicsDevice);

	auto device = m_graphicsDevice.GetDevice();
	auto context = m_graphicsDevice.GetDeviceContext();

	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	device.As(&m_dxgiDevice);
	m_dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
	dxgiAdapter.As(&m_dxgiAdapter);

	m_camera.SetSpeed(100.0f);
	m_camera.SetPosition({ 0.0f, 40.0f, -500.0f });
	m_camera.SetFar(10000.0f);

	InitializeImGui();

	InitializeScene();


	m_states = std::make_unique<DirectX::CommonStates>(device.Get());
	m_batch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context.Get());

	m_effect = std::make_unique<DirectX::BasicEffect>(device.Get());
	m_effect->SetVertexColorEnabled(true);
	m_effect->SetView(m_view);
	m_effect->SetProjection(m_projection);

	{
		void const* shaderByteCode;
		size_t byteCodeLength;

		m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

		device->CreateInputLayout(
			DirectX::VertexPositionColor::InputElements, DirectX::VertexPositionColor::InputElementCount,
			shaderByteCode, byteCodeLength,
			m_inputLayout.ReleaseAndGetAddressOf());
	}
}

void PBRApp::OnUpdate()
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


	//m_lightFOV = std::min<float>(std::max<float>(m_camera.GetPosition().y / 100.0f, 1.0f), 5.0f);

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

	Vector3 focusPosition = m_camera.GetPosition() + m_camera.GetForward() *
		(m_camera.GetPosition().y < 1000.0f ? m_camera.GetPosition().y : (m_lightForwardDistFromCam + m_camera.GetPosition().y));
	m_lightPosition = focusPosition + -m_lightDirection * m_lightFar * 0.92f;
	m_lightView = DirectX::XMMatrixLookAtLH(m_lightPosition, focusPosition, lightUp);

	m_lightProjection = DirectX::XMMatrixPerspectiveFovLH(
		ToRadian(m_lightFOV),
		static_cast<float>(m_shadowMapWidth) / m_shadowMapHeight,
		m_lightNear,
		m_lightFar);

	for (auto& mesh : m_skeletalMeshes)
	{
		mesh.Update(MyTime::DeltaTime());
	}
}

void PBRApp::OnRender()
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

	deviceContext->UpdateSubresource(m_transformBuffer->GetRawBuffer(), 0, nullptr, &transformBuffer, 0, 0);

	EnvironmentBuffer environmentBuffer{};
	environmentBuffer.cameraPos = m_camera.GetPosition();
	environmentBuffer.lightDirection = m_lightDirection;
	environmentBuffer.lightColor = m_lightColor;
	environmentBuffer.ambientLightColor = m_ambientLightColor;
	environmentBuffer.shadowMapSize = m_shadowMapWidth;// * m_shadowMapHeight;
	environmentBuffer.useShadowPCF = m_useShadowPCF;

	deviceContext->UpdateSubresource(m_environmentBuffer->GetRawBuffer(), 0, nullptr, &environmentBuffer, 0, 0);

	deviceContext->UpdateSubresource(m_overrideMatBuffer->GetRawBuffer(), 0, nullptr, &m_overrideMaterialCB, 0, 0);

	// common
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	// shadow
	D3D11_VIEWPORT shadowViewport{};
	shadowViewport.TopLeftX = 0;
	shadowViewport.TopLeftY = 0;
	shadowViewport.Width = static_cast<float>(m_shadowMapWidth);
	shadowViewport.Height = static_cast<float>(m_shadowMapHeight);
	shadowViewport.MinDepth = 0.0f;
	shadowViewport.MaxDepth = 1.0f;

	deviceContext->RSSetViewports(1, &shadowViewport);

	deviceContext->OMSetRenderTargets(0, nullptr, m_shadowMapDSV->GetRawDepthStencilView());
	deviceContext->ClearDepthStencilView(m_shadowMapDSV->GetRawDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	RenderShadowMap();

	// final
	deviceContext->RSSetViewports(1, &m_graphicsDevice.GetViewport());

	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
	deviceContext->ClearRenderTargetView(renderTargetView.Get(), Color{ 0.5f, 0.8f, 1.0f, 1.0f });
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	RenderFinal();

	//deviceContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	////deviceContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
	//deviceContext->RSSetState(m_states->CullNone());

	//m_effect->SetView(m_view);
	//m_effect->SetProjection(m_projection);
	//m_effect->Apply(deviceContext.Get());

	//deviceContext->IASetInputLayout(m_inputLayout.Get());
	//m_batch->Begin();

	//// Frustum을 월드 공간에 배치
	//DirectX::BoundingFrustum frustum(m_lightProjection);
	//Matrix lightWorldMatrix = m_lightView.Invert();
	//frustum.Transform(frustum, lightWorldMatrix);

	//DX::Draw(m_batch.get(), frustum, DirectX::Colors::Blue);

	//m_batch->End();

	RenderImGui();

	m_graphicsDevice.EndDraw();
}

void PBRApp::OnShutdown()
{
	ShutdownImGui();
}

void PBRApp::RenderShadowMap()
{
	const auto& deviceContext = m_graphicsDevice.GetDeviceContext();

	deviceContext->OMSetDepthStencilState(m_shadowMapDSS->GetRawDepthStencilState(), 0);
	deviceContext->RSSetState(m_shadowMapRSS->GetRawRasterizerState());

	for (auto& mesh : m_staticMeshes)
	{
		mesh.DrawShadowMap(deviceContext);
	}

	for (auto& mesh : m_skeletalMeshes)
	{
		mesh.DrawShadowMap(deviceContext);
	}

	deviceContext->RSSetState(nullptr);
	deviceContext->OMSetDepthStencilState(nullptr, 0);
}

void PBRApp::RenderFinal()
{
	const auto& deviceContext = m_graphicsDevice.GetDeviceContext();

	deviceContext->PSSetShaderResources(7, 1, m_shadowMapSRV->GetShaderResourceView().GetAddressOf());

	for (auto& mesh : m_staticMeshes)
	{
		mesh.Draw(deviceContext);
	}

	for (auto& mesh : m_skeletalMeshes)
	{
		mesh.Draw(deviceContext);
	}

	ID3D11ShaderResourceView* nullSRV[]{ nullptr };
	deviceContext->PSSetShaderResources(7, 1, nullSRV);
}

void PBRApp::RenderImGui()
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
	if (ImGui::Checkbox("Override Material", &m_overrideMaterial))
	{
		if (m_overrideMaterial)
		{
			m_overrideMaterialCB.overrideMaterial = 1;
		}
		else
		{
			m_overrideMaterialCB.overrideMaterial = 0;
		}
	}
	ImGui::ColorEdit3("BaseColor", &m_overrideMaterialCB.baseColor.x);
	ImGui::SliderFloat("Metalness", &m_overrideMaterialCB.metalness, 0.0f, 1.0f);
	ImGui::SliderFloat("Roughness", &m_overrideMaterialCB.roughness, 0.0f, 1.0f);


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

	ImGui::Checkbox("Use Shadow PCF", &m_useShadowPCF);
	ImGui::Image((ImTextureID)(intptr_t)m_shadowMapSRV->GetRawShaderResourceView(), ImVec2(300.0f, 300.0f));

	ImGui::Text("%d FPS", GetLastFPS());

	DXGI_QUERY_VIDEO_MEMORY_INFO memInfo = {};
	m_dxgiAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo);
	ImGui::Text("VRAM: %s", FormatBytes(memInfo.CurrentUsage).c_str());

	HANDLE hProcess = GetCurrentProcess();
	PROCESS_MEMORY_COUNTERS_EX pmc;
	pmc.cb = sizeof(PROCESS_MEMORY_COUNTERS_EX);

	// 현재 프로세스의 메모리 사용 정보 조회
	GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
	ImGui::Text("DRAM: %s", FormatBytes(pmc.WorkingSetSize).c_str());
	ImGui::Text("PageFile: %s", FormatBytes(pmc.PagefileUsage - pmc.WorkingSetSize).c_str());
	if (ImGui::Button("Trim"))
	{
		m_dxgiDevice->Trim();
	}

	if (ImGui::Button("Add Static Mesh"))
	{
		auto size = m_staticMeshes.size();
		m_staticMeshes.emplace_back(L"zeldaPosed001.fbx");

		float x = (size % 5) * 200.0f + 100.0f;
		float z = (size / 5) * 200.0f;

		m_staticMeshes.back().SetWorld(DirectX::SimpleMath::Matrix::CreateTranslation(x, 0.0f, z).Transpose());
	}

	if (ImGui::Button("Remove Static Mesh"))
	{
		if (!m_staticMeshes.empty())
		{
			m_staticMeshes.pop_back();
		}
	}

	if (ImGui::Button("Add Skeletal Mesh"))
	{
		auto size = m_skeletalMeshes.size();
		m_skeletalMeshes.emplace_back(L"SkinningTest.fbx");

		float x = (size % 5) * -200.0f - 100.0f;
		float z = (size / 5) * 200.0f;

		m_skeletalMeshes.back().PlayAnimation(0);
		m_skeletalMeshes.back().SetWorld(DirectX::SimpleMath::Matrix::CreateTranslation(x, 0.0f, z).Transpose());
	}

	if (ImGui::Button("Remove Skeletal Mesh"))
	{
		if (!m_skeletalMeshes.empty())
		{
			m_skeletalMeshes.pop_back();
		}
	}

	ImGui::End();

	ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void PBRApp::InitializeImGui()
{
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_graphicsDevice.GetDevice().Get(), m_graphicsDevice.GetDeviceContext().Get());
}

void PBRApp::InitializeScene()
{
	const auto& device = m_graphicsDevice.GetDevice();
	auto deviceContext = m_graphicsDevice.GetDeviceContext();

	m_transformBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"Transform", sizeof(TransformBuffer));
	m_environmentBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"Environment", sizeof(EnvironmentBuffer));
	m_overrideMatBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"OverrideMat", sizeof(OverrideMaterial));

	deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::Transform), 1, m_transformBuffer->GetBuffer().GetAddressOf());
	deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::Environment), 1, m_environmentBuffer->GetBuffer().GetAddressOf());
	deviceContext->PSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::Environment), 1, m_environmentBuffer->GetBuffer().GetAddressOf());
	deviceContext->PSSetConstantBuffers(6, 1, m_overrideMatBuffer->GetBuffer().GetAddressOf());

	m_staticMeshes.emplace_back(L"char.fbx", L"PBRPS.hlsl");
	m_staticMeshes.back().SetWorld(DirectX::SimpleMath::Matrix::CreateTranslation(0.0f, 30.0f, 0.0f).Transpose());

	m_staticMeshes.emplace_back(L"machete_1k.fbx", L"PBRPS.hlsl");
	m_staticMeshes.back().SetWorld(DirectX::SimpleMath::Matrix::CreateTranslation(100.0f, 30.0f, 0.0f).Transpose());

	m_staticMeshes.emplace_back(L"wooden_axe_1k.fbx", L"PBRPS.hlsl");
	m_staticMeshes.back().SetWorld(DirectX::SimpleMath::Matrix::CreateTranslation(-100.0f, 30.0f, 0.0f).Transpose());

	m_staticMeshes.emplace_back(L"brass_goblets_1k.fbx", L"PBRPS.hlsl");
	m_staticMeshes.back().SetWorld(DirectX::SimpleMath::Matrix::CreateTranslation(-200.0f, 30.0f, 0.0f).Transpose());

	m_staticMeshes.emplace_back(L"cannon_01_1k.fbx", L"PBRPS.hlsl");
	m_staticMeshes.back().SetWorld(DirectX::SimpleMath::Matrix::CreateTranslation(200.0f, 30.0f, 0.0f).Transpose());

	m_staticMeshes.emplace_back(L"mid_century_lounge_chair_1k.fbx", L"PBRPS.hlsl");
	m_staticMeshes.back().SetWorld(DirectX::SimpleMath::Matrix::CreateTranslation(-300.0f, 30.0f, 0.0f).Transpose());

	m_staticMeshes.emplace_back(L"vintage_video_camera_1k.fbx", L"PBRPS.hlsl");
	m_staticMeshes.back().SetWorld(DirectX::SimpleMath::Matrix::CreateTranslation(300.0f, 30.0f, 0.0f).Transpose());

	m_staticMeshes.emplace_back(L"Floor.fbx");

	//m_skeletalMeshes.emplace_back(L"SkinningTest.fbx");
	//m_skeletalMeshes.back().PlayAnimation(0);
	//m_skeletalMeshes.back().SetWorld(DirectX::SimpleMath::Matrix::CreateTranslation(-100.0f, 0.0f, 0.0f).Transpose());

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

		m_shadowMapTex2D = D3DResourceManager::Get().GetOrCreateTexture2D(L"ShadowMap", texDesc);

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		m_shadowMapDSV = D3DResourceManager::Get().GetOrCreateDepthStencilView(L"ShadowMap", m_shadowMapTex2D->GetTexture2D(), dsvDesc);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		m_shadowMapSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(L"ShadowMap", m_shadowMapTex2D->GetTexture2D(), srvDesc);

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

		m_shadowMapDSS = D3DResourceManager::Get().GetOrCreateDepthStencilState(L"ShadowMap", depthStencilDesc);

		D3D11_RASTERIZER_DESC rsDesc = {};
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.CullMode = D3D11_CULL_BACK;
		rsDesc.DepthBias = 5000;
		rsDesc.DepthBiasClamp = 0.0f;
		rsDesc.SlopeScaledDepthBias = 2.0f;
		rsDesc.DepthClipEnable = true;

		m_shadowMapRSS = D3DResourceManager::Get().GetOrCreateRasterizerState(L"ShadowMap", rsDesc);
	}
}

void PBRApp::ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

LRESULT PBRApp::MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	return WinApp::MessageProc(hWnd, uMsg, wParam, lParam);
}