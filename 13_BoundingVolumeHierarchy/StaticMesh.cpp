#include "StaticMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>

#include "../Common/Helper.h"

struct StaticMeshResource
{
	std::vector<StaticMeshSection> meshes;
	std::vector<Material> materials;
	DirectX::BoundingBox boundingBox;
};

namespace
{
	std::unordered_map<std::wstring, std::weak_ptr<StaticMeshResource>> g_resourceMap; // 간단한 리소스 매니저
}

StaticMesh::StaticMesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::string& fileName, const Matrix& world)
	: m_world{ world }, m_name{ std::filesystem::path(fileName).stem().c_str() }
{
	using DirectX::SimpleMath::Vector3;

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
		m_resource = std::make_shared<StaticMeshResource>();

		Assimp::Importer importer;

		unsigned int importFlags = aiProcess_Triangulate |
			aiProcess_GenNormals |
			aiProcess_GenUVCoords |
			aiProcess_GenBoundingBoxes |
			aiProcess_CalcTangentSpace |
			aiProcess_ConvertToLeftHanded |
			aiProcess_PreTransformVertices;

		const aiScene* scene = importer.ReadFile(fileName, importFlags);

		const aiNode* rootNode = scene->mRootNode;

		m_resource->meshes.reserve(scene->mNumMeshes);

		Vector3 min{ INFINITY, INFINITY, INFINITY };
		Vector3 max{ -INFINITY, -INFINITY, -INFINITY };

		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			m_resource->meshes.emplace_back(device, scene->mMeshes[i]);

			const aiAABB& aabb = scene->mMeshes[i]->mAABB;

			min.x = std::min<float>(aabb.mMin.x, min.x);
			min.y = std::min<float>(aabb.mMin.y, min.y);
			min.z = std::min<float>(aabb.mMin.z, min.z);
			max.x = std::max<float>(aabb.mMax.x, max.x);
			max.y = std::max<float>(aabb.mMax.y, max.y);
			max.z = std::max<float>(aabb.mMax.z, max.z);
		}

		m_resource->boundingBox.Extents = (max - min) / 2;
		m_resource->boundingBox.Center = min + m_resource->boundingBox.Extents;

		m_resource->materials.reserve(scene->mNumMaterials);

		for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
		{
			m_resource->materials.emplace_back(device, scene->mMaterials[i]);
		}
	}
}

const std::wstring& StaticMesh::GetName() const
{
	return m_name;
}

const std::vector<StaticMeshSection>& StaticMesh::GetMeshes() const
{
	return m_resource->meshes;
}

const std::vector<Material>& StaticMesh::GetMaterials() const
{
	return m_resource->materials;
}

const StaticMesh::Matrix& StaticMesh::GetWorld() const
{
	return m_world;
}

DirectX::BoundingBox StaticMesh::GetWorldBoundingBox() const
{
	DirectX::BoundingBox worldBoundingBox;
	m_resource->boundingBox.Transform(worldBoundingBox, m_world);

	return worldBoundingBox;
}

void StaticMesh::SetWorld(const Matrix& world)
{
	m_world = world;
}