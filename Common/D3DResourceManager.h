#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <d3d11.h>
#include <wrl/client.h>

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
class Texture2D;
class DepthStencilView;
class DepthStencilState;
class RasterizerState;

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
	std::unordered_map<std::wstring, std::weak_ptr<Texture2D>> m_texture2Ds;
	std::unordered_map<std::wstring, std::weak_ptr<DepthStencilView>> m_depthStencilViews;
	std::unordered_map<std::wstring, std::weak_ptr<DepthStencilState>> m_depthStencilStates;
	std::unordered_map<std::wstring, std::weak_ptr<RasterizerState>> m_rasterizerStates;

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
	std::shared_ptr<ShaderResourceView> GetOrCreateShaderResourceView(const std::wstring& name, const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture2D,
		const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc);
	std::shared_ptr<InputLayout> GetOrCreateInputLayout(const std::wstring& filePath, const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements);
	std::shared_ptr<SamplerState> GetOrCreateSamplerState(const std::wstring& name, const D3D11_SAMPLER_DESC& samplerDesc);
	std::shared_ptr<Texture2D> GetOrCreateTexture2D(const std::wstring& name, const D3D11_TEXTURE2D_DESC& texDesc);
	std::shared_ptr<DepthStencilView> GetOrCreateDepthStencilView(const std::wstring& name, const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture2D,
		const D3D11_DEPTH_STENCIL_VIEW_DESC& dsvDesc);
	std::shared_ptr<DepthStencilState> GetOrCreateDepthStencilState(const std::wstring& name, const D3D11_DEPTH_STENCIL_DESC& dsDesc);
	std::shared_ptr<RasterizerState> GetOrCreateRasterizerState(const std::wstring& name, const D3D11_RASTERIZER_DESC& rsDesc);
};