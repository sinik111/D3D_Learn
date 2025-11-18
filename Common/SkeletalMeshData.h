#pragma once

#include <vector>
#include <string>
#include <memory>

#include "../Common/Vertex.h"

#include "AssetData.h"

struct aiScene;
class SkeletonData;

struct SkeletalMeshSection
{
    std::wstring name;
    unsigned int m_boneReference = 0;
    unsigned int materialIndex;
    INT vertexOffset;
    UINT indexOffset;
    UINT indexCount;
};

class SkeletalMeshData :
    public AssetData
{
private:
    std::vector<BoneWeightVertex3D> m_boneWeightVertices;
    std::vector<CommonVertex3D> m_vertices;
    std::vector<DWORD> m_indices;
    std::vector<SkeletalMeshSection> m_meshSections;
    bool m_isRigid = false;

public:
    void Create(const aiScene* scene, const std::shared_ptr<SkeletonData>& skeletonData, bool isRigid);

public:
    const std::vector<BoneWeightVertex3D>& GetBoneWeightVertices() const;
    const std::vector<CommonVertex3D>& GetVertices() const;
    const std::vector<DWORD>& GetIndices() const;
    const std::vector<SkeletalMeshSection>& GetMeshSections() const;
    bool IsRigid() const;
};