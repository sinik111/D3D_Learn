#include "SkeletalMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <queue>

#include "../Common/Helper.h"

using Matrix = DirectX::SimpleMath::Matrix;

static size_t GetNodeCount(const aiNode* node)
{
	size_t count = 1;

	if (node->mNumChildren > 0)
	{
		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			count += GetNodeCount(node->mChildren[i]);
		}
	}

	return count;
}

SkeletalMesh::SkeletalMesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const char* fileName, const Matrix& world)
	: m_name{ std::filesystem::path(fileName).stem().c_str() },
	m_meshes{ std::make_shared<std::vector<SkeletalMeshSection>>() },
	m_materials{ std::make_shared<std::vector<Material>>() },
	m_animations{ std::make_shared<std::vector<Animation>>() },
	m_boneInfos{ std::make_shared<std::vector<BoneInfo>>() }
{
	Assimp::Importer importer;

	unsigned int importFlags = aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_GenUVCoords |
		aiProcess_CalcTangentSpace |
		aiProcess_ConvertToLeftHanded;

	const aiScene* scene = importer.ReadFile(fileName, importFlags);

	const aiNode* rootNode = scene->mRootNode; // RootNode, Scene 최상위 노드
	aiAnimation** animation = scene->mAnimations;

	const size_t nodeCount = GetNodeCount(rootNode->mChildren[0]);
	m_boneInfos->reserve(nodeCount);

	std::queue<aiNode*> nodeQueue; // 상위 노드 순으로 만들기 위한 큐
	nodeQueue.push(rootNode->mChildren[0]);

	m_boneInfos->emplace_back(rootNode->mTransformation[0], -1);

	int parentBoneIndex = 0;

	while (!nodeQueue.empty())
	{
		const aiNode* node = nodeQueue.front();
		nodeQueue.pop();

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			aiNode* child = node->mChildren[i];

			m_boneInfos->emplace_back(child->mTransformation[0], parentBoneIndex);
			nodeQueue.push(child);
		}

		++parentBoneIndex;
	}

	//m_meshes->reserve(scene->mNumMeshes);

	//for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	//{
	//	m_meshes->emplace_back(device, scene->mMeshes[i]);
	//}

	//m_materials->reserve(scene->mNumMaterials);

	//for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	//{
	//	m_materials->emplace_back(device, scene->mMaterials[i]);
	//}
}

SkeletalMesh::SkeletalMesh(const SkeletalMesh& other)
{
}

SkeletalMesh::SkeletalMesh(const SkeletalMesh& other, const Matrix& world)
{
}

SkeletalMesh& SkeletalMesh::operator=(const SkeletalMesh& other)
{
	return *this;
}

SkeletalMesh::SkeletalMesh(SkeletalMesh&& other)
{
}

SkeletalMesh& SkeletalMesh::operator=(SkeletalMesh&& other)
{
	return *this;
}

const std::wstring& SkeletalMesh::GetName() const 
{
	return m_name;
}

const std::vector<SkeletalMeshSection>& SkeletalMesh::GetMeshes() const
{
	return *m_meshes.get();
}

const std::vector<Material>& SkeletalMesh::GetMaterials() const 
{
	return *m_materials.get();
}

const Matrix& SkeletalMesh::GetWorld() const 
{
	return m_world;
}

const std::array<Matrix, 128>& SkeletalMesh::GetModels() const 
{
	return m_models;
}

void SkeletalMesh::SetWorld(const Matrix& world) 
{
	m_world = world;
}