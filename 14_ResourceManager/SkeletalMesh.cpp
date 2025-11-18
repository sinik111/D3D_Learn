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

using DirectX::SimpleMath::Matrix;

SkeletalMesh::SkeletalMesh(const std::wstring& filePath)
{
	m_skeletalMeshData = AssetManager::Get().GetOrCreateSkeletalMeshAsset(filePath);
	m_materialData = AssetManager::Get().GetOrCreateMaterialAsset(filePath);
	m_animationData = AssetManager::Get().GetOrCreateAnimationAsset(filePath);
	m_skeletonData = AssetManager::Get().GetOrCreateSkeletonAsset(filePath);

	bool isRigid = m_skeletalMeshData->IsRigid();

	if (isRigid)
	{
		m_vertexBuffer = D3DResourceManager::Get().GetOrCreateVertexBuffer(filePath, m_skeletalMeshData->GetVertices());
		m_finalPassVertexShader = D3DResourceManager::Get().GetOrCreateVertexShader(L"RigidAnimVS.hlsl");
		m_shadowPassVertexShader = D3DResourceManager::Get().GetOrCreateVertexShader(L"RigidAnimLightViewVS.hlsl");
		static const auto s_layout = CommonVertex3D::GetLayout();
		m_inputLayout = D3DResourceManager::Get().GetOrCreateInputLayout(L"RigidAnimVS.hlsl", s_layout.data(), static_cast<UINT>(s_layout.size()));
	}
	else
	{
		m_vertexBuffer = D3DResourceManager::Get().GetOrCreateVertexBuffer(filePath, m_skeletalMeshData->GetBoneWeightVertices());
		m_boneOffsetBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"BoneOffset", sizeof(Matrix) * MAX_BONE_NUM);
		m_finalPassVertexShader = D3DResourceManager::Get().GetOrCreateVertexShader(L"SkinningAnimVS.hlsl");
		m_shadowPassVertexShader = D3DResourceManager::Get().GetOrCreateVertexShader(L"SkinningAnimLightViewVS.hlsl");
		static const auto s_layout = BoneWeightVertex3D::GetLayout();
		m_inputLayout = D3DResourceManager::Get().GetOrCreateInputLayout(L"SkinningAnimVS.hlsl", s_layout.data(), static_cast<UINT>(s_layout.size()));
	}
	m_indexBuffer = D3DResourceManager::Get().GetOrCreateIndexBuffer(filePath, m_skeletalMeshData->GetIndices());
	m_materialBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"Material", sizeof(MaterialBuffer));
	m_worldTransformBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"WorldTransform", sizeof(WorldTransformBuffer));
	m_bonePoseBuffer = D3DResourceManager::Get().GetOrCreateConstantBuffer(L"BonePose", sizeof(Matrix) * MAX_BONE_NUM);
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
		MaterialBuffer materialCB{};

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::DIFFUSE_TEXTURE))
		{
			srvs.diffuseTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				material.texturePaths.at(MaterialKey::DIFFUSE_TEXTURE), TextureType::Texture2D);
		}
		else
		{
			static const unsigned char s_data[4]{ 255, 255, 255, 255 };
			static const D3D11_SUBRESOURCE_DATA subData{ s_data, 4 };

			srvs.diffuseTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(L"DummyTexWhite", s_texDesc, subData);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::DIFFUSE_COLOR))
		{
			materialCB.diffuse = material.vectorValues.at(MaterialKey::DIFFUSE_COLOR);
		}

		materialCB.ambient = { 1.0f, 1.0f, 1.0f, 1.0f };

		// 다 0 임..
		//if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::AMBIENT_COLOR))
		//{
		//	materialCB.ambient = material.vectorValues.at(MaterialKey::AMBIENT_COLOR);
		//}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::NORMAL_TEXTURE))
		{
			srvs.normalTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				material.texturePaths.at(MaterialKey::NORMAL_TEXTURE), TextureType::Texture2D);
		}
		else
		{
			static const unsigned char s_data[4]{ 128, 128, 255, 255 };
			static const D3D11_SUBRESOURCE_DATA subData{ s_data, 4 };

			srvs.normalTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(L"DummyTexFlat", s_texDesc, subData);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::SPECULAR_TEXTURE))
		{
			srvs.specularTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				material.texturePaths.at(MaterialKey::SPECULAR_TEXTURE), TextureType::Texture2D);
		}
		else
		{
			static const unsigned char s_data[4]{ 0, 0, 0, 0 };
			static const D3D11_SUBRESOURCE_DATA subData{ s_data, 4 };

			srvs.specularTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(L"DummyTexBlack", s_texDesc, subData);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::SPECULAR_COLOR))
		{
			materialCB.specular = material.vectorValues.at(MaterialKey::SPECULAR_COLOR);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::SHININESS_FACTOR))
		{
			materialCB.shininess = material.scalarValues.at(MaterialKey::SHININESS_FACTOR);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::EMISSIVE_TEXTURE))
		{
			srvs.emissiveTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				material.texturePaths.at(MaterialKey::EMISSIVE_TEXTURE), TextureType::Texture2D);
		}
		else
		{
			static const unsigned char s_data[4]{ 0, 0, 0, 0 };
			static const D3D11_SUBRESOURCE_DATA subData{ s_data, 4 };

			srvs.emissiveTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(L"DummyTexBlack", s_texDesc, subData);
		}

		if (material.materialFlags & static_cast<unsigned long long>(MaterialKey::OPACITY_TEXTURE))
		{
			srvs.opacityTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(
				material.texturePaths.at(MaterialKey::OPACITY_TEXTURE), TextureType::Texture2D);
		}
		else
		{
			static const unsigned char s_data[4]{ 255, 255, 255, 255 };
			static const D3D11_SUBRESOURCE_DATA subData{ s_data, 4 };

			srvs.opacityTextureSRV = D3DResourceManager::Get().GetOrCreateShaderResourceView(L"DummyTexWhite", s_texDesc, subData);
		}

		m_textureSRVs.push_back(std::move(srvs));
		m_materialCBs.push_back(materialCB);
	}

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
	const bool isRigid = m_skeletalMeshData->IsRigid();

	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer->GetBuffer().GetAddressOf(), &s_vertexBufferStride, &s_vertexBufferOffset);
	deviceContext->IASetIndexBuffer(m_indexBuffer->GetBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetInputLayout(m_inputLayout->GetInputLayout().Get());

	deviceContext->VSSetShader(m_finalPassVertexShader->GetShader().Get(), nullptr, 0);
	deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::WorldTransform),
		1, m_worldTransformBuffer->GetBuffer().GetAddressOf());
	deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::BonePoseMatrix),
		1, m_bonePoseBuffer->GetBuffer().GetAddressOf());
	deviceContext->UpdateSubresource(m_bonePoseBuffer->GetBuffer().Get(), 0, nullptr, m_skeletonPose.data(), 0, 0);
	if (!isRigid)
	{
		deviceContext->VSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::BoneOffsetMatrix),
			1, m_boneOffsetBuffer->GetBuffer().GetAddressOf());
		deviceContext->UpdateSubresource(m_worldTransformBuffer->GetBuffer().Get(), 0, nullptr, &m_worldTransformCB, 0, 0);
		deviceContext->UpdateSubresource(m_boneOffsetBuffer->GetBuffer().Get(), 0, nullptr, m_skeletonData->GetBoneOffsets().data(), 0, 0);
	}

	deviceContext->PSSetSamplers(0, 1, m_samplerState->GetSamplerState().GetAddressOf());
	deviceContext->PSSetShader(m_finalPassPixelShader->GetShader().Get(), nullptr, 0);
	deviceContext->PSSetConstantBuffers(static_cast<UINT>(ConstantBufferSlot::Material),
		1, m_materialBuffer->GetBuffer().GetAddressOf());

	const auto& meshSections = m_skeletalMeshData->GetMeshSections();

	if (isRigid)
	{
		for (const auto& meshSection : meshSections)
		{
			m_worldTransformCB.refBoneIndex = meshSection.m_boneReference;
			deviceContext->UpdateSubresource(m_worldTransformBuffer->GetBuffer().Get(), 0, nullptr, &m_worldTransformCB, 0, 0);

			const auto textureSRVs = m_textureSRVs[meshSection.materialIndex].AsRawArray();

			deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());
			deviceContext->UpdateSubresource(m_materialBuffer->GetBuffer().Get(), 0, nullptr, &m_materialCBs[meshSection.materialIndex], 0, 0);
			deviceContext->DrawIndexed(meshSection.indexCount, meshSection.indexOffset, meshSection.vertexOffset);
		}
	}
	else
	{
		for (const auto& meshSection : meshSections)
		{
			const auto textureSRVs = m_textureSRVs[meshSection.materialIndex].AsRawArray();

			deviceContext->PSSetShaderResources(0, static_cast<UINT>(textureSRVs.size()), textureSRVs.data());
			deviceContext->UpdateSubresource(m_materialBuffer->GetBuffer().Get(), 0, nullptr, &m_materialCBs[meshSection.materialIndex], 0, 0);
			deviceContext->DrawIndexed(meshSection.indexCount, meshSection.indexOffset, meshSection.vertexOffset);
		}
	}
}