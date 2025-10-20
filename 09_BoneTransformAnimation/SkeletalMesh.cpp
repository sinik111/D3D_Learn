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

	const aiNode* meshRootNode = rootNode->mChildren[0]; // Mesh의 최상위 노드(주로 pelvis)

	const size_t nodeCount = GetNodeCount(meshRootNode);
	m_boneInfos->reserve(nodeCount);

	m_boneInfos->emplace_back(meshRootNode->mTransformation[0], -1); // 루트 index 0

	std::queue<std::pair<aiNode*, int>> nodeQueue; // 상위 노드 순으로 만들기 위한 큐
	nodeQueue.push({ rootNode->mChildren[0], 0 });

	while (!nodeQueue.empty())
	{
		auto [ node, parentIndex ] = nodeQueue.front();
		nodeQueue.pop();

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			aiNode* child = node->mChildren[i];

			nodeQueue.push({ child, static_cast<int>(m_boneInfos->size()) });
			m_boneInfos->emplace_back(child->mTransformation[0], parentIndex);
		}
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

SkeletalMesh::SkeletalMesh(const SkeletalMesh& other, const Matrix& world)
{
	*this = other;

	m_world = world;
}

SkeletalMesh::SkeletalMesh(SkeletalMesh&& other) noexcept
	: m_meshes{ std::move(other.m_meshes) }, m_materials{ std::move(other.m_materials) },
	m_animations{ std::move(other.m_animations) }, m_boneInfos{ std::move(other.m_boneInfos) },
	m_name{ std::move(other.m_name) }, m_world{ other.m_world }, m_bones{ std::move(other.m_bones) },
	m_models{ std::move(other.m_models) }
{

}

SkeletalMesh& SkeletalMesh::operator=(SkeletalMesh&& other) noexcept
{
	if (this != &other)
	{
		m_meshes = std::move(other.m_meshes);
		m_materials = std::move(other.m_materials);
		m_animations = std::move(other.m_animations);
		m_boneInfos = std::move(other.m_boneInfos);
		m_name = std::move(other.m_name);
		m_world = other.m_world;
		m_bones = std::move(other.m_bones);
		m_models = std::move(other.m_models);
	}

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