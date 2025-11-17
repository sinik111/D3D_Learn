#include "ShaderResourceView.h"

#include <directxtk/WICTextureLoader.h>
#include <directxtk/DDSTextureLoader.h>

void ShaderResourceView::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filePath, TextureType type)
{
	if (type == TextureType::Texture2D)
	{
		DirectX::CreateWICTextureFromFile(device.Get(), filePath.c_str(), nullptr, &m_srv);
	}

	if (type == TextureType::TextureCube)
	{
		DirectX::CreateDDSTextureFromFile(device.Get(), filePath.c_str(), nullptr, &m_srv);
	}
}

void ShaderResourceView::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
	const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc)
{
	device->CreateShaderResourceView(texture.Get(), &srvDesc, &m_srv);
}

const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& ShaderResourceView::GetSRV() const
{
	return m_srv;
}
