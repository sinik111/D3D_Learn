#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <directxtk/SimpleMath.h>
#include <memory>

#include "../Common/WinApp.h"
#include "Model.h"

struct InstanceData
{
	DirectX::SimpleMath::Matrix world;
};

class FBXLoadingApp :
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

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_instancingVertexShader;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_skyboxInputLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_instancingInputLayout;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceBuffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubeDiffuseRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubeNormalRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubeSpecularRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_skyboxTextureRV;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;

	std::vector<Model> m_models;
	std::vector<InstanceData> m_instanceDatas;

	UINT m_vertexBufferStride = 0;
	UINT m_vertexBufferOffset = 0;
	UINT m_indexCount = 0;
	const UINT m_maxShells = 50;
	UINT m_currentShells = 10;
	UINT m_startShell = 4;

	Matrix m_world;
	Matrix m_view;
	Matrix m_projection;

	Vector3 m_scale{ 50.0f, 50.0f, 50.0f };
	Vector3 m_rotation{ 0.0f, 0.0f, 0.0f };
	Vector3 m_position{ 0.0f, 0.0f, 0.0f };
	Vector4 m_materialAmbient{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 m_materialSpecular{ 1.0f, 1.0f, 1.0f, 1.0f };

	Matrix m_lightRotationMatrix;
	const Vector4 m_originalLightDir{ 0.0f, -1.0f, 0.0f, 0.0f };
	Vector4 m_lightDirection;
	Vector3 m_lightRotation{ -75.0f, 25.0f, 0.0f };
	Vector4 m_lightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 m_ambientLightColor{ 0.1f, 0.1f, 0.1f, 1.0f };
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