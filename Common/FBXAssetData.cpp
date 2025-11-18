#include "FBXAssetData.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>

#include "../Common/Helper.h"

#include "StaticMeshData.h"
#include "MaterialData.h"

void FBXAssetData::Create(FBXAssetKind kind, const std::wstring& filePath)
{
	switch (kind)
	{
	case FBXAssetKind::Static:
		LoadStaticMesh(filePath);
		break;

	case FBXAssetKind::Rigid:
		break;

	case FBXAssetKind::Skinning:
		break;
	}
}

std::shared_ptr<StaticMeshData> FBXAssetData::GetStaticMeshData() const
{
    return m_staticMesh;
}

std::shared_ptr<MaterialData> FBXAssetData::GetMaterialData() const
{
    return m_material;
}

void FBXAssetData::LoadStaticMesh(const std::wstring& filePath)
{
	Assimp::Importer importer;

	unsigned int importFlags =
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_GenUVCoords |
		aiProcess_CalcTangentSpace |
		aiProcess_ConvertToLeftHanded |
		aiProcess_PreTransformVertices;

	const aiScene* scene = importer.ReadFile(ToMultibyteStr(filePath), importFlags);

	m_staticMesh->Create(scene);
	m_material->Create(scene);
}
