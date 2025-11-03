#include "StaticMesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>

#include "../Common/Helper.h"

StaticMesh::StaticMesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const char* fileName, const Matrix& world)
	: m_world{ world }, m_name{ std::filesystem::path(fileName).stem().c_str() }
{
	Assimp::Importer importer;

	unsigned int importFlags = aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_GenUVCoords |
		aiProcess_CalcTangentSpace |
		aiProcess_ConvertToLeftHanded |
		aiProcess_PreTransformVertices;

	const aiScene* scene = importer.ReadFile(fileName, importFlags);

	const aiNode* rootNode = scene->mRootNode;

	m_meshes.reserve(scene->mNumMeshes);

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		m_meshes.emplace_back(device, scene->mMeshes[i]);
	}

	m_materials.reserve(scene->mNumMaterials);

	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		m_materials.emplace_back(device, scene->mMaterials[i]);
	}
}

const std::wstring& StaticMesh::GetName() const
{
	return m_name;
}

const std::vector<StaticMeshSection>& StaticMesh::GetMeshes() const
{
	return m_meshes;
}

const std::vector<Material>& StaticMesh::GetMaterials() const
{
	return m_materials;
}

const StaticMesh::Matrix& StaticMesh::GetWorld() const
{
	return m_world;
}

void StaticMesh::SetWorld(const Matrix& world)
{
	m_world = world;
}