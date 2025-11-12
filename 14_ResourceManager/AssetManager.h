#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <queue>

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
	std::queue<std::shared_ptr<AssetData>> m_cachedAssets;
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
	std::shared_ptr<AssetData> GetAsset(AssetKey key);
};