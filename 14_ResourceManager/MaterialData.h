#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include <directxtk/SimpleMath.h>

#include "AssetData.h"

struct aiScene;

enum class MaterialKey : unsigned long long
{
    DIFFUSE_TEXTURE     =   1 << 0,
    NORMAL_TEXTURE      =   1 << 1,
    SPECULAR_TEXTURE    =   1 << 2,
    EMISSIVE_TEXTURE    =   1 << 3,
    OPACITY_TEXTURE     =   1 << 4,
    DIFFUSE_COLOR       =   1 << 5,
    AMBIENT_COLOR       =   1 << 6,
    SPECULAR_COLOR      =   1 << 7,
    EMISSIVE_COLOR      =   1 << 8,
    SHININESS_FACTOR    =   1 << 9,
    OPACITY_FACTOR      =   1 << 10,
};

class MaterialData :
    public AssetData
{
private:
    struct Material
    {
        std::unordered_map<MaterialKey, std::wstring> texturePaths;
        std::unordered_map<MaterialKey, DirectX::SimpleMath::Vector4> vectorValues;
        std::unordered_map<MaterialKey, float> scalarValues;
        unsigned long long materialFlags = 0;
    };

private:
    std::vector<Material> m_materials;

public:
    void Create(const aiScene* scene);
};