#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <array>

class AssetData;

enum class AssetKind
{
	StaticMesh, Material
};

struct AssetKey
{
	std::wstring path;
	AssetKind kind;

	bool operator==(const AssetKey& other) const
	{
		return path == other.path && kind == other.kind;
	}
};

namespace std
{
    template <>
    struct hash<AssetKey>
    {
        size_t HashCombine(size_t seed, size_t hashVal) const
        {
            return seed ^ (hashVal + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        }

        size_t operator()(const AssetKey& key) const noexcept
        {
            size_t pathHash = std::hash<std::wstring>{}(key.path);
            size_t kindHash = std::hash<AssetKind>{}(key.kind);

            return HashCombine(pathHash, kindHash);
        }
    };
}

class AssetManager
{
private:
	// assimp로 fbx로드할 때는 mesh, material, animation등 여러가지를 전부 불러와야해서
	// 최초 fbx 로드할 때 임시 공간에 저장해둠
	// 개별 파일로 분리하면 필요없음
	std::array<std::shared_ptr<AssetData>, 10> m_tempAssets;
	size_t m_tempAssetIndex = 0;
	std::unordered_map<AssetKey, std::weak_ptr<AssetData>> m_assets;

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
	std::shared_ptr<AssetData> GetOrCreateAsset(AssetKey key);
};