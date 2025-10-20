#pragma once

#include "../Common/WinApp.h"

#include <wrl/client.h>
#include <d3d11.h>
#include <directxtk/SimpleMath.h>

#include "StaticMesh.h"
#include "SkeletalMesh.h"

class BoneTransformAnimationApp :
	public WinApp
{
private:
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Vector4 = DirectX::SimpleMath::Vector4;
	using Matrix = DirectX::SimpleMath::Matrix;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_blinnPhongVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_blinnPhongPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_blinnPhongInputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

	std::vector<SkeletalMesh> m_staticMeshes;

	UINT m_vertexBufferStride = 0;
	UINT m_vertexBufferOffset = 0;
	UINT m_indexCount = 0;

	Matrix m_view;
	Matrix m_projection;

	Matrix m_world;
	Vector3 m_scale{ 50.0f, 50.0f, 50.0f };
	Vector3 m_rotation{ 0.0f, 0.0f, 0.0f };
	Vector3 m_position{ 0.0f, 25.0f, 0.0f };
	Vector4 m_materialAmbient{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 m_materialSpecular{ 1.0f, 1.0f, 1.0f, 1.0f };

	Matrix m_lightRotationMatrix;
	const Vector4 m_originalLightDir{ 0.0f, -1.0f, 0.0f, 0.0f };
	Vector4 m_lightDirection;
	Vector3 m_lightRotation{ -40.0f, 25.0f, 0.0f };
	Vector4 m_lightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 m_ambientLightColor{ 0.1f, 0.1f, 0.1f, 1.0f };
	Vector4 m_shininess{ 64.0f, 0.0f, 0.0f, 0.0f };

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