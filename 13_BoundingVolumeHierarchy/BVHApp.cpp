#include "BVHApp.h"

#include <cmath>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <directxtk/DDSTextureLoader.h>

#include "../Common/MyTime.h"
#include "../Common/Helper.h"
#include "../Common/Vertex.h"
#include "../Common/D3DResourceManager.h"
#include "../Common/Input.h"
#include "../Common/DebugDraw.h"
#include "../Common/ShaderConstant.h"
#include "../Common/InputLayout.h"
#include "../Common/VertexBuffer.h"
#include "../Common/IndexBuffer.h"
#include "../Common/VertexShader.h"
#include "../Common/PixelShader.h"


using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct SimpleConstant
{
	Matrix world;
	Matrix view;
	Matrix projection;
	Vector3 lightDir;
	float __pad1;
	Vector3 lightColor;
	float __pad2;
	Vector3 ambientColor;
	float __pad3;
};

void BVHApp::Initialize()
{
	m_width = 1600;
	m_height = 900;

	WinApp::Initialize();

	D3DResourceManager::Get().SetGraphicsDevice(&m_graphicsDevice);

	auto device = m_graphicsDevice.GetDevice();
	auto context = m_graphicsDevice.GetDeviceContext();

	m_camera.SetSpeed(10.0f);
	m_camera.SetPosition({ 0.0f, 0.0f, -10.0f });
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

void BVHApp::OnUpdate()
{
	m_lightRotationMatrix =
		Matrix::CreateRotationX(ToRadian(m_lightRotation.x)) *
		Matrix::CreateRotationY(ToRadian(m_lightRotation.y));

	m_lightDirection = DirectX::XMVector3TransformNormal(m_originalLightDir, m_lightRotationMatrix);
	m_lightDirection.Normalize();

	
	auto& info = *(m_cubeInfo.begin());
	float speed = 25.0f;
	if (Input::IsKeyHeld(DirectX::Keyboard::Keys::Up))
	{
		info.position.z += speed * MyTime::DeltaTime();
		m_changed = true;
	}

	if (Input::IsKeyHeld(DirectX::Keyboard::Keys::Down))
	{
		info.position.z -= speed * MyTime::DeltaTime();
		m_changed = true;
	}

	if (Input::IsKeyHeld(DirectX::Keyboard::Keys::Left))
	{
		info.position.x -= speed * MyTime::DeltaTime();
		m_changed = true;
	}

	if (Input::IsKeyHeld(DirectX::Keyboard::Keys::Right))
	{
		info.position.x += speed * MyTime::DeltaTime();
		m_changed = true;
	}

	if (Input::IsKeyHeld(DirectX::Keyboard::Keys::PageUp))
	{
		info.position.y += speed * MyTime::DeltaTime();
		m_changed = true;
	}

	if (Input::IsKeyHeld(DirectX::Keyboard::Keys::PageDown))
	{
		info.position.y -= speed * MyTime::DeltaTime();
		m_changed = true;
	}

	if (m_changed)
	{
		info.aabb = DirectX::BoundingBox(info.position, info.scale * 0.5f + Vector3(0.1f));
		m_bvh.ChangeAABB(info.bvhId, info.aabb);
		if (m_currentMethodIndex == 0)
		{
			m_bvh.FullyRebuild(false);
		}
		else if (m_currentMethodIndex == 1)
		{
			m_bvh.FullyRebuild();
		}
		else if (m_currentMethodIndex == 2)
		{
			m_bvh.Refit();
		}
		else if (m_currentMethodIndex == 3)
		{
			m_bvh.RefitWithRotation();
		}
		else if (m_currentMethodIndex == 4)
		{
			m_bvh.OptimizeObject(info.bvhId);
		}

		m_changed = false;
	}
}

void BVHApp::OnRender()
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

	deviceContext->VSSetConstantBuffers(0, 1, m_simpleConstantBuffer->GetBuffer().GetAddressOf());
	deviceContext->PSSetConstantBuffers(0, 1, m_simpleConstantBuffer->GetBuffer().GetAddressOf());

	// common
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// final
	deviceContext->RSSetViewports(1, &m_graphicsDevice.GetViewport());

	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
	deviceContext->ClearRenderTargetView(renderTargetView.Get(), Color{ 0.5f, 0.8f, 1.0f, 1.0f });
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	RenderFinal();

	deviceContext->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	//deviceContext->OMSetDepthStencilState(m_states->DepthNone(), 0);
	deviceContext->RSSetState(m_states->CullNone());

	m_effect->SetView(m_view);
	m_effect->SetProjection(m_projection);
	m_effect->Apply(deviceContext.Get());

	deviceContext->IASetInputLayout(m_inputLayout.Get());
	m_batch->Begin();

	for (auto& tr : m_cubeInfo)
	{
		DX::Draw(m_batch.get(), tr.aabb, DirectX::Colors::Red);
	}

	const auto& nodes = m_bvh.GetNodes();

	const int n = 20;
	const float frequency = DirectX::XM_2PI / n;

	const int step = 7;

	int i = 0;
	for (const auto& node : nodes)
	{
		float r = 0.5f + 0.5f * std::sin(frequency * (i * step) + 0.0f);
		float g = 0.5f + 0.5f * std::sin(frequency * (i * step) + 2.0f);
		float b = 0.5f + 0.5f * std::sin(frequency * (i * step) + 4.0f);

		DX::Draw(m_batch.get(), node.aabb, { r, g, b, 1.0f });

		i++;
	}

	m_batch->End();

	RenderImGui();

	m_graphicsDevice.EndDraw();
}

