#include "ResourceManagerApp.h"

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

void ResourceManagerApp::Initialize()
{
	m_width = 1600;
	m_height = 900;

	WinApp::Initialize();

	D3DResourceManager::Get().SetGraphicsDevice(&m_graphicsDevice);

	const auto& device = m_graphicsDevice.GetDevice();

	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	device.As(&m_dxgiDevice);
	m_dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());
	dxgiAdapter.As(&m_dxgiAdapter);

	m_camera.SetSpeed(100.0f);
	m_camera.SetPosition({ 0.0f, 40.0f, -500.0f });
	m_camera.SetFar(10000.0f);

	InitializeImGui();

	InitializeScene();
}

void ResourceManagerApp::OnUpdate()
{
	m_lightRotationMatrix =
		Matrix::CreateRotationX(ToRadian(m_lightRotation.x)) *
		Matrix::CreateRotationY(ToRadian(m_lightRotation.y));

	m_lightDirection = DirectX::XMVector3TransformNormal(m_originalLightDir, m_lightRotationMatrix);
	m_lightDirection.Normalize();

	for (auto& mesh : m_skeletalMeshes)
	{
		mesh.Update(MyTime::DeltaTime());
	}
}

void ResourceManagerApp::OnRender()
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
	environmentBuffer.useShadowPCF = m_useShadowPCF;

	deviceContext->UpdateSubresource(m_environmentBuffer->GetRawBuffer(), 0, nullptr, &environmentBuffer, 0, 0);

	// common
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	deviceContext->RSSetViewports(1, &m_graphicsDevice.GetViewport());

	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
	deviceContext->ClearRenderTargetView(renderTargetView.Get(), Color{ 0.5f, 0.8f, 1.0f, 1.0f });
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	RenderFinal();

	RenderImGui();

	m_graphicsDevice.EndDraw();
}

void ResourceManagerApp::OnShutdown()
{
	ShutdownImGui();
}
void ResourceManagerApp::RenderFinal()
{
	const auto& deviceContext = m_graphicsDevice.GetDeviceContext();

	for (auto& mesh : m_staticMeshes)
	{
		mesh.Draw(deviceContext);
	}

	for (auto& mesh : m_skeletalMeshes)
	{
		mesh.Draw(deviceContext);
	}
}

void ResourceManagerApp::RenderImGui()
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

void ResourceManagerApp::InitializeImGui()
{
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_graphicsDevice.GetDevice().Get(), m_graphicsDevice.GetDeviceContext().Get());
}

void ResourceManagerApp::InitializeScene()
{
	const auto& device = m_graphicsDevice.GetDevice();
	auto deviceContext = m_graphicsDevice.GetDeviceContext();

	m_transformBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"Transform", sizeof(TransformBuffer));
	m_environmentBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"Environment", sizeof(EnvironmentBuffer));

	deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::Transform), 1, m_transformBuffer->GetBuffer().GetAddressOf());
	deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::Environment), 1, m_environmentBuffer->GetBuffer().GetAddressOf());
	deviceContext->PSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::Environment), 1, m_environmentBuffer->GetBuffer().GetAddressOf());

	m_staticMeshes.emplace_back(L"zeldaPosed001.fbx");
	m_staticMeshes.back().SetWorld(DirectX::SimpleMath::Matrix::CreateTranslation(100.0f, 0.0f, 0.0f).Transpose());

	m_skeletalMeshes.emplace_back(L"SkinningTest.fbx");
	m_skeletalMeshes.back().PlayAnimation(0);
	m_skeletalMeshes.back().SetWorld(DirectX::SimpleMath::Matrix::CreateTranslation(-100.0f, 0.0f, 0.0f).Transpose());
}

void ResourceManagerApp::ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

LRESULT ResourceManagerApp::MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	return WinApp::MessageProc(hWnd, uMsg, wParam, lParam);
}