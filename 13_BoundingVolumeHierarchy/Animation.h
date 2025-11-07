#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include <directxtk/SimpleMath.h>

#include "Skeleton.h"

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

struct aiAnimation;
class SkeletonInfo;

class Animation
{
private:
	using BoneName = std::wstring;
	using BoneAnimIndex = unsigned int;

	std::wstring m_name;
	std::vector<BoneAnimation> m_boneAnimations;
	std::unordered_map<BoneName, BoneAnimIndex> m_animMappingTable;
	float m_duration;

public:
	Animation(const aiAnimation* animation, const SkeletonInfo* skeletonInfo);

public:
	const BoneAnimation* GetBoneAnimationByBoneName(const std::wstring& boneName) const;
	float GetDuration() const;

public:
	void SetupBoneAnimation(std::vector<Bone>& out) const;
};