void BVHApp::OnShutdown()
{
	ShutdownImGui();
}

void BVHApp::RenderFinal()
{
	const auto& deviceContext = m_graphicsDevice.GetDeviceContext();

	// constant buffer
	SimpleConstant simpleConstant{};
	simpleConstant.view = m_view.Transpose();
	simpleConstant.projection = m_projection.Transpose();
	simpleConstant.lightDir = m_lightDirection;
	simpleConstant.lightColor = m_lightColor;
	simpleConstant.ambientColor = m_ambientLightColor;

	UINT stride = m_cubeVertexBuffer->GetBufferStride();
	UINT offset = 0;

	deviceContext->IASetInputLayout(m_cubeInputLayout->GetRawInputLayout());
	deviceContext->IASetVertexBuffers(0, 1, m_cubeVertexBuffer->GetBuffer().GetAddressOf(), &stride, &offset);
	deviceContext->IASetIndexBuffer(m_cubeIndexBuffer->GetRawBuffer(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->VSSetShader(m_cubeVertexShader->GetRawShader(), nullptr, 0);
	deviceContext->PSSetShader(m_cubePixelShader->GetRawShader(), nullptr, 0);

	for (auto& tr : m_cubeInfo)
	{
		Matrix world = Matrix::CreateScale(tr.scale) * Matrix::CreateTranslation(tr.position);

		simpleConstant.world = world.Transpose();
		deviceContext->UpdateSubresource(m_simpleConstantBuffer->GetRawBuffer(), 0, nullptr, &simpleConstant, 0, 0);
		deviceContext->DrawIndexed(m_indexCount, 0, 0);
	}
}

void BVHApp::RenderImGui()
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
	ImGui::Text("MoveCube");
	ImGui::Text("+X: ArrowRight / -X: ArrowLeft");
	ImGui::Text("+Z: ArrowUp / -Z: ArrowDown");
	ImGui::Text("+Y: PageUp / -Y: PageDown");
	const char* items[]{ "Fully rebuild(median)", "Fully rebuild(Full sweep SAH)", "Refit only", "Refit with rotation", "Remove/Insert"};
	if (ImGui::Combo("BVH Update Method", &m_currentMethodIndex, items, 5))
	{
		m_changed = true;
	}

	ImGui::NewLine();

	ImGui::SeparatorText("Light");

	ImGui::DragFloat2("Rotation(x, y)##2", &m_lightRotation.x, 0.5f);
	ImGui::InputFloat3("Direction", &m_lightDirection.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::ColorEdit3("Direct", &m_lightColor.x);
	ImGui::ColorEdit3("Ambient", &m_ambientLightColor.x);
	
	if (ImGui::Button("Reset##3"))
	{
		m_lightRotation = { -65.0f, -35.0f, 0.0f };
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

void BVHApp::InitializeImGui()
{
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(m_hWnd);
	ImGui_ImplDX11_Init(m_graphicsDevice.GetDevice().Get(), m_graphicsDevice.GetDeviceContext().Get());
}

void BVHApp::InitializeScene()
{
	m_simpleConstantBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"SimpleConstant", sizeof(SimpleConstant));

	// cube
	{
		std::vector<PositionNormalVertex3D> vertices{
			// position, normal
			{ { -0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f } }, // 0 - 0
			{ {  0.5f,  0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f } }, // 1 - 1
			{ { -0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f } }, // 2 - 2
			{ {  0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f, -1.0f } }, // 3 - 3

			{ {  0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f } }, // 4 - 4
			{ { -0.5f,  0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f } }, // 5 - 5
			{ {  0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f } }, // 6 - 6
			{ { -0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f } }, // 7 - 7

			{ { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f } }, // 5 - 8
			{ { -0.5f,  0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f } }, // 0 - 9
			{ { -0.5f, -0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f } }, // 7 - 10
			{ { -0.5f, -0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f } }, // 2 - 11

			{ {  0.5f,  0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f } }, // 1 - 12
			{ {  0.5f,  0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f } }, // 4 - 13
			{ {  0.5f, -0.5f, -0.5f }, {  1.0f,  0.0f,  0.0f } }, // 3 - 14
			{ {  0.5f, -0.5f,  0.5f }, {  1.0f,  0.0f,  0.0f } }, // 6 - 15

			{ { -0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f } }, // 2 - 16
			{ {  0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f } }, // 3 - 17
			{ { -0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f } }, // 7 - 18
			{ {  0.5f, -0.5f,  0.5f }, {  0.0f, -1.0f,  0.0f } }, // 6 - 19

			{ { -0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f } }, // 5 - 20
			{ {  0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f } }, // 4 - 21
			{ { -0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f } }, // 0 - 22
			{ {  0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f } }, // 1 - 23
		};

		std::vector<DWORD> indices{
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

		m_indexCount = static_cast<UINT>(indices.size());
		m_cubeVertexBuffer = D3DResourceManager::Get().GetOrCreateVertexBuffer(L"Cube", vertices);
		m_cubeIndexBuffer = D3DResourceManager::Get().GetOrCreateIndexBuffer(L"Cube", indices);
		auto layoutDesc = PositionNormalVertex3D::GetLayout();
		m_cubeInputLayout = D3DResourceManager::Get().GetOrCreateInputLayout(L"PositionNormalVS.hlsl",
			layoutDesc.data(), static_cast<UINT>(layoutDesc.size()));
		m_cubeVertexShader = D3DResourceManager::Get().GetOrCreateVertexShader(L"PositionNormalVS.hlsl");
		m_cubePixelShader = D3DResourceManager::Get().GetOrCreatePixelShader(L"LambertPS.hlsl");

		m_cubeInfo.reserve(20);
		std::vector<DirectX::BoundingBox> aabbs;

		{
			Vector3 position{};
			Vector3 scale{ 1.0f, 1.0f, 1.0f };
			DirectX::BoundingBox aabb{ position, 0.5f * scale + Vector3(0.1f) };
			m_cubeInfo.push_back({ position, scale, aabb });
			aabbs.push_back(aabb);
		}

		float areaSize = 20.0f;
		float maxCubeSize = 3.0f;
		for (int i = 1; i < (int)areaSize; ++i)
		{
			Vector3 position{ RandomFloat(-areaSize, areaSize), RandomFloat(-areaSize, areaSize), RandomFloat(-areaSize, areaSize) };
			Vector3 scale{ RandomFloat(1, maxCubeSize), RandomFloat(1, maxCubeSize), RandomFloat(1, maxCubeSize) };
			DirectX::BoundingBox aabb{ position, 0.5f * scale + Vector3(0.1f) };

			m_cubeInfo.push_back({ position, scale, aabb });
			aabbs.push_back(aabb);
		}

		auto ids = m_bvh.Insert(aabbs);

		for (size_t i = 0; i < ids.size(); ++i)
		{
			m_cubeInfo[i].bvhId = ids[i];
		}
	}
}

void BVHApp::ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

LRESULT BVHApp::MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	return WinApp::MessageProc(hWnd, uMsg, wParam, lParam);
}