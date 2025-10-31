#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <directxtk/SimpleMath.h>
#include <memory>

#include "../Common/WinApp.h"

#include "StaticMesh.h"

class ToonShadingWithOutlineApp :
	public WinApp
{
private:
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Vector4 = DirectX::SimpleMath::Vector4;
	using Matrix = DirectX::SimpleMath::Matrix;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_outlineVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_outlinePixelShader;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_phongToonVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_phongToonPixelShader;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_vertexInputLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_simpleVertexInputLayout;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_outlineTextureRV;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_outlineRasterizerState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_outlineDepthStencilState;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

	std::unique_ptr<StaticMesh> m_teapot;

	UINT m_vertexBufferStride;
	UINT m_vertexBufferOffset = 0;

	Matrix m_view;
	Matrix m_projection;

	Matrix m_world;
	Vector3 m_scale{ 0.01f, 0.01f, 0.01f };
	Vector3 m_rotation{ 0.0f, 0.0f, 0.0f };
	Vector3 m_position{ 0.0f, 0.0f, 0.0f };

	Vector4 m_materialDiffuse{ 1.0f, 0.0f, 1.0f, 1.0f };
	Vector4 m_materialAmbient{ 0.1f, 0.1f, 0.1f, 1.0f };
	Vector4 m_materialSpecular{ 1.0f, 1.0f, 1.0f, 1.0f };

	Matrix m_lightRotationMatrix;
	const Vector4 m_originalLightDir{ 0.0f, -1.0f, 0.0f, 0.0f };
	Vector4 m_lightDirection;
	Vector3 m_lightRotation{ 0.0f, 0.0f, 0.0f };
	Vector4 m_lightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 m_ambientLightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	float m_shininess = 64.0f;

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