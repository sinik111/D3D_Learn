#include "AssetManager.h"

#include "FBXAssetData.h"
#include "StaticMeshData.h"
#include "MaterialData.h"
#include "SkeletalMeshData.h"
#include "SkeletonData.h"
#include "AnimationData.h"

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

	std::shared_ptr<FBXAssetData> fbx = std::make_shared<FBXAssetData>();
	fbx->Create(FBXAssetKind::Static, filePath);

	m_staticMeshAssets[filePath] = fbx->GetStaticMeshData();
	m_materialAssets[filePath] = fbx->GetMaterialData();

	m_tempAssets[m_tempAssetIndex++ % MAX_TEMP_ASSET] = fbx;

	return fbx->GetStaticMeshData();
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

	std::shared_ptr<FBXAssetData> fbx = std::make_shared<FBXAssetData>();
	fbx->Create(FBXAssetKind::Static, filePath);

	m_staticMeshAssets[filePath] = fbx->GetStaticMeshData();
	m_materialAssets[filePath] = fbx->GetMaterialData();

	m_tempAssets[m_tempAssetIndex++ % MAX_TEMP_ASSET] = fbx;

	return fbx->GetMaterialData();
}

std::shared_ptr<SkeletalMeshData> AssetManager::GetOrCreateSkeletalMeshAsset(const std::wstring& filePath)
{
	auto find = m_skeletalMeshAssets.find(filePath);
	if (find != m_skeletalMeshAssets.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<FBXAssetData> fbx = std::make_shared<FBXAssetData>();
	fbx->Create(FBXAssetKind::Skeletal, filePath);

	m_skeletalMeshAssets[filePath] = fbx->GetSkeletalMeshData();
	m_materialAssets[filePath] = fbx->GetMaterialData();
	m_animationAssets[filePath] = fbx->GetAnimationData();
	m_skeletonAssets[filePath] = fbx->GetSkeletonData();

	m_tempAssets[m_tempAssetIndex++ % MAX_TEMP_ASSET] = fbx;

	return fbx->GetSkeletalMeshData();
}

std::shared_ptr<AnimationData> AssetManager::GetOrCreateAnimationAsset(const std::wstring& filePath)
{
	auto find = m_animationAssets.find(filePath);
	if (find != m_animationAssets.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<FBXAssetData> fbx = std::make_shared<FBXAssetData>();
	fbx->Create(FBXAssetKind::Skeletal, filePath);

	m_skeletalMeshAssets[filePath] = fbx->GetSkeletalMeshData();
	m_materialAssets[filePath] = fbx->GetMaterialData();
	m_animationAssets[filePath] = fbx->GetAnimationData();
	m_skeletonAssets[filePath] = fbx->GetSkeletonData();

	m_tempAssets[m_tempAssetIndex++ % MAX_TEMP_ASSET] = fbx;

	return fbx->GetAnimationData();
}

std::shared_ptr<SkeletonData> AssetManager::GetOrCreateSkeletonAsset(const std::wstring& filePath)
{
	auto find = m_skeletonAssets.find(filePath);
	if (find != m_skeletonAssets.end())
	{
		if (!find->second.expired())
		{
			return find->second.lock();
		}
	}

	std::shared_ptr<FBXAssetData> fbx = std::make_shared<FBXAssetData>();
	fbx->Create(FBXAssetKind::Skeletal, filePath);

	m_skeletalMeshAssets[filePath] = fbx->GetSkeletalMeshData();
	m_materialAssets[filePath] = fbx->GetMaterialData();
	m_animationAssets[filePath] = fbx->GetAnimationData();
	m_skeletonAssets[filePath] = fbx->GetSkeletonData();

	m_tempAssets[m_tempAssetIndex++ % MAX_TEMP_ASSET] = fbx;

	return fbx->GetSkeletonData();
}
