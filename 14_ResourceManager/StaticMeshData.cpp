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
	m_name = ToWideCharStr(scene->mName.C_Str());

	m_meshSections.reserve(scene->mNumMeshes);

	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		StaticMeshSection meshSection{};

		const aiMesh* mesh = scene->mMeshes[i];

		meshSection.name = ToWideCharStr(mesh->mName.C_Str());

		const unsigned int numVertices = mesh->mNumVertices;
		const unsigned int numFaces = mesh->mNumFaces;
		
		meshSection.vertices.reserve(numVertices);

		for (unsigned int i = 0; i < numVertices; ++i)
		{
			meshSection.vertices.emplace_back(
				&mesh->mVertices[i].x,
				&mesh->mTextureCoords[0][i].x,
				&mesh->mNormals[i].x,
				&mesh->mTangents[i].x,
				&mesh->mBitangents[i].x);
		}

		meshSection.indexCount = numFaces * 3;

		if (numVertices > USHRT_MAX)
		{
			meshSection.indexFormat = IndexFormat::R32_UINT;

			meshSection.indices32.reserve(meshSection.indexCount);

			for (unsigned int i = 0; i < numFaces; ++i)
			{
				meshSection.indices32.push_back(mesh->mFaces[i].mIndices[0]);
				meshSection.indices32.push_back(mesh->mFaces[i].mIndices[1]);
				meshSection.indices32.push_back(mesh->mFaces[i].mIndices[2]);
			}
		}
		else
		{
			meshSection.indexFormat = IndexFormat::R16_UINT;

			meshSection.indices16.reserve(meshSection.indexCount);

			for (unsigned int i = 0; i < numFaces; ++i)
			{
				meshSection.indices16.push_back(mesh->mFaces[i].mIndices[0]);
				meshSection.indices16.push_back(mesh->mFaces[i].mIndices[1]);
				meshSection.indices16.push_back(mesh->mFaces[i].mIndices[2]);
			}
		}

		m_meshSections.push_back(std::move(meshSection));
	}
}