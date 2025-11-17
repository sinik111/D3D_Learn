#include "AssetManager.h"

#include "FBXAssetData.h"
#include "StaticMeshData.h"
#include "MaterialData.h"

AssetManager& AssetManager::Get()
{
	static AssetManager s_instance;

	return s_instance;
}

std::shared_ptr<AssetData> AssetManager::GetOrCreateAsset(AssetKey key)
{
	auto find = m_assets.find(key);
	if (find != m_assets.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<AssetData> assetData;

	switch (key.kind)
	{
	case AssetKind::StaticMesh:
	{
		FBXAssetData fbx;
		fbx.Create(FBXAssetKind::Static, key.path);

		const auto& meshData = fbx.GetStaticMeshData();
		const auto& materialData = fbx.GetMaterialData();

		m_assets[key] = meshData;
		m_assets[{ key.path, AssetKind::Material }] = materialData;

		assetData = meshData;

		m_tempAssets[m_tempAssetIndex % m_tempAssets.size()] = meshData;
		m_tempAssets[m_tempAssetIndex % m_tempAssets.size()] = materialData;
	}
		break;

	case AssetKind::Material:
	{
		FBXAssetData fbx;
		fbx.Create(FBXAssetKind::Static, key.path);

		const auto& meshData = fbx.GetStaticMeshData();
		const auto& materialData = fbx.GetMaterialData();

		m_assets[{ key.path, AssetKind::StaticMesh }] = meshData;
		m_assets[key] = materialData;

		assetData = materialData;

		m_tempAssets[m_tempAssetIndex % m_tempAssets.size()] = meshData;
		m_tempAssets[m_tempAssetIndex % m_tempAssets.size()] = materialData;
	}
		break;
	}

	return assetData;
}