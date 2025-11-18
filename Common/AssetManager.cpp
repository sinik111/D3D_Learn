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

	FBXAssetData fbx;
	fbx.Create(FBXAssetKind::Skeletal, filePath);

	const auto& meshData = fbx.GetSkeletalMeshData();
	const auto& materialData = fbx.GetMaterialData();
	const auto& animationData = fbx.GetAnimationData();
	const auto& skeletonData = fbx.GetSkeletonData();

	m_skeletalMeshAssets[filePath] = meshData;
	m_materialAssets[filePath] = materialData;
	m_animationAssets[filePath] = animationData;
	m_skeletonAssets[filePath] = skeletonData;

	m_tempAssets[m_tempAssetIndex++ % 10] = meshData;
	m_tempAssets[m_tempAssetIndex++ % 10] = materialData;
	m_tempAssets[m_tempAssetIndex++ % 10] = animationData;
	m_tempAssets[m_tempAssetIndex++ % 10] = skeletonData;

	return meshData;
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

	FBXAssetData fbx;
	fbx.Create(FBXAssetKind::Skeletal, filePath);

	const auto& meshData = fbx.GetSkeletalMeshData();
	const auto& materialData = fbx.GetMaterialData();
	const auto& animationData = fbx.GetAnimationData();
	const auto& skeletonData = fbx.GetSkeletonData();

	m_skeletalMeshAssets[filePath] = meshData;
	m_materialAssets[filePath] = materialData;
	m_animationAssets[filePath] = animationData;
	m_skeletonAssets[filePath] = skeletonData;

	m_tempAssets[m_tempAssetIndex++ % 10] = meshData;
	m_tempAssets[m_tempAssetIndex++ % 10] = materialData;
	m_tempAssets[m_tempAssetIndex++ % 10] = animationData;
	m_tempAssets[m_tempAssetIndex++ % 10] = skeletonData;

	return animationData;
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

	FBXAssetData fbx;
	fbx.Create(FBXAssetKind::Skeletal, filePath);

	const auto& meshData = fbx.GetSkeletalMeshData();
	const auto& materialData = fbx.GetMaterialData();
	const auto& animationData = fbx.GetAnimationData();
	const auto& skeletonData = fbx.GetSkeletonData();

	m_skeletalMeshAssets[filePath] = meshData;
	m_materialAssets[filePath] = materialData;
	m_animationAssets[filePath] = animationData;
	m_skeletonAssets[filePath] = skeletonData;

	m_tempAssets[m_tempAssetIndex++ % 10] = meshData;
	m_tempAssets[m_tempAssetIndex++ % 10] = materialData;
	m_tempAssets[m_tempAssetIndex++ % 10] = animationData;
	m_tempAssets[m_tempAssetIndex++ % 10] = skeletonData;

	return skeletonData;
}
