#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <array>

class AssetData;
class StaticMeshData;
class MaterialData;
class SkeletalMeshData;
class AnimationData;
class SkeletonData;

class AssetManager
{
private:
	// assimp로 fbx로드할 때는 mesh, material, animation등 여러가지를 전부 불러와야해서
	// fbx 로드할 때 임시 공간에 저장해둠
	// 개별 파일로 분리하면 필요없음
	std::array<std::shared_ptr<AssetData>, 10> m_tempAssets;
	size_t m_tempAssetIndex = 0;
	std::unordered_map<std::wstring, std::weak_ptr<StaticMeshData>> m_staticMeshAssets;
	std::unordered_map<std::wstring, std::weak_ptr<MaterialData>> m_materialAssets;
	std::unordered_map<std::wstring, std::weak_ptr<SkeletonData>> m_skeletonAssets;
	std::unordered_map<std::wstring, std::weak_ptr<SkeletalMeshData>> m_skeletalMeshAssets;
	std::unordered_map<std::wstring, std::weak_ptr<AnimationData>> m_animationAssets;

private:
	AssetManager() = default;
	~AssetManager() = default;
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;
	AssetManager(AssetManager&&) = delete;
	AssetManager& operator=(AssetManager&&) = delete;

public:
	static AssetManager& Get();

public:
	std::shared_ptr<StaticMeshData> GetOrCreateStaticMeshAsset(const std::wstring& filePath);
	std::shared_ptr<MaterialData> GetOrCreateMaterialAsset(const std::wstring& filePath);
	std::shared_ptr<SkeletonData> GetOrCreateSkeletonAsset(const std::wstring& filePath);
	std::shared_ptr<SkeletalMeshData> GetOrCreateSkeletalMeshAsset(const std::wstring& filePath);
	std::shared_ptr<AnimationData> GetOrCreateAnimationAsset(const std::wstring& filePath);
};