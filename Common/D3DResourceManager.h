#pragma once

#include <unordered_map>
#include <memory>
#include <string>

#include "Vertex.h"


class D3DResource;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class VertexShader;
class PixelShader;
class ShaderResourceView;
class InputLayout;
class SamplerState;

class GraphicsDevice;

class D3DResourceManager
{
private:
	std::unordered_map<std::wstring, std::weak_ptr<VertexBuffer>> m_vertexBuffers;
	std::unordered_map<std::wstring, std::weak_ptr<IndexBuffer>> m_indexBuffers;
	std::unordered_map<std::pair<std::wstring, UINT>, std::weak_ptr<ConstantBuffer>> m_constantBuffers;
	std::unordered_map<std::wstring, std::weak_ptr<VertexShader>> m_vertexShaders;
	std::unordered_map<std::pair<std::wstring, unsigned long long>, std::weak_ptr<PixelShader>> m_pixelShaders;
	std::unordered_map<std::wstring, std::weak_ptr<ShaderResourceView>> m_shaderResourceViews;
	std::unordered_map<std::type_info, std::weak_ptr<InputLayout>> m_inputLayouts;
	std::unordered_map<std::wstring, std::weak_ptr<SamplerState>> m_samplerStates;

	GraphicsDevice* m_graphicsDevice = nullptr;

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
};