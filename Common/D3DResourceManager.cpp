#include "D3DResourceManager.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "ConstantBuffer.h"
#include "InputLayout.h"
#include "ShaderResourceView.h"
#include "SamplerState.h"
#include "GraphicsDevice.h"
#include "Texture2D.h"
#include "DepthStencilView.h"
#include "DepthStencilState.h"
#include "RasterizerState.h"

D3DResourceManager& D3DResourceManager::Get()
{
	static D3DResourceManager s_instance;

	return s_instance;
}

void D3DResourceManager::SetGraphicsDevice(const GraphicsDevice* graphicsDevice)
{
	m_graphicsDevice = graphicsDevice;
}

std::shared_ptr<VertexBuffer> D3DResourceManager::GetOrCreateVertexBuffer(const std::wstring& filePath,
	const std::vector<CommonVertex3D>& vertices)
{
	VertexBufferKey key{ filePath, VertexFormat::Common3D };
	if (auto find = m_vertexBuffers.find(key); find != m_vertexBuffers.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>();
	vertexBuffer->Create(m_graphicsDevice->GetDevice(), vertices);

	m_vertexBuffers[key] = vertexBuffer;

	return vertexBuffer;
}

std::shared_ptr<VertexBuffer> D3DResourceManager::GetOrCreateVertexBuffer(const std::wstring& filePath,
	const std::vector<BoneWeightVertex3D>& vertices)
{
	VertexBufferKey key{ filePath, VertexFormat::BoneWeight3D };
	if (auto find = m_vertexBuffers.find(key); find != m_vertexBuffers.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>();
	vertexBuffer->Create(m_graphicsDevice->GetDevice(), vertices);

	m_vertexBuffers[key] = vertexBuffer;

	return vertexBuffer;
}

std::shared_ptr<IndexBuffer> D3DResourceManager::GetOrCreateIndexBuffer(const std::wstring& filePath,
	const std::vector<DWORD>& indices)
{
	if (auto find = m_indexBuffers.find(filePath); find != m_indexBuffers.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>();
	indexBuffer->Create(m_graphicsDevice->GetDevice(), indices);

	m_indexBuffers[filePath] = indexBuffer;

	return indexBuffer;
}

std::shared_ptr<ConstantBuffer> D3DResourceManager::GetOrCreateConstantBuffer(const std::wstring& name, UINT byteWidth)
{
	if (auto find = m_constantBuffers.find(name); find != m_constantBuffers.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<ConstantBuffer> constantBuffer = std::make_shared<ConstantBuffer>();
	constantBuffer->Create(m_graphicsDevice->GetDevice(), byteWidth);

	m_constantBuffers[name] = constantBuffer;

	return constantBuffer;
}

std::shared_ptr<VertexShader> D3DResourceManager::GetOrCreateVertexShader(const std::wstring& filePath)
{
	if (auto find = m_vertexShaders.find(filePath); find != m_vertexShaders.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<VertexShader> vertexShader = std::make_shared<VertexShader>();
	vertexShader->Create(m_graphicsDevice->GetDevice(), filePath);

	m_vertexShaders[filePath] = vertexShader;

	return vertexShader;
}

std::shared_ptr<PixelShader> D3DResourceManager::GetOrCreatePixelShader(const std::wstring& filePath)
{
	if (auto find = m_pixelShaders.find(filePath); find != m_pixelShaders.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<PixelShader> pixelShader = std::make_shared<PixelShader>();
	pixelShader->Create(m_graphicsDevice->GetDevice(), filePath);

	m_pixelShaders[filePath] = pixelShader;

	return pixelShader;
}

std::shared_ptr<ShaderResourceView> D3DResourceManager::GetOrCreateShaderResourceView(const std::wstring& filePath,
	TextureType type)
{
	if (auto find = m_shaderResourceViews.find(filePath); find != m_shaderResourceViews.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<ShaderResourceView> shaderResourceView = std::make_shared<ShaderResourceView>();
	shaderResourceView->Create(m_graphicsDevice->GetDevice(), filePath, type);

	m_shaderResourceViews[filePath] = shaderResourceView;

	return shaderResourceView;
}

std::shared_ptr<ShaderResourceView> D3DResourceManager::GetOrCreateShaderResourceView(const std::wstring& name,
	const D3D11_TEXTURE2D_DESC& textureDesc, const D3D11_SUBRESOURCE_DATA& subData)
{
	if (auto find = m_shaderResourceViews.find(name); find != m_shaderResourceViews.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<ShaderResourceView> shaderResourceView = std::make_shared<ShaderResourceView>();
	shaderResourceView->Create(m_graphicsDevice->GetDevice(), textureDesc, subData);

	m_shaderResourceViews[name] = shaderResourceView;

	return shaderResourceView;
}

std::shared_ptr<ShaderResourceView> D3DResourceManager::GetOrCreateShaderResourceView(const std::wstring& name,
	const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture2D, const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc)
{
	if (auto find = m_shaderResourceViews.find(name); find != m_shaderResourceViews.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<ShaderResourceView> shaderResourceView = std::make_shared<ShaderResourceView>();
	shaderResourceView->Create(m_graphicsDevice->GetDevice(), texture2D, srvDesc);

	m_shaderResourceViews[name] = shaderResourceView;

	return shaderResourceView;
}

std::shared_ptr<InputLayout> D3DResourceManager::GetOrCreateInputLayout(const std::wstring& filePath,
	const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements)
{
	if (auto find = m_inputLayouts.find(filePath); find != m_inputLayouts.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<InputLayout> inputLayout = std::make_shared<InputLayout>();
	inputLayout->Create(m_graphicsDevice->GetDevice(), filePath, layoutDesc, numElements);

	m_inputLayouts[filePath] = inputLayout;

	return inputLayout;
}

std::shared_ptr<SamplerState> D3DResourceManager::GetOrCreateSamplerState(const std::wstring& name,
	const D3D11_SAMPLER_DESC& samplerDesc)
{
	if (auto find = m_samplerStates.find(name); find != m_samplerStates.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<SamplerState> samplerState = std::make_shared<SamplerState>();
	samplerState->Create(m_graphicsDevice->GetDevice(), samplerDesc);

	m_samplerStates[name] = samplerState;

	return samplerState;
}

std::shared_ptr<Texture2D> D3DResourceManager::GetOrCreateTexture2D(const std::wstring& name,
	const D3D11_TEXTURE2D_DESC& texDesc)
{
	if (auto find = m_texture2Ds.find(name); find != m_texture2Ds.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<Texture2D> texture2D = std::make_shared<Texture2D>();
	texture2D->Create(m_graphicsDevice->GetDevice(), texDesc);

	m_texture2Ds[name] = texture2D;

	return texture2D;
}

std::shared_ptr<DepthStencilView> D3DResourceManager::GetOrCreateDepthStencilView(const std::wstring& name,
	const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture2D, const D3D11_DEPTH_STENCIL_VIEW_DESC& dsvDesc)
{
	if (auto find = m_depthStencilViews.find(name); find != m_depthStencilViews.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<DepthStencilView> depthStencilView = std::make_shared<DepthStencilView>();
	depthStencilView->Create(m_graphicsDevice->GetDevice(), texture2D, dsvDesc);

	m_depthStencilViews[name] = depthStencilView;

	return depthStencilView;
}

std::shared_ptr<DepthStencilState> D3DResourceManager::GetOrCreateDepthStencilState(const std::wstring& name,
	const D3D11_DEPTH_STENCIL_DESC& dsDesc)
{
	if (auto find = m_depthStencilStates.find(name); find != m_depthStencilStates.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<DepthStencilState> depthStencilState = std::make_shared<DepthStencilState>();
	depthStencilState->Create(m_graphicsDevice->GetDevice(), dsDesc);

	m_depthStencilStates[name] = depthStencilState;

	return depthStencilState;
}

std::shared_ptr<RasterizerState> D3DResourceManager::GetOrCreateRasterizerState(const std::wstring& name,
	const D3D11_RASTERIZER_DESC& rsDesc)
{
	if (auto find = m_rasterizerStates.find(name); find != m_rasterizerStates.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<RasterizerState> rasterizerState = std::make_shared<RasterizerState>();
	rasterizerState->Create(m_graphicsDevice->GetDevice(), rsDesc);

	m_rasterizerStates[name] = rasterizerState;

	return rasterizerState;
}