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
    void Create(const aiScene* scene);
    //void Create(const std::wstring& filePath);
    // todo:
    // 나중에 데이터 기반 키로 변경 시 filePath가 아닌 content 또는 guid 키로 생성하게 바꾸기
    // 지금은 assimp aiScene 안에 mesh, material, skeleton, animation 등
    // 데이터가 다 들어있어서 상위 함수(CreateStaticMesh)에서 하위 함수들
    // (CreateStaticMeshData, CreateMaterialData, CreateAnimationData 등등)을
    // 호출하는 방식으로 구현하고 해당 포인터들을 하나의 구조체에 담아서 return하는 방식으로 함.
};