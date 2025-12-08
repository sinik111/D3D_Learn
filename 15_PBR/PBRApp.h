#pragma once

#include "../Common/WinApp.h"

#include <wrl/client.h>
#include <d3d11.h>
#include <directxtk/SimpleMath.h>
#include <memory>
#include <directxtk/CommonStates.h>
#include <directxtk/Effects.h>
#include <directxtk/PrimitiveBatch.h>
#include <directxtk/VertexTypes.h>

#include <dxgi1_6.h> // IDXGIFactory7
#pragma comment(lib, "dxgi.lib")

#include "../Common/ConstantBuffer.h"

#include "StaticMesh.h"
#include "SkeletalMesh.h"

class Texture2D;
class DepthStencilView;
class ShaderResourceView;
class RasterizerState;
class DepthStencilState;
class SamplerState;

struct OverrideMaterial
{
	DirectX::SimpleMath::Vector4 baseColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	float metalness = 0.0f;
	float roughness = 0.0f;
	int overrideMaterial = 0;
	float ambientOcclusion = 0.5f;
};

class PBRApp :
	public WinApp
{
private:
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Vector4 = DirectX::SimpleMath::Vector4;
	using Matrix = DirectX::SimpleMath::Matrix;

	std::shared_ptr<ConstantBuffer> m_transformBuffer;
	std::shared_ptr<ConstantBuffer> m_environmentBuffer;
	std::shared_ptr<ConstantBuffer> m_overrideMatBuffer;
	std::shared_ptr<ConstantBuffer> m_worldTransformBuffer;

	Microsoft::WRL::ComPtr<IDXGIAdapter3> m_dxgiAdapter;
	Microsoft::WRL::ComPtr<IDXGIDevice3> m_dxgiDevice;

	std::shared_ptr<Texture2D> m_shadowMapTex2D;
	std::shared_ptr<DepthStencilView> m_shadowMapDSV;
	std::shared_ptr<ShaderResourceView> m_shadowMapSRV;
	std::shared_ptr<RasterizerState> m_shadowMapRSS;
	std::shared_ptr<DepthStencilState> m_shadowMapDSS;

	std::shared_ptr<ShaderResourceView> m_cubeMapSRV;
	std::shared_ptr<ShaderResourceView> m_irradianceMapSRV;
	std::shared_ptr<ShaderResourceView> m_specularMapSRV;
	std::shared_ptr<ShaderResourceView> m_brdfLutSRV;
	std::shared_ptr<SamplerState> m_clampSampler;

	//skybox
	UINT m_indexCount = 0;
	std::shared_ptr<VertexBuffer> m_cubeVertexBuffer;
	std::shared_ptr<IndexBuffer> m_cubeIndexBuffer;
	std::shared_ptr<InputLayout> m_cubeInputLayout;
	std::shared_ptr<VertexShader> m_skyboxVertexShader;
	std::shared_ptr<PixelShader> m_skyboxPixelShader;
	std::shared_ptr<RasterizerState> m_skyboxRSState;
	std::shared_ptr<DepthStencilState> m_skyboxDSState;
	std::shared_ptr<SamplerState> m_samplerState;

	std::vector<StaticMesh> m_staticMeshes;
	std::vector<SkeletalMesh> m_skeletalMeshes;

	// Debug Draw
	using VertexType = DirectX::VertexPositionColor;

	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::BasicEffect> m_effect;
	std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	Matrix m_view;
	Matrix m_projection;
	Matrix m_lightView;
	Matrix m_lightProjection;

	Matrix m_lightRotationMatrix;
	const Vector3 m_originalLightDir{ 0.0f, -1.0f, 0.0f };
	Vector3 m_lightDirection;
	Vector3 m_lightRotation{ -40.0f, 88.0f, 0.0f };
	Vector4 m_lightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 m_ambientLightColor{ 0.1f, 0.1f, 0.1f, 1.0f };

	Vector3 m_lightPosition;

	OverrideMaterial m_overrideMaterialCB;
	bool m_overrideMaterial = false;

	float m_lightNear = 90000.0f;
	float m_lightFar = 100000.0f;
	float m_lightFOV = 2.0f;
	float m_lightForwardDistFromCam = 1000.0f;
	int m_shadowMapWidth = 8192;
	int m_shadowMapHeight = 8192;
	int m_pcfSize = 1;
	bool m_useShadowPCF = true;
	bool m_useIBL = true;

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