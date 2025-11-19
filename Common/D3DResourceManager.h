#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <d3d11.h>

#include "Vertex.h"
#include "ResourceKey.h"

class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class VertexShader;
class PixelShader;
class ShaderResourceView;
class InputLayout;
class SamplerState;

class GraphicsDevice;

enum class TextureType;

class D3DResourceManager
{
private:
	std::unordered_map<VertexBufferKey, std::weak_ptr<VertexBuffer>> m_vertexBuffers;
	std::unordered_map<std::wstring, std::weak_ptr<IndexBuffer>> m_indexBuffers;
	std::unordered_map<std::wstring, std::weak_ptr<ConstantBuffer>> m_constantBuffers;
	std::unordered_map<std::wstring, std::weak_ptr<VertexShader>> m_vertexShaders;
	std::unordered_map<std::wstring, std::weak_ptr<PixelShader>> m_pixelShaders;
	std::unordered_map<std::wstring, std::weak_ptr<ShaderResourceView>> m_shaderResourceViews;
	std::unordered_map<std::wstring, std::weak_ptr<InputLayout>> m_inputLayouts;
	std::unordered_map<std::wstring, std::weak_ptr<SamplerState>> m_samplerStates;

	const GraphicsDevice* m_graphicsDevice = nullptr;

private:
	D3DResourceManager() = default;
	~D3DResourceManager() = default;
	D3DResourceManager(const D3DResourceManager&) = delete;
	D3DResourceManager& operator=(const D3DResourceManager&) = delete;
	D3DResourceManager(D3DResourceManager&&) = delete;
	D3DResourceManager& operator=(D3DResourceManager&&) = delete;

public:
	static D3DResourceManager& Get();

public:
	void SetGraphicsDevice(const GraphicsDevice* graphicsDevice);
	std::shared_ptr<VertexBuffer> GetOrCreateVertexBuffer(const std::wstring& filePath, const std::vector<CommonVertex3D>& vertices);
	std::shared_ptr<VertexBuffer> GetOrCreateVertexBuffer(const std::wstring& filePath, const std::vector<BoneWeightVertex3D>& vertices);
	std::shared_ptr<IndexBuffer> GetOrCreateIndexBuffer(const std::wstring& filePath, const std::vector<DWORD>& indices);
	std::shared_ptr<ConstantBuffer> GetOrCreateConstantBuffer(const std::wstring& name, UINT byteWidth);
	std::shared_ptr<VertexShader> GetOrCreateVertexShader(const std::wstring& filePath);
	std::shared_ptr<PixelShader> GetOrCreatePixelShader(const std::wstring& filePath);
	std::shared_ptr<ShaderResourceView> GetOrCreateShaderResourceView(const std::wstring& filePath, TextureType type);
	std::shared_ptr<ShaderResourceView> GetOrCreateShaderResourceView(const std::wstring& name, const D3D11_TEXTURE2D_DESC& textureDesc,
		const D3D11_SUBRESOURCE_DATA& subData);
	std::shared_ptr<InputLayout> GetOrCreateInputLayout(const std::wstring& filePath, const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements);
	std::shared_ptr<SamplerState> GetOrCreateSamplerState(const std::wstring& name, const D3D11_SAMPLER_DESC& samplerDesc);
};