#include "MaterialHelper.h"

#include "MaterialData.h"
#include "D3DResourceManager.h"
#include "ShaderResourceView.h"

namespace MaterialHelper
{
	const D3D11_TEXTURE2D_DESC g_texDesc{
		1, 1, 1, 1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		{ 1, 0 },
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_SHADER_RESOURCE,
		0, 0
	};

	void SetupTextureSRV(std::shared_ptr<ShaderResourceView>& srv, const Material& material, MaterialKey key, const std::wstring& dummyName, const unsigned char colorData[4])
	{
		if (material.materialFlags & static_cast<unsigned long long>(key))
		{
			srv = D3DResourceManager::Get().GetOrCreateShaderResourceView(material.texturePaths.at(key), TextureType::Texture2D);
		}
		else
		{
			srv = D3DResourceManager::Get().GetOrCreateShaderResourceView(dummyName, g_texDesc, { colorData, 4 });
		}
	}

	void SetupMaterialVector(DirectX::SimpleMath::Vector4& v, const Material& material, MaterialKey key)
	{
		if (material.materialFlags & static_cast<unsigned long long>(key))
		{
			v = material.vectorValues.at(key);
		}
	}

	void SetupMaterialScalar(float& f, const Material& material, MaterialKey key)
	{
		if (material.materialFlags & static_cast<unsigned long long>(key))
		{
			f = material.scalarValues.at(key);
		}
	}
}