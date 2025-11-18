#pragma once

#include "../Common/WinApp.h"

#include <wrl/client.h>
#include <d3d11.h>
#include <directxtk/SimpleMath.h>

#include <dxgi1_6.h> // IDXGIFactory7
#pragma comment(lib, "dxgi.lib")

#include "StaticMesh.h"

class ResourceManagerApp :
	public WinApp
{
private:
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Vector4 = DirectX::SimpleMath::Vector4;
	using Matrix = DirectX::SimpleMath::Matrix;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_transformCB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_environmentCB;
	Microsoft::WRL::ComPtr<IDXGIAdapter3> m_dxgiAdapter;
	Microsoft::WRL::ComPtr<IDXGIDevice3> m_dxgiDevice;

	std::vector<StaticMesh> m_staticMeshes;

	Matrix m_view;
	Matrix m_projection;
	Matrix m_lightView;
	Matrix m_lightProjection;

	Matrix m_lightRotationMatrix;
	const Vector3 m_originalLightDir{ 0.0f, -1.0f, 0.0f };
	Vector3 m_lightDirection;
	Vector3 m_lightRotation{ -40.0f, 25.0f, 0.0f };
	Vector4 m_lightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 m_ambientLightColor{ 0.1f, 0.1f, 0.1f, 1.0f };
	bool m_useShadowPCF = true;

public:
	void Initialize() override;

private:
	void OnUpdate() override;
	void OnRender() override;
	void OnShutdown() override;

	void RenderFinal();
	void RenderImGui();

private:
	void InitializeImGui();
	void InitializeScene();

	void ShutdownImGui();

	LRESULT MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};