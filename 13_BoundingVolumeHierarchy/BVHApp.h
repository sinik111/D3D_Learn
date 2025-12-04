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

#include "../Common/ConstantBuffer.h"

#include "BVH.h"

class VertexBuffer;
class IndexBuffer;
class InputLayout;
class VertexShader;
class PixelShader;

struct CubeInfo
{
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 scale;
	DirectX::BoundingBox aabb;
};

class BVHApp :
	public WinApp
{
private:
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Vector4 = DirectX::SimpleMath::Vector4;
	using Matrix = DirectX::SimpleMath::Matrix;

	std::shared_ptr<ConstantBuffer> m_simpleConstantBuffer;

	std::shared_ptr<VertexBuffer> m_cubeVertexBuffer;
	std::shared_ptr<IndexBuffer> m_cubeIndexBuffer;
	std::shared_ptr<InputLayout> m_cubeInputLayout;
	std::shared_ptr<VertexShader> m_cubeVertexShader;
	std::shared_ptr<PixelShader> m_cubePixelShader;

	// Debug Draw
	using VertexType = DirectX::VertexPositionColor;

	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::BasicEffect> m_effect;
	std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	std::vector<CubeInfo> m_cubeInfo;
	Matrix m_view;
	Matrix m_projection;

	Matrix m_lightRotationMatrix;
	const Vector3 m_originalLightDir{ 0.0f, -1.0f, 0.0f };
	Vector3 m_lightDirection;
	Vector3 m_lightRotation{ -65.0f, -35.0f, 0.0f };
	Vector3 m_lightColor{ 1.0f, 1.0f, 1.0f };
	Vector3 m_ambientLightColor{ 0.1f, 0.1f, 0.1f };

	UINT m_indexCount = 0;
	BVH m_bvh;

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