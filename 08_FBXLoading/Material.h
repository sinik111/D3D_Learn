#pragma once

#include <string>
#include <d3d11.h>
#include <wrl/client.h>

struct aiMaterial;

struct TextureSRVs
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> specularTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> emissiveTextureSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> opacityTextureSRV;
};

class Material
{
private:
	std::string m_name;
	TextureSRVs m_textureSRVs;

public:
	Material(const Microsoft::WRL::ComPtr<ID3D11Device>& device, aiMaterial* material);

public:
	const TextureSRVs& GetTextureSRVs() const;

public:
	static void CreateDefaultTextureSRV(const Microsoft::WRL::ComPtr<ID3D11Device>& device);
	static void DestroyDefaultTextureSRV();
};