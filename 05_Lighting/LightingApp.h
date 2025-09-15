#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <directxtk/SimpleMath.h>

#include "../Common/WinApp.h"

class LightingApp :
	public WinApp
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	UINT m_vertexBufferStride = 0;
	UINT m_vertexBufferOffset = 0;
	UINT m_indexCount = 0;

	DirectX::SimpleMath::Matrix m_world;
	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_projection;

	DirectX::SimpleMath::Vector3 m_scale{ 3.0f, 6.0f, 3.0f };
	DirectX::SimpleMath::Vector3 m_rotation{ 30.0f, 60.0f, 0.0f };
	DirectX::SimpleMath::Vector3 m_position{ 0.0f, 0.0f, 0.0f };

	DirectX::SimpleMath::Matrix m_lightRotationMatrix;
	const DirectX::SimpleMath::Vector4 m_originalLightDir{ 0.0f, 1.0f, 0.0f, 0.0f };
	DirectX::SimpleMath::Vector3 m_lightRotation{ -45.0f, 45.0f, 0.0f };
	DirectX::SimpleMath::Vector4 m_lightColor{ 1.0f, 1.0f, 1.0f, 1.0f };

public:
	void Initialize() override;

private:
	void OnUpdate() override;
	void OnRender() override;
	void OnShutdown() override;

	void RenderImGui();

private:
	void InitializeImGui();
	void InitializeScene();

	void ShutdownImGui();

	LRESULT MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};