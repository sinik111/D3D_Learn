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

void D3DResourceManager::SetGraphicsDevice(const GraphicsDevice* graphicsDevice)
{
	m_graphicsDevice = graphicsDevice;
}

std::shared_ptr<VertexBuffer> D3DResourceManager::GetOrCreateVertexBuffer(const std::wstring& filePath, const std::vector<CommonVertex3D>& vertices)
{
	auto find = m_vertexBuffers.find(filePath);
	if (find != m_vertexBuffers.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<VertexBuffer> vertexBuffer;
	vertexBuffer->Create(m_graphicsDevice->GetDevice(), vertices);

	m_vertexBuffers[filePath] = vertexBuffer;

	return vertexBuffer;
}

std::shared_ptr<IndexBuffer> D3DResourceManager::GetOrCreateIndexBuffer(const std::wstring& filePath, const std::vector<DWORD>& indices)
{
	auto find = m_indexBuffers.find(filePath);
	if (find != m_indexBuffers.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<IndexBuffer> indexBuffer;
	indexBuffer->Create(m_graphicsDevice->GetDevice(), indices);

	m_indexBuffers[filePath] = indexBuffer;

	return indexBuffer;
}

std::shared_ptr<ConstantBuffer> D3DResourceManager::GetOrCreateConstantBuffer(const std::wstring& name, UINT byteWidth)
{
	auto find = m_constantBuffers.find(name);
	if (find != m_constantBuffers.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<ConstantBuffer> constantBuffer;
	constantBuffer->Create(m_graphicsDevice->GetDevice(), byteWidth);

	m_constantBuffers[name] = constantBuffer;

	return constantBuffer;
}

std::shared_ptr<VertexShader> D3DResourceManager::GetOrCreateVertexShader(const std::wstring& filePath)
{
	auto find = m_vertexShaders.find(filePath);
	if (find != m_vertexShaders.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<VertexShader> vertexShader;
	vertexShader->Create(m_graphicsDevice->GetDevice(), filePath);

	m_vertexShaders[filePath] = vertexShader;

	return vertexShader;
}

std::shared_ptr<PixelShader> D3DResourceManager::GetOrCreatePixelShader(const std::wstring& filePath)
{
	auto find = m_pixelShaders.find(filePath);
	if (find != m_pixelShaders.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<PixelShader> pixelShader;
	pixelShader->Create(m_graphicsDevice->GetDevice(), filePath);

	m_pixelShaders[filePath] = pixelShader;

	return pixelShader;
}

std::shared_ptr<ShaderResourceView> D3DResourceManager::GetOrCreateShaderResourceView(const std::wstring& filePath, TextureType type)
{
	auto find = m_shaderResourceViews.find(filePath);
	if (find != m_shaderResourceViews.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<ShaderResourceView> shaderResourceView;
	shaderResourceView->Create(m_graphicsDevice->GetDevice(), filePath, type);

	m_shaderResourceViews[filePath] = shaderResourceView;

	return shaderResourceView;
}

std::shared_ptr<ShaderResourceView> D3DResourceManager::GetOrCreateShaderResourceView(const std::wstring& name,
	const D3D11_TEXTURE2D_DESC& textureDesc, const D3D11_SUBRESOURCE_DATA& subData)
{
	auto find = m_shaderResourceViews.find(name);
	if (find != m_shaderResourceViews.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<ShaderResourceView> shaderResourceView;
	shaderResourceView->Create(m_graphicsDevice->GetDevice(), textureDesc, subData);

	m_shaderResourceViews[name] = shaderResourceView;

	return shaderResourceView;
}

std::shared_ptr<InputLayout> D3DResourceManager::GetOrCreateInputLayout(const std::wstring& filePath,
	const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements)
{
	auto find = m_inputLayouts.find(filePath);
	if (find != m_inputLayouts.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<InputLayout> inputLayout;
	inputLayout->Create(m_graphicsDevice->GetDevice(), filePath, layoutDesc, numElements);

	m_inputLayouts[filePath] = inputLayout;

	return inputLayout;
}

std::shared_ptr<SamplerState> D3DResourceManager::GetOrCreateSamplerState(const std::wstring& name, const D3D11_SAMPLER_DESC& samplerDesc)
{
	auto find = m_samplerStates.find(name);
	if (find != m_samplerStates.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<SamplerState> samplerState;
	samplerState->Create(m_graphicsDevice->GetDevice(), samplerDesc);

	m_samplerStates[name] = samplerState;

	return samplerState;
}