#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <directxtk/SimpleMath.h>
#include <memory>

#include "SkeletonData.h"
#include "AssetData.h"

struct PositionKey
{
	float time;
	DirectX::SimpleMath::Vector3 value;

	PositionKey(float time, const float* f)
		: time{ time }, value{ f }
	{

	}
};

struct RotationKey
{
	float time;
	DirectX::SimpleMath::Quaternion value;

	RotationKey(float time, const float* f)
		: time{ time }, value{ f[1], f[2], f[3], f[0] }
	{

	}
};

struct ScaleKey
{
	float time;
	DirectX::SimpleMath::Vector3 value;

	ScaleKey(float time, const float* f)
		: time{ time }, value{ f }
	{

	}
};

struct LastKeyIndex;

struct BoneAnimation
{
	using Vector3 = DirectX::SimpleMath::Vector3;
	using Quaternion = DirectX::SimpleMath::Quaternion;

	std::vector<PositionKey> positionKeys;
	std::vector<RotationKey> rotationKeys;
	std::vector<ScaleKey> scaleKeys;
	unsigned int boneIndex;

	void Evaluate(float time, LastKeyIndex& inOutLastKeyIndex, Vector3& outPosition, Quaternion& outRotation, Vector3& outScale) const;
};

struct Animation
{
	using BoneName = std::wstring;
	using BoneAnimIndex = unsigned int;

	std::wstring name;
	std::vector<BoneAnimation> boneAnimations;
	std::unordered_map<BoneName, BoneAnimIndex> animMappingTable;
	float duration;

	void SetupBoneAnimation(std::vector<Bone>& out) const;
};

struct aiScene;
class SkeletonData;

class AnimationData :
    public AssetData
{
private:
	std::vector<Animation> m_animations;

public:
	void Create(const aiScene* scene, const std::shared_ptr<SkeletonData>& skeletonData);

public:
	const std::vector<Animation>& GetAnimations() const;
};