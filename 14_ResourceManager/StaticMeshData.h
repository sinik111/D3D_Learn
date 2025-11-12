#pragma once

#include <vector>
#include <string>

#include "../Common/Vertex.h"

#include "AssetData.h"

struct aiScene;

enum class IndexFormat
{
    R16_UINT,
    R32_UINT
};

class StaticMeshData :
    public AssetData
{
private:
    struct StaticMeshSection
    {
        std::wstring name;
        std::vector<CommonVertex3D> vertices;
        std::vector<WORD> indices16;
        std::vector<DWORD> indices32;
        IndexFormat indexFormat;
        unsigned int materialIndex;
        UINT indexCount;
    };

private:
    std::vector<StaticMeshSection> m_meshSections;

public:
    void Create(const std::wstring& filePath);
    void Create(const aiScene* scene);
};