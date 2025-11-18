#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include <directxtk/SimpleMath.h>

#include "AssetData.h"

struct aiScene;

enum class MaterialKey : unsigned long long
{
    DIFFUSE_TEXTURE     = 1ULL << 0,
    NORMAL_TEXTURE      = 1ULL << 1,
    SPECULAR_TEXTURE    = 1ULL << 2,
    EMISSIVE_TEXTURE    = 1ULL << 3,
    OPACITY_TEXTURE     = 1ULL << 4,
    DIFFUSE_COLOR       = 1ULL << 5,
    AMBIENT_COLOR       = 1ULL << 6,
    SPECULAR_COLOR      = 1ULL << 7,
    EMISSIVE_COLOR      = 1ULL << 8,
    SHININESS_FACTOR    = 1ULL << 9,
    OPACITY_FACTOR      = 1ULL << 10,
};

struct Material
{
    std::unordered_map<MaterialKey, std::wstring> texturePaths;
    std::unordered_map<MaterialKey, DirectX::SimpleMath::Vector4> vectorValues;
    std::unordered_map<MaterialKey, float> scalarValues;
    unsigned long long materialFlags = 0;
};

class MaterialData :
    public AssetData
{
private:
    std::vector<Material> m_materials;

public:
    void Create(const std::wstring& filePath);
    void Create(const aiScene* scene);

public:
    const std::vector<Material>& GetMaterials() const;
};