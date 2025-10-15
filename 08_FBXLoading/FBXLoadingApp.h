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

	InstanceData(DirectX::SimpleMath::Matrix world)
		: world{ world }
	{

	}
};

class FBXLoadingApp :
	public WinApp
{
private:
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Vector4 = DirectX::SimpleMath::Vector4;
	using Matrix = DirectX::SimpleMath::Matrix;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_blinnPhongVertexShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_skyboxVertexShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_instancingVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_blinnPhongPixelShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_skyboxPixelShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_instancingPixelShader;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_blinnPhongInputLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_skyboxInputLayout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_instancingInputLayout;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_cubeVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_cubeIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceBuffer;

	TextureSRVs m_cubeTextureSRVs;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_skyboxTextureRV;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

	std::vector<Model> m_models;
	Model* m_crystalModel = nullptr;
	std::vector<InstanceData> m_instanceDatas;

	UINT m_vertexBufferStride = 0;
	UINT m_vertexBufferOffset = 0;
	UINT m_instanceBufferStrides[2]{};
	UINT m_instanceBufferOffsets[2]{};
	UINT m_indexCount = 0;

	const size_t m_maxShells = 1000;
	const size_t m_minShell = 1;
	size_t m_currentShells = 100;

	Matrix m_view;
	Matrix m_projection;

	Matrix m_cubeWorld;
	Vector3 m_scale{ 50.0f, 50.0f, 50.0f };
	Vector3 m_rotation{ 0.0f, 0.0f, 0.0f };
	Vector3 m_position{ 0.0f, 25.0f, 0.0f };
	Vector4 m_materialAmbient{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 m_materialSpecular{ 1.0f, 1.0f, 1.0f, 1.0f };
	float m_crystalScale = 1.0f;
	float m_crystalDegree = 0.0f;
	Vector3 m_crystalPosition;

	Matrix m_lightRotationMatrix;
	const Vector4 m_originalLightDir{ 0.0f, -1.0f, 0.0f, 0.0f };
	Vector4 m_lightDirection;
	Vector3 m_lightRotation{ -40.0f, 25.0f, 0.0f };
	Vector4 m_lightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 m_ambientLightColor{ 0.1f, 0.1f, 0.1f, 1.0f };
	Vector4 m_shininess{ 64.0f, 0.0f, 0.0f, 0.0f };
	bool m_useInstancing = true;
	bool m_changed = true;

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

	void AddShell();
	void SubShell();
};