#include "StaticMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>

#include "../Common/Helper.h"
#include "../Common/ShaderResourceView.h"
#include "../Common/AssetManager.h"
#include "../Common/D3DResourceManager.h"
#include "../Common/StaticMeshData.h"
#include "../Common/ShaderConstant.h"
#include "../Common/MaterialData.h"

std::array<ID3D11ShaderResourceView*, 5> TextureSRVs::AsRawArray() const
{
	return {
		diffuseTextureSRV->GetShaderResourceView().Get(),
		normalTextureSRV->GetShaderResourceView().Get(),
		specularTextureSRV->GetShaderResourceView().Get(),
		emissiveTextureSRV->GetShaderResourceView().Get(),
		opacityTextureSRV->GetShaderResourceView().Get()
	};
}

StaticMesh::StaticMesh(const std::wstring& filePath)
{
	m_staticMeshData = AssetManager::Get().GetOrCreateStaticMeshAsset(filePath);
	m_materialData = AssetManager::Get().GetOrCreateMaterialAsset(filePath);

	m_vertexBuffer = D3DResourceManager::Get().GetOrCreateVertexBuffer(filePath, m_staticMeshData->GetVertices());
	m_indexBuffer = D3DResourceManager::Get().GetOrCreateIndexBuffer(filePath, m_staticMeshData->GetIndices());
	m_materialBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"Material", sizeof(MaterialBuffer));
	m_worldTransformBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"WorldTransform", sizeof(WorldTransformBuffer));
	m_finalPassVertexShader = D3DResourceManager::Get().GetOrCreateVertexShader(L"BasicVS.hlsl");
	m_shadowPassVertexShader = D3DResourceManager::Get().GetOrCreateVertexShader(L"BasicLightViewVS.hlsl");
	m_finalPassPixelShader = D3DResourceManager::Get().GetOrCreatePixelShader(L"BlinnPhongPS.hlsl");
	m_shadowPassPixelShader = D3DResourceManager::Get().GetOrCreatePixelShader(L"LightViewPS.hlsl");

	const auto& materials = m_materialData->GetMaterials();
	m_textureSRVs.reserve(materials.size());

	static const D3D11_TEXTURE2D_DESC s_texDesc{
		1,
		1,
		1,
		1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		{ 1, 0 },
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_SHADER_RESOURCE,
		0,
		0
	};

	for (const auto& material : materials)
	{
		TextureSRVs srvs{};

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::DIFFUSE_TEXTURE))
		{
			srvs.diffuseTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				material.texturePaths.at(MaterialKey::DIFFUSE_TEXTURE), TextureType::Texture2D);
		}
		else
		{
			static const unsigned char s_data[4]{ 255, 255, 255, 255 };

			D3D11_SUBRESOURCE_DATA subData{};
			subData.pSysMem = s_data;
			subData.SysMemPitch = 4;

			srvs.diffuseTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				std::to_wstring(*(unsigned int*)s_data), s_texDesc, subData);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::DIFFUSE_COLOR))
		{
			m_materialCB.diffuse = material.vectorValues.at(MaterialKey::DIFFUSE_COLOR);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::AMBIENT_COLOR))
		{
			m_materialCB.ambient = material.vectorValues.at(MaterialKey::AMBIENT_COLOR);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::NORMAL_TEXTURE))
		{
			srvs.normalTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				material.texturePaths.at(MaterialKey::NORMAL_TEXTURE), TextureType::Texture2D);
		}
		else
		{
			static const unsigned char s_data[4]{ 128, 128, 255, 255 };

			D3D11_SUBRESOURCE_DATA subData{};
			subData.pSysMem = s_data;
			subData.SysMemPitch = 4;

			srvs.normalTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				std::to_wstring(*(unsigned int*)s_data), s_texDesc, subData);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::SPECULAR_TEXTURE))
		{
			srvs.specularTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				material.texturePaths.at(MaterialKey::SPECULAR_TEXTURE), TextureType::Texture2D);
		}
		else
		{
			static const unsigned char s_data[4]{ 0, 0, 0, 0 };

			D3D11_SUBRESOURCE_DATA subData{};
			subData.pSysMem = s_data;
			subData.SysMemPitch = 4;

			srvs.specularTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				std::to_wstring(*(unsigned int*)s_data), s_texDesc, subData);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::SPECULAR_COLOR))
		{
			m_materialCB.specular = material.vectorValues.at(MaterialKey::DIFFUSE_COLOR);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::SHININESS_FACTOR))
		{
			m_materialCB.shininess = material.scalarValues.at(MaterialKey::SHININESS_FACTOR);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::EMISSIVE_TEXTURE))
		{
			srvs.emissiveTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				material.texturePaths.at(MaterialKey::EMISSIVE_TEXTURE), TextureType::Texture2D);
		}
		else
		{
			static const unsigned char s_data[4]{ 0, 0, 0, 0 };

			D3D11_SUBRESOURCE_DATA subData{};
			subData.pSysMem = s_data;
			subData.SysMemPitch = 4;

			srvs.emissiveTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				std::to_wstring(*(unsigned int*)s_data), s_texDesc, subData);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::OPACITY_TEXTURE))
		{
			srvs.opacityTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				material.texturePaths.at(MaterialKey::OPACITY_TEXTURE), TextureType::Texture2D);
		}
		else
		{
			static const unsigned char s_data[4]{ 255, 255, 255, 255 };

			D3D11_SUBRESOURCE_DATA subData{};
			subData.pSysMem = s_data;
			subData.SysMemPitch = 4;

			srvs.opacityTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				std::to_wstring(*(unsigned int*)s_data), s_texDesc, subData);
		}

		m_textureSRVs.push_back(std::move(srvs));
	}
	
	static const auto& s_layout = CommonVertex3D::GetLayout();
	m_inputLayout = D3DResourceManager::Get().GetOrCreateInputLayout(L"BasicVS.hlsl", s_layout.data(), static_cast<UINT>(s_layout.size()));

	static const D3D11_SAMPLER_DESC s_samplerDesc{
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_WRAP,
		0.0f,
		0,
		D3D11_COMPARISON_NEVER,
		{},
		0,
		D3D11_FLOAT32_MAX
	};

	m_samplerState = D3DResourceManager::Get().GetOrCreateSamplerState(L"Linear", s_samplerDesc);
}

void StaticMesh::DrawShadowMap(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& deviceContext)
{
}

void StaticMesh::Draw(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& deviceContext)
{
}