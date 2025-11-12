#include "AssetManager.h"

#include "FBXAssetData.h"
#include "StaticMeshData.h"
#include "MaterialData.h"

AssetManager& AssetManager::Get()
{
	static AssetManager s_instance;

	return s_instance;
}

std::shared_ptr<AssetData> AssetManager::GetAsset(AssetKey key)
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

		m_assets[key] = fbx.GetStaticMeshData();
		m_assets[{ key.path, AssetKind::Material }] = fbx.GetMaterialData();

		assetData = fbx.GetStaticMeshData();

		m_cachedAssets.push(fbx.GetStaticMeshData());
		m_cachedAssets.push(fbx.GetMaterialData());
	}
		break;

	case AssetKind::Material:
	{
		FBXAssetData fbx;
		fbx.Create(FBXAssetKind::Static, key.path);

		m_assets[{ key.path, AssetKind::StaticMesh }] = fbx.GetStaticMeshData();
		m_assets[key] = fbx.GetMaterialData();

		assetData = fbx.GetMaterialData();

		m_cachedAssets.push(fbx.GetStaticMeshData());
		m_cachedAssets.push(fbx.GetMaterialData());
	}
		break;
	}

	return assetData;
}