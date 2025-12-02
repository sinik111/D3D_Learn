#include "ShaderResourceView.h"

#include <directxtk/WICTextureLoader.h>
#include <directxtk/DDSTextureLoader.h>
#include <DirectXTex.h>
#include <filesystem>

void ShaderResourceView::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filePath, TextureType type)
{
	namespace fs = std::filesystem;

	if (type == TextureType::Texture2D)
	{
		auto extension = fs::path(filePath).extension();

		if (extension == ".tga" || extension == ".TGA")
		{
			DirectX::ScratchImage image;
			LoadFromTGAFile(filePath.c_str(), nullptr, image);
			
			CreateShaderResourceView(device.Get(),image.GetImages(), image.GetImageCount(), image.GetMetadata(), &m_shaderResourceView);
		}
		else
		{
			DirectX::CreateWICTextureFromFile(device.Get(), filePath.c_str(), nullptr, &m_shaderResourceView);
		}
	}

	if (type == TextureType::TextureCube)
	{
		DirectX::CreateDDSTextureFromFile(device.Get(), filePath.c_str(), nullptr, &m_shaderResourceView);
	}
}

void ShaderResourceView::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const D3D11_TEXTURE2D_DESC& textureDesc,
	const D3D11_SUBRESOURCE_DATA& subData)
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;

	device->CreateTexture2D(&textureDesc, &subData, &texture);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	device->CreateShaderResourceView(texture.Get(), &srvDesc, &m_shaderResourceView);
}

void ShaderResourceView::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture2D, const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc)
{
	device->CreateShaderResourceView(texture2D.Get(), &srvDesc, &m_shaderResourceView);
}

const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& ShaderResourceView::GetShaderResourceView() const
{
	return m_shaderResourceView;
}

ID3D11ShaderResourceView* ShaderResourceView::GetRawShaderResourceView() const
{
	return m_shaderResourceView.Get();
}
