#pragma once

#include <vector>
#include <string>

#include "../Common/Vertex.h"

#include "AssetData.h"

struct aiScene;

class StaticMeshData :
    public AssetData
{
private:
    struct StaticMeshSection
    {
        std::wstring name;
        unsigned int materialIndex;
        INT vertexOffset;
        UINT indexOffset;
        UINT indexCount;
    };

private:
    std::vector<CommonVertex3D> m_vertices;
    std::vector<DWORD> m_indices;
    std::vector<StaticMeshSection> m_meshSections;

public:
    void Create(const std::wstring& filePath);
    void Create(const aiScene* scene);

public:
    const std::vector<CommonVertex3D>& GetVertices() const;
    const std::vector<DWORD>& GetIndices() const;
};