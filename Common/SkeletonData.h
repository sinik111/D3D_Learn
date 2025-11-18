#pragma once

#include <string>
#include <directxtk/SimpleMath.h>
#include <unordered_map>
#include <vector>
#include <array>

#include "AssetData.h"

constexpr size_t MAX_BONE_NUM = 128;

using BoneMatrixArray = std::array<DirectX::SimpleMath::Matrix, MAX_BONE_NUM>;

struct BoneInfo
{
	std::wstring name;
	DirectX::SimpleMath::Matrix relative;
	unsigned int index;
	int parentIndex;

	BoneInfo(const std::wstring& name, const DirectX::SimpleMath::Matrix& relative, unsigned int index, int parentIndex)
		: name{ name }, relative{ relative }, index{ index }, parentIndex{ parentIndex }
	{

	}
};

struct LastKeyIndex
{
	size_t position;
	size_t rotation;
	size_t scale;
};

struct BoneAnimation;

struct Bone
{
	std::wstring name;
	DirectX::SimpleMath::Matrix local;
	DirectX::SimpleMath::Matrix model;
	int parentIndex;
	unsigned int index;
	const BoneAnimation* boneAnimation = nullptr;
	LastKeyIndex lastKeyIndex{};

	Bone(const std::wstring& name, int parentIndex, unsigned int index, const DirectX::SimpleMath::Matrix& local)
		: name{ name }, parentIndex{ parentIndex }, index{ index }, local{ local }
	{

	}
};

struct aiScene;

class SkeletonData :
    public AssetData
{
private:
	using BoneIndex = unsigned int;
	using BoneName = std::wstring;
	using MeshName = std::wstring;

	std::vector<BoneInfo> m_bones;
	std::unordered_map<BoneName, BoneIndex> m_boneMappingTable;
	std::unordered_map<MeshName, BoneIndex> m_meshMappingTable;
	BoneMatrixArray m_boneOffsets;

public:
	void Create(const aiScene* scene);

public:
	const std::vector<BoneInfo>& GetBones() const;
	unsigned int GetBoneIndexByBoneName(const std::wstring& boneName) const;
	unsigned int GetBoneIndexByMeshName(const std::wstring& meshName) const;
	BoneInfo* GetBoneInfoByIndex(size_t index);
	const BoneMatrixArray& GetBoneOffsets() const;

	void SetBoneOffset(const DirectX::SimpleMath::Matrix& offset, unsigned int boneIndex);

	void SetupSkeletonInstance(std::vector<Bone>& out) const;
};