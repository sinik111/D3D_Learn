#include "StaticMeshData.h"

#include <climits>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../Common/Helper.h"

void StaticMeshData::Create(const std::wstring& filePath)
{
	Assimp::Importer importer;

	unsigned int importFlags = aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_GenUVCoords |
		aiProcess_CalcTangentSpace |
		aiProcess_ConvertToLeftHanded |
		aiProcess_PreTransformVertices;

	const aiScene* scene = importer.ReadFile(ToMultibyteStr(filePath), importFlags);

	Create(scene);
}

void StaticMeshData::Create(const aiScene* scene)
{
	m_meshSections.reserve(scene->mNumMeshes);

	int totalVertices = 0;
	unsigned int totalIndices = 0;

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[i];

		m_meshSections.push_back(
			{
				ToWideCharStr(mesh->mName.C_Str()),
				mesh->mMaterialIndex,
				totalVertices,
				totalIndices,
				mesh->mNumFaces * 3
			}
		);

		totalVertices += mesh->mNumVertices;
		totalIndices += mesh->mNumFaces * 3;
	}

	m_vertices.reserve(totalVertices);
	m_indices.reserve(totalIndices);

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[i];

		const unsigned int numVertices = mesh->mNumVertices;
		const unsigned int numFaces = mesh->mNumFaces;

		const INT currentVertexOffset = m_meshSections[i].vertexOffset;

		for (unsigned int j = 0; j < numVertices; ++j)
		{
			m_vertices.emplace_back(
				&mesh->mVertices[j].x,
				&mesh->mTextureCoords[0][j].x,
				&mesh->mNormals[j].x,
				&mesh->mTangents[j].x,
				&mesh->mBitangents[j].x);
		}

		for (unsigned int j = 0; i < numFaces; ++j)
		{
			m_indices.push_back(mesh->mFaces[j].mIndices[0] + currentVertexOffset);
			m_indices.push_back(mesh->mFaces[j].mIndices[1] + currentVertexOffset);
			m_indices.push_back(mesh->mFaces[j].mIndices[2] + currentVertexOffset);
		}
	}
}

const std::vector<CommonVertex3D>& StaticMeshData::GetVertices() const
{
	return m_vertices;
}

const std::vector<DWORD>& StaticMeshData::GetIndices() const
{
	return m_indices;
}
