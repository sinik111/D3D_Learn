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
#include "../Common/ConstantBuffer.h"
#include "../Common/VertexBuffer.h"
#include "../Common/IndexBuffer.h"
#include "../Common/VertexShader.h"
#include "../Common/PixelShader.h"
#include "../Common/InputLayout.h"
#include "../Common/SamplerState.h"
#include "../Common/MaterialHelper.h"

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
	m_materialCBs.reserve(materials.size());

	for (const auto& material : materials)
	{
		TextureSRVs srvs{};
		MaterialBuffer materialCB{};

		MaterialHelper::SetupTextureSRV(srvs.diffuseTextureSRV, material, MaterialKey::DIFFUSE_TEXTURE, L"DummyTexWhite", MaterialHelper::WHITE_DATA);
		MaterialHelper::SetupTextureSRV(srvs.normalTextureSRV, material, MaterialKey::NORMAL_TEXTURE, L"DummyTexFlat", MaterialHelper::FLAT_DATA);
		MaterialHelper::SetupTextureSRV(srvs.specularTextureSRV, material, MaterialKey::SPECULAR_TEXTURE, L"DummyTexBlack", MaterialHelper::BLACK_DATA);
		MaterialHelper::SetupTextureSRV(srvs.emissiveTextureSRV, material, MaterialKey::EMISSIVE_TEXTURE, L"DummyTexBlack", MaterialHelper::BLACK_DATA);
		MaterialHelper::SetupTextureSRV(srvs.opacityTextureSRV, material, MaterialKey::OPACITY_TEXTURE, L"DummyTexWhite", MaterialHelper::WHITE_DATA);

		MaterialHelper::SetupMaterialVector(materialCB.diffuse, material, MaterialKey::DIFFUSE_COLOR);
		//MaterialHelper::SetupMaterialVector(materialCB.ambient, material, MaterialKey::AMBIENT_COLOR); // ╢ы 0юс..
		MaterialHelper::SetupMaterialVector(materialCB.specular, material, MaterialKey::SPECULAR_COLOR);

		MaterialHelper::SetupMaterialScalar(materialCB.shininess, material, MaterialKey::SHININESS_FACTOR);

		m_textureSRVs.push_back(std::move(srvs));
		m_materialCBs.push_back(materialCB);
	}
	
	static const auto s_layout = CommonVertex3D::GetLayout();
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

void StaticMesh::Draw(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& deviceContext) const
{
	static const UINT s_vertexBufferOffset = 0;
	const UINT s_vertexBufferStride = m_vertexBuffer->GetBufferStride();

	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer->GetBuffer().GetAddressOf(), &s_vertexBufferStride, &s_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_indexBuffer->GetRawBuffer(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetInputLayout(m_inputLayout->GetRawInputLayout());

	deviceContext->VSSetShader(m_finalPassVertexShader->GetRawShader(), nullptr, 0);
	deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::WorldTransform),
		1, m_worldTransformBuffer->GetBuffer().GetAddressOf());
	deviceContext->UpdateSubresource(m_worldTransformBuffer->GetRawBuffer(), 0, nullptr, &m_worldTransformCB, 0, 0);

	deviceContext->PSSetSamplers(0, 1, m_samplerState->GetSamplerState().GetAddressOf());
	deviceContext->PSSetShader(m_finalPassPixelShader->GetRawShader(), nullptr, 0);
	deviceContext->PSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::Material),
		1, m_materialBuffer->GetBuffer().GetAddressOf());

	const auto& meshSections = m_staticMeshData->GetMeshSections();

	for (const auto& meshSection : meshSections)
	{
		const auto textureSRVs = m_textureSRVs[meshSection.materialIndex].AsRawArray();

		deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());
		deviceContext->UpdateSubresource(m_materialBuffer->GetRawBuffer(), 0, nullptr, &m_materialCBs[meshSection.materialIndex], 0, 0);
		deviceContext->DrawIndexed(meshSection.indexCount, meshSection.indexOffset, meshSection.vertexOffset);
	}
}

void StaticMesh::SetWorld(const DirectX::SimpleMath::Matrix& world)
{
	m_worldTransformCB.world = world;
}