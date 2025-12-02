#include "SkeletalMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <queue>
#include <unordered_map>
#include <functional>

#include "../Common/Helper.h"
#include "../Common/AssetManager.h"
#include "../Common/D3DResourceManager.h"
#include "../Common/ShaderConstant.h"
#include "../Common/SkeletalMeshData.h"
#include "../Common/MaterialData.h"
#include "../Common/ConstantBuffer.h"
#include "../Common/VertexBuffer.h"
#include "../Common/IndexBuffer.h"
#include "../Common/VertexShader.h"
#include "../Common/PixelShader.h"
#include "../Common/InputLayout.h"
#include "../Common/SamplerState.h"
#include "../Common/SkeletonData.h"
#include "../Common/AnimationData.h"
#include "../Common/MaterialHelper.h"

using DirectX::SimpleMath::Matrix;

SkeletalMesh::SkeletalMesh(const std::wstring& filePath)
{
	m_skeletalMeshData = AssetManager::Get().GetOrCreateSkeletalMeshAsset(filePath);
	m_materialData = AssetManager::Get().GetOrCreateMaterialAsset(filePath);
	m_animationData = AssetManager::Get().GetOrCreateAnimationAsset(filePath);
	m_skeletonData = AssetManager::Get().GetOrCreateSkeletonAsset(filePath);

	if (m_skeletalMeshData->IsRigid())
	{
		m_vertexBuffer = D3DResourceManager::Get().GetOrCreateVertexBuffer(filePath, m_skeletalMeshData->GetVertices());
		m_finalPassVertexShader = D3DResourceManager::Get().GetOrCreateVertexShader(L"RigidAnimVS.hlsl");
		m_shadowPassVertexShader = D3DResourceManager::Get().GetOrCreateVertexShader(L"RigidAnimLightViewVS.hlsl");
		const auto layout = CommonVertex3D::GetLayout();
		m_inputLayout = D3DResourceManager::Get().GetOrCreateInputLayout(L"RigidAnimVS.hlsl", layout.data(), static_cast<UINT>(layout.size()));
	}
	else
	{
		m_vertexBuffer = D3DResourceManager::Get().GetOrCreateVertexBuffer(filePath, m_skeletalMeshData->GetBoneWeightVertices());
		m_boneOffsetBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"BoneOffset", sizeof(Matrix) * MAX_BONE_NUM);
		m_finalPassVertexShader = D3DResourceManager::Get().GetOrCreateVertexShader(L"SkinningAnimVS.hlsl");
		m_shadowPassVertexShader = D3DResourceManager::Get().GetOrCreateVertexShader(L"SkinningAnimLightViewVS.hlsl");
		const auto layout = BoneWeightVertex3D::GetLayout();
		m_inputLayout = D3DResourceManager::Get().GetOrCreateInputLayout(L"SkinningAnimVS.hlsl", layout.data(), static_cast<UINT>(layout.size()));
	}
	m_indexBuffer = D3DResourceManager::Get().GetOrCreateIndexBuffer(filePath, m_skeletalMeshData->GetIndices());
	m_materialBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"Material", sizeof(MaterialBuffer));
	m_worldTransformBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"WorldTransform", sizeof(WorldTransformBuffer));
	m_bonePoseBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"BonePose", sizeof(Matrix) * MAX_BONE_NUM);
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
		//MaterialHelper::SetupMaterialVector(materialCB.ambient, material, MaterialKey::AMBIENT_COLOR); // 다 0임..
		MaterialHelper::SetupMaterialVector(materialCB.specular, material, MaterialKey::SPECULAR_COLOR);

		MaterialHelper::SetupMaterialScalar(materialCB.shininess, material, MaterialKey::SHININESS_FACTOR);

		m_textureSRVs.push_back(std::move(srvs));
		m_materialCBs.push_back(materialCB);
	}

	{
		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		m_samplerState = D3DResourceManager::Get().GetOrCreateSamplerState(L"Linear", samplerDesc);
	}

	{
		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;

		m_comparisonSamplerState = D3DResourceManager::Get().GetOrCreateSamplerState(L"Comparison", samplerDesc);
	}

	// 인스턴스 데이터 생성
	m_skeletonData->SetupSkeletonInstance(m_skeleton);
}

void SkeletalMesh::SetWorld(const Matrix& world) 
{
	m_worldTransformCB.world = world;
}

void SkeletalMesh::Update(float deltaTime)
{
	const auto& animations = m_animationData->GetAnimations();

	if (!animations.empty())
	{
		m_animationProgressTime += deltaTime;
		m_animationProgressTime = std::fmod(m_animationProgressTime, animations[m_animationIndex].duration);
	}

	for (auto& bone : m_skeleton)
	{
		if (bone.boneAnimation != nullptr)
		{
			DirectX::SimpleMath::Vector3 position, scale;
			DirectX::SimpleMath::Quaternion rotation;
			bone.boneAnimation->Evaluate(m_animationProgressTime, bone.lastKeyIndex, position, rotation, scale);
			bone.local = Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) * Matrix::CreateTranslation(position);
		}

		if (bone.parentIndex != -1)
		{
			bone.model = bone.local * m_skeleton[bone.parentIndex].model;
		}
		else
		{
			bone.model = bone.local;
		}

		m_skeletonPose[bone.index] = bone.model.Transpose();
	}
}

void SkeletalMesh::PlayAnimation(size_t index)
{
	m_animationIndex = index;
	m_animationProgressTime = 0.0f;
	m_animationData->GetAnimations()[index].SetupBoneAnimation(m_skeleton);
}

void SkeletalMesh::Draw(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& deviceContext)
{
	static const UINT s_vertexBufferOffset = 0;
	const UINT s_vertexBufferStride = m_vertexBuffer->GetBufferStride();

	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer->GetBuffer().GetAddressOf(), &s_vertexBufferStride, &s_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_indexBuffer->GetRawBuffer(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetInputLayout(m_inputLayout->GetRawInputLayout());

	deviceContext->VSSetShader(m_finalPassVertexShader->GetRawShader(), nullptr, 0);
	deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::WorldTransform),
		1, m_worldTransformBuffer->GetBuffer().GetAddressOf());
	deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::BonePoseMatrix),
		1, m_bonePoseBuffer->GetBuffer().GetAddressOf());
	deviceContext->UpdateSubresource(m_bonePoseBuffer->GetRawBuffer(), 0, nullptr, m_skeletonPose.data(), 0, 0);

	deviceContext->PSSetSamplers(0, 1, m_samplerState->GetSamplerState().GetAddressOf());
	deviceContext->PSSetSamplers(1, 1, m_comparisonSamplerState->GetSamplerState().GetAddressOf());
	deviceContext->PSSetShader(m_finalPassPixelShader->GetRawShader(), nullptr, 0);
	deviceContext->PSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::Material),
		1, m_materialBuffer->GetBuffer().GetAddressOf());

	const auto& meshSections = m_skeletalMeshData->GetMeshSections();

	if (m_skeletalMeshData->IsRigid())
	{
		for (const auto& meshSection : meshSections)
		{
			m_worldTransformCB.refBoneIndex = meshSection.m_boneReference;
			deviceContext->UpdateSubresource(m_worldTransformBuffer->GetRawBuffer(), 0, nullptr, &m_worldTransformCB, 0, 0);

			const auto textureSRVs = m_textureSRVs[meshSection.materialIndex].AsRawArray();

			deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());
			deviceContext->UpdateSubresource(m_materialBuffer->GetRawBuffer(), 0, nullptr, &m_materialCBs[meshSection.materialIndex], 0, 0);
			deviceContext->DrawIndexed(meshSection.indexCount, meshSection.indexOffset, meshSection.vertexOffset);
		}
	}
	else
	{
		deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::BoneOffsetMatrix),
			1, m_boneOffsetBuffer->GetBuffer().GetAddressOf());
		deviceContext->UpdateSubresource(m_worldTransformBuffer->GetRawBuffer(), 0, nullptr, &m_worldTransformCB, 0, 0);
		deviceContext->UpdateSubresource(m_boneOffsetBuffer->GetRawBuffer(), 0, nullptr, m_skeletonData->GetBoneOffsets().data(), 0, 0);

		for (const auto& meshSection : meshSections)
		{
			const auto textureSRVs = m_textureSRVs[meshSection.materialIndex].AsRawArray();

			deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());
			deviceContext->UpdateSubresource(m_materialBuffer->GetRawBuffer(), 0, nullptr, &m_materialCBs[meshSection.materialIndex], 0, 0);
			deviceContext->DrawIndexed(meshSection.indexCount, meshSection.indexOffset, meshSection.vertexOffset);
		}
	}
}

void SkeletalMesh::DrawShadowMap(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& deviceContext)
{
	static const UINT s_vertexBufferOffset = 0;
	const UINT s_vertexBufferStride = m_vertexBuffer->GetBufferStride();

	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer->GetBuffer().GetAddressOf(), &s_vertexBufferStride, &s_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_indexBuffer->GetRawBuffer(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetInputLayout(m_inputLayout->GetRawInputLayout());

	deviceContext->VSSetShader(m_shadowPassVertexShader->GetRawShader(), nullptr, 0);
	deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::WorldTransform),
		1, m_worldTransformBuffer->GetBuffer().GetAddressOf());
	deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::BonePoseMatrix),
		1, m_bonePoseBuffer->GetBuffer().GetAddressOf());
	deviceContext->UpdateSubresource(m_bonePoseBuffer->GetRawBuffer(), 0, nullptr, m_skeletonPose.data(), 0, 0);

	deviceContext->PSSetSamplers(0, 1, m_samplerState->GetSamplerState().GetAddressOf());
	deviceContext->PSSetShader(m_shadowPassPixelShader->GetRawShader(), nullptr, 0);

	const auto& meshSections = m_skeletalMeshData->GetMeshSections();

	if (m_skeletalMeshData->IsRigid())
	{
		for (const auto& meshSection : meshSections)
		{
			auto textureSRV = m_textureSRVs[meshSection.materialIndex].opacityTextureSRV;

			deviceContext->PSSetShaderResources(0, 1, textureSRV->GetShaderResourceView().GetAddressOf());

			m_worldTransformCB.refBoneIndex = meshSection.m_boneReference;
			deviceContext->UpdateSubresource(m_worldTransformBuffer->GetRawBuffer(), 0, nullptr, &m_worldTransformCB, 0, 0);

			deviceContext->DrawIndexed(meshSection.indexCount, meshSection.indexOffset, meshSection.vertexOffset);
		}
	}
	else
	{
		deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::BoneOffsetMatrix),
			1, m_boneOffsetBuffer->GetBuffer().GetAddressOf());
		deviceContext->UpdateSubresource(m_worldTransformBuffer->GetRawBuffer(), 0, nullptr, &m_worldTransformCB, 0, 0);
		deviceContext->UpdateSubresource(m_boneOffsetBuffer->GetRawBuffer(), 0, nullptr, m_skeletonData->GetBoneOffsets().data(), 0, 0);

		for (const auto& meshSection : meshSections)
		{
			auto textureSRV = m_textureSRVs[meshSection.materialIndex].opacityTextureSRV;

			deviceContext->PSSetShaderResources(0, 1, textureSRV->GetShaderResourceView().GetAddressOf());

			deviceContext->DrawIndexed(meshSection.indexCount, meshSection.indexOffset, meshSection.vertexOffset);
		}
	}
}