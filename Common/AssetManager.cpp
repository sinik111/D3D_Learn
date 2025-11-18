#include "AssetManager.h"

#include "FBXAssetData.h"
#include "StaticMeshData.h"
#include "MaterialData.h"

AssetManager& AssetManager::Get()
{
	static AssetManager s_instance;

	return s_instance;
}

std::shared_ptr<StaticMeshData> AssetManager::GetOrCreateStaticMeshAsset(const std::wstring& filePath)
{
	auto find = m_staticMeshAssets.find(filePath);
	if (find != m_staticMeshAssets.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	FBXAssetData fbx;
	fbx.Create(FBXAssetKind::Static, filePath);

	const auto& meshData = fbx.GetStaticMeshData();
	const auto& materialData = fbx.GetMaterialData();

	m_staticMeshAssets[filePath] = meshData;
	m_materialAssets[filePath] = materialData;

	m_tempAssets[m_tempAssetIndex++ % 10] = meshData;
	m_tempAssets[m_tempAssetIndex++ % 10] = materialData;

	return meshData;
}

std::shared_ptr<MaterialData> AssetManager::GetOrCreateMaterialAsset(const std::wstring& filePath)
{
	auto find = m_materialAssets.find(filePath);
	if (find != m_materialAssets.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	FBXAssetData fbx;
	fbx.Create(FBXAssetKind::Static, filePath);

	const auto& meshData = fbx.GetStaticMeshData();
	const auto& materialData = fbx.GetMaterialData();

	m_staticMeshAssets[filePath] = meshData;
	m_materialAssets[filePath] = materialData;

	m_tempAssets[m_tempAssetIndex++ % 10] = meshData;
	m_tempAssets[m_tempAssetIndex++ % 10] = materialData;

	return materialData;
}