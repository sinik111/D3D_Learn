#pragma once

#include <memory>
#include <d3d11.h>
#include <string>
#include <wrl/client.h>

#include "../Common/ShaderConstant.h"
#include "../Common/ShaderResourceView.h"

class StaticMeshData;
class MaterialData;

class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class VertexShader;
class PixelShader;
class ShaderResourceView;
class InputLayout;
class SamplerState;

class StaticMesh
{
private:
	// assets
	std::shared_ptr<StaticMeshData> m_staticMeshData;
	std::shared_ptr<MaterialData> m_materialData;

	// resources
	std::shared_ptr<VertexBuffer> m_vertexBuffer;
	std::shared_ptr<IndexBuffer> m_indexBuffer;
	std::shared_ptr<ConstantBuffer> m_materialBuffer;
	std::shared_ptr<ConstantBuffer> m_worldTransformBuffer;
	std::shared_ptr<VertexShader> m_finalPassVertexShader;
	std::shared_ptr<VertexShader> m_shadowPassVertexShader;
	std::shared_ptr<PixelShader> m_finalPassPixelShader;
	std::shared_ptr<PixelShader> m_shadowPassPixelShader;
	std::vector<TextureSRVs> m_textureSRVs;
	std::shared_ptr<InputLayout> m_inputLayout;
	std::shared_ptr<SamplerState> m_samplerState;

	// instance
	std::vector<MaterialBuffer> m_materialCBs;
	WorldTransformBuffer m_worldTransformCB;

public:
	StaticMesh(const std::wstring& filePath);

public:
	void Draw(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& deviceContext) const;

public:
	void SetWorld(const DirectX::SimpleMath::Matrix& world);
};