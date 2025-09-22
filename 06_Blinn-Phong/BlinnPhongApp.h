#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <directxtk/SimpleMath.h>

#include "../Common/WinApp.h"

class BlinnPhongApp :
	public WinApp
{
private:
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Vector4 = DirectX::SimpleMath::Vector4;
	using Matrix = DirectX::SimpleMath::Matrix;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_skyboxVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_skyboxPixelShader;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_skyboxInputLayout;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_skyboxVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_skyboxIndexBuffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_crystalTextureRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_skyboxTextureRV;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

	UINT m_vertexBufferStride = 0;
	UINT m_indexCount = 0;

	UINT m_skyboxIndexCount = 0;

	UINT m_vertexBufferOffset = 0;

	Matrix m_world;
	Matrix m_view;
	Matrix m_projection;

	Vector3 m_scale{ 3.0f, 6.0f, 3.0f };
	Vector3 m_rotation{ 30.0f, 60.0f, 0.0f };
	Vector3 m_position{ 0.0f, 0.0f, 0.0f };

	Matrix m_lightRotationMatrix;
	const Vector4 m_originalLightDir{ 0.0f, -1.0f, 0.0f, 0.0f };
	Vector4 m_lightDirection;
	Vector3 m_lightRotation{ -35.0f, 145.0f, 0.0f };
	Vector4 m_lightColor{ 1.0f, 1.0f, 1.0f, 1.0f };

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
	void CreateCrystal();
	void CreateSkyBox();

	void ShutdownImGui();

	LRESULT MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};