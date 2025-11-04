#pragma once

#include "../Common/WinApp.h"

#include <wrl/client.h>
#include <d3d11.h>
#include <directxtk/SimpleMath.h>

#include "StaticMesh.h"
#include "SkeletalMesh.h"

class ShadowMappingApp :
	public WinApp
{
private:
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Vector4 = DirectX::SimpleMath::Vector4;
	using Matrix = DirectX::SimpleMath::Matrix;
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComPtr<ID3D11VertexShader> m_basicVS;
	ComPtr<ID3D11VertexShader> m_basicLightViewVS;
	ComPtr<ID3D11VertexShader> m_rigidAnimVS;
	ComPtr<ID3D11VertexShader> m_rigidAnimLightViewVS;
	ComPtr<ID3D11VertexShader> m_skinningAnimVS;
	ComPtr<ID3D11VertexShader> m_skinningAnimLightViewVS;
	ComPtr<ID3D11VertexShader> m_skyboxVS;

	ComPtr<ID3D11PixelShader> m_blinnPhongPS;
	ComPtr<ID3D11PixelShader> m_skyboxPS;
	ComPtr<ID3D11PixelShader> m_lightViewPS;

	ComPtr<ID3D11InputLayout> m_commonInputLayout;
	ComPtr<ID3D11InputLayout> m_skinningInputLayout;
	ComPtr<ID3D11InputLayout> m_posInputLayout;

	ComPtr<ID3D11Buffer> m_worldTransformCB;
	ComPtr<ID3D11Buffer> m_transformCB;
	ComPtr<ID3D11Buffer> m_environmentCB;
	ComPtr<ID3D11Buffer> m_materialCB;
	ComPtr<ID3D11Buffer> m_bonePoseCB;
	ComPtr<ID3D11Buffer> m_boneOffsetCB;

	ComPtr<ID3D11Buffer> m_cubeVertexBuffer;
	ComPtr<ID3D11Buffer> m_cubeIndexBuffer;

	ComPtr<ID3D11SamplerState> m_samplerState;
	ComPtr<ID3D11SamplerState> m_samplerComparisonState;

	ComPtr<ID3D11ShaderResourceView> m_skyboxTextureSRV;
	ComPtr<ID3D11RasterizerState> m_skyboxRSState;
	ComPtr<ID3D11DepthStencilState> m_skyboxDSState;

	ComPtr<ID3D11Texture2D> m_shadowMap;
	ComPtr<ID3D11DepthStencilView> m_shadowMapDSV;
	ComPtr<ID3D11ShaderResourceView> m_shadowMapSRV;
	ComPtr<ID3D11RasterizerState> m_shadowMapRSState;
	ComPtr<ID3D11DepthStencilState> m_shadowMapDSState;

	std::vector<StaticMesh> m_staticMeshes;
	std::vector<SkeletalMesh> m_rigidAnimMeshes;
	std::vector<SkeletalMesh> m_skinningAnimMeshes;

	UINT m_boneWeightVertexBufferStride = 0;
	UINT m_commonVertexBufferStride = 0;
	UINT m_cubeVertexBufferStride = 0;
	UINT m_vertexBufferOffset = 0;
	UINT m_cubeIndexCount = 0;

	Matrix m_view;
	Matrix m_projection;
	Matrix m_lightView;
	Matrix m_lightProjection;

	Vector4 m_materialAmbient{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 m_materialSpecular{ 1.0f, 1.0f, 1.0f, 1.0f };

	int m_shadowMapWidth = 8192;
	int m_shadowMapHeight = 8192;
	float m_lightNear = 90000.0f;
	float m_lightFar = 100000.0f;
	float m_lightFOV = 2.0f;
	float m_lightForwardDistFromCam = 1000.0f;

	Vector3 m_lightPosition;

	Matrix m_lightRotationMatrix;
	const Vector3 m_originalLightDir{ 0.0f, -1.0f, 0.0f };
	Vector3 m_lightDirection;
	Vector3 m_lightRotation{ 45.0f, 45.0f, 0.0f };
	Vector4 m_lightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 m_ambientLightColor{ 0.1f, 0.1f, 0.1f, 1.0f };
	float m_shininess = 64.0f;
	bool m_useShadowPCF = true;

public:
	void Initialize() override;

private:
	void OnUpdate() override;
	void OnRender() override;
	void OnShutdown() override;

	void RenderShadowMap();
	void RenderFinal();
	void RenderImGui();

private:
	void InitializeImGui();
	void InitializeScene();

	void ShutdownImGui();

	LRESULT MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
};