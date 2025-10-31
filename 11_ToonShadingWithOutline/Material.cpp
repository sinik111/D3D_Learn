#include "Material.h"

#include <assimp/material.h>
#include <directxtk/WICTextureLoader.h>
#include <filesystem>

#include "../Common/Helper.h"

static TextureSRVs* s_defaultTextureSRVs = nullptr;

Material::Material(const Microsoft::WRL::ComPtr<ID3D11Device>& device, aiMaterial* material)
{
	namespace fs = std::filesystem;

	aiString path;

	if (aiReturn_SUCCESS == material->GetTexture(aiTextureType_DIFFUSE, 0, &path))
	{
		std::wstring fileName = fs::path(ToWideCharStr(path.C_Str())).filename();

		DirectX::CreateWICTextureFromFile(device.Get(), fileName.c_str(), nullptr, &m_textureSRVs.diffuseTextureSRV);
	}
	else
	{
		m_textureSRVs.diffuseTextureSRV = s_defaultTextureSRVs->diffuseTextureSRV;
	}

	if (aiReturn_SUCCESS == material->GetTexture(aiTextureType_NORMALS, 0, &path))
	{
		std::wstring fileName = fs::path(ToWideCharStr(path.C_Str())).filename();

		DirectX::CreateWICTextureFromFile(device.Get(), fileName.c_str(), nullptr, &m_textureSRVs.normalTextureSRV);
	}
	else
	{
		m_textureSRVs.normalTextureSRV = s_defaultTextureSRVs->normalTextureSRV;
	}

	if (aiReturn_SUCCESS == material->GetTexture(aiTextureType_SPECULAR, 0, &path))
	{
		std::wstring fileName = fs::path(ToWideCharStr(path.C_Str())).filename();

		DirectX::CreateWICTextureFromFile(device.Get(), fileName.c_str(), nullptr, &m_textureSRVs.specularTextureSRV);
	}
	else
	{
		m_textureSRVs.specularTextureSRV = s_defaultTextureSRVs->specularTextureSRV;
	}

	if (aiReturn_SUCCESS == material->GetTexture(aiTextureType_EMISSIVE, 0, &path))
	{
		std::wstring fileName = fs::path(ToWideCharStr(path.C_Str())).filename();

		DirectX::CreateWICTextureFromFile(device.Get(), fileName.c_str(), nullptr, &m_textureSRVs.emissiveTextureSRV);
	}
	else
	{
		m_textureSRVs.emissiveTextureSRV = s_defaultTextureSRVs->emissiveTextureSRV;
	}

	if (aiReturn_SUCCESS == material->GetTexture(aiTextureType_OPACITY, 0, &path))
	{
		std::wstring fileName = fs::path(ToWideCharStr(path.C_Str())).filename();

		DirectX::CreateWICTextureFromFile(device.Get(), fileName.c_str(), nullptr, &m_textureSRVs.opacityTextureSRV);
	}
	else
	{
		m_textureSRVs.opacityTextureSRV = s_defaultTextureSRVs->opacityTextureSRV;
	}
}

const TextureSRVs& Material::GetTextureSRVs() const
{
	return m_textureSRVs;
}

void Material::CreateDefaultTextureSRV(const Microsoft::WRL::ComPtr<ID3D11Device>& device)
{
	if (s_defaultTextureSRVs != nullptr)
	{
		return;
	}

	enum TextureType
	{
		Diffuse,
		Normal,
		Specular,
		Emissive,
		Opacity,
		Max
	};

	unsigned char diffuseData[]{ 255, 255, 255, 255 };
	unsigned char normalData[]{ 128, 128, 255, 255 };
	unsigned char blackData[]{ 0, 0, 0, 255 };
	unsigned char whiteData[]{ 255, 255, 255, 255 };

	s_defaultTextureSRVs = new TextureSRVs;

	for (int i = 0; i < TextureType::Max; ++i)
	{
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = 1;
		texDesc.Height = 1;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_IMMUTABLE;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA subData{};
		switch (i)
		{
		case Diffuse:
			subData.pSysMem = diffuseData;
			break;
		case Normal:
			subData.pSysMem = normalData;
			break;
		case Specular:
			subData.pSysMem = blackData;
			break;
		case Emissive:
			subData.pSysMem = blackData;
			break;
		case Opacity:
			subData.pSysMem = whiteData;
			break;
		}

		subData.SysMemPitch = 4;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;

		device->CreateTexture2D(&texDesc, &subData, &texture);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		switch (i)
		{
		case Diffuse:
			device->CreateShaderResourceView(texture.Get(), &srvDesc, &s_defaultTextureSRVs->diffuseTextureSRV);
			break;
		case Normal:
			device->CreateShaderResourceView(texture.Get(), &srvDesc, &s_defaultTextureSRVs->normalTextureSRV);
			break;
		case Specular:
			device->CreateShaderResourceView(texture.Get(), &srvDesc, &s_defaultTextureSRVs->specularTextureSRV);
			break;
		case Emissive:
			device->CreateShaderResourceView(texture.Get(), &srvDesc, &s_defaultTextureSRVs->emissiveTextureSRV);
			break;
		case Opacity:
			device->CreateShaderResourceView(texture.Get(), &srvDesc, &s_defaultTextureSRVs->opacityTextureSRV);
			break;
		}
	}
}

void Material::DestroyDefaultTextureSRV()
{
	delete s_defaultTextureSRVs;

	s_defaultTextureSRVs = nullptr;
}

const TextureSRVs& Material::GetDefaultTextureSRVs()
{
	return *s_defaultTextureSRVs;
}