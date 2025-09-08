#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi1_3.h>

#include "../Common/WinApp.h"

namespace DirectX
{
	namespace SimpleMath
	{
		struct Matrix;
	}
}

class TransformApp :
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

	DirectX::SimpleMath::Matrix m_world1;
	DirectX::SimpleMath::Matrix m_world2;
	DirectX::SimpleMath::Matrix m_world3;
	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_projection;

	float m_firstRotationY = 0.0f;
	float m_secondRotationY = 0.0f;
	float m_thirdRotationY = 0.0f;

	DirectX::SimpleMath::Vector3 m_firstPosition{ 0.0f, 0.0f, 0.0f };
	DirectX::SimpleMath::Vector3 m_secondPosition{ 5.0f, 0.0f, 0.0f };
	DirectX::SimpleMath::Vector3 m_thirdPosition{ 2.0f, 0.0f, 0.0f };

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