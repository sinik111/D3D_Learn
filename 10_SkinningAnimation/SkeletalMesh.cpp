#include "SkeletalMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <queue>
#include <unordered_map>
#include <functional>

#include "../Common/Helper.h"

#include "SkeletalMeshSection.h"
#include "Material.h"
#include "Animation.h"

using DirectX::SimpleMath::Matrix;

struct SkeletalMeshResource
{
	std::vector<SkeletalMeshSection> meshes;
	std::vector<Material> materials;
	std::vector<Animation> animations;
	std::unique_ptr<SkeletonInfo> skeletonInfo;
};

namespace
{
	std::unordered_map<std::wstring, std::weak_ptr<SkeletalMeshResource>> g_resourceMap; // 埃窜茄 府家胶 概聪历
}

SkeletalMesh::SkeletalMesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::string& fileName, const Matrix& world)
	: m_name{ std::filesystem::path(fileName).stem().wstring() }, m_world{ world }
{
	bool exist = false;

	auto it = g_resourceMap.find(m_name);
	if (it != g_resourceMap.end())
	{
		if (!it->second.expired())
		{
			m_resource = it->second.lock();

			exist = true;
		}
	}

	if (!exist)
	{
		m_resource = std::make_shared<SkeletalMeshResource>();

		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);

		unsigned int importFlags =
			aiProcess_Triangulate |
			aiProcess_GenNormals |
			aiProcess_GenUVCoords |
			aiProcess_CalcTangentSpace |
			aiProcess_LimitBoneWeights |
			aiProcess_ConvertToLeftHanded;

		const aiScene* scene = importer.ReadFile(fileName, importFlags);

		// bone 积己
		m_resource->skeletonInfo = std::make_unique<SkeletonInfo>(scene);

		// mesh sections 积己
		m_resource->meshes.reserve(scene->mNumMeshes);

		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			const aiMesh* mesh = scene->mMeshes[i];
			m_resource->meshes.emplace_back(device, mesh, m_resource->skeletonInfo.get());
		}

		// material 积己
		m_resource->materials.reserve(scene->mNumMaterials);

		for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
		{
			m_resource->materials.emplace_back(device, scene->mMaterials[i]);
		}

		// animation 积己
		m_resource->animations.reserve(scene->mNumAnimations);
		
		for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
		{
			m_resource->animations.emplace_back(scene->mAnimations[i], m_resource->skeletonInfo.get());
		}

		g_resourceMap[m_name] = m_resource;
	}

	// 牢胶畔胶 单捞磐 积己
	m_resource->skeletonInfo->SetupSkeletonInstance(m_skeleton);
}

const std::wstring& SkeletalMesh::GetName() const 
{
	return m_name;
}

const std::vector<SkeletalMeshSection>& SkeletalMesh::GetMeshes() const
{
	return m_resource->meshes;
}

const std::vector<Material>& SkeletalMesh::GetMaterials() const 
{
	return m_resource->materials;
}

const Matrix& SkeletalMesh::GetWorld() const 
{
	return m_world;
}

const BoneMatrixArray& SkeletalMesh::GetSkeletonPose() const
{
	return m_skeletonPose;
}

const BoneMatrixArray& SkeletalMesh::GetBoneOffsets() const
{
	return m_resource->skeletonInfo->GetBoneOffsets();
}

void SkeletalMesh::SetWorld(const Matrix& world) 
{
	m_world = world;
}

void SkeletalMesh::Update(float deltaTime)
{
	if (!m_resource->animations.empty())
	{
		m_animationProgressTime += deltaTime;
		m_animationProgressTime = std::fmod(m_animationProgressTime, m_resource->animations[m_animationIndex].GetDuration());
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
	m_resource->animations[index].SetupBoneAnimation(m_skeleton);
}