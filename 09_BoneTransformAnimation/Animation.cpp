#include "Animation.h"

#include <assimp/anim.h>
#include <cassert>

#include "../Common/Helper.h"

void BoneAnimation::Evaluate(float time, LastKeyIndex& inOutLastKeyIndex, Vector3& outPosition, Quaternion& outRotation, Vector3& outScale) const
{
	size_t positionKeyIndex = inOutLastKeyIndex.position;
	size_t rotationKeyIndex = inOutLastKeyIndex.rotation;
	size_t scaleKeyIndex = inOutLastKeyIndex.scale;

	while (true)
	{
		size_t nextIndex = (positionKeyIndex + 1) % positionKeys.size();
		
		if (positionKeys[nextIndex].time > time)
		{
			outPosition = DirectX::SimpleMath::Vector3::Lerp(
				positionKeys[positionKeyIndex].value,
				positionKeys[nextIndex].value,
				(time - (positionKeys[positionKeyIndex].time)) / (positionKeys[nextIndex].time - positionKeys[positionKeyIndex].time)
			);
			
			inOutLastKeyIndex.position = positionKeyIndex;
			
			break;
		}

		positionKeyIndex = nextIndex;
	}

	while (true)
	{
		size_t nextIndex = (rotationKeyIndex + 1) % rotationKeys.size();

		if (rotationKeys[nextIndex].time > time)
		{
			outRotation = DirectX::SimpleMath::Quaternion::Slerp(
				rotationKeys[rotationKeyIndex].value,
				rotationKeys[nextIndex].value,
				(time - (rotationKeys[rotationKeyIndex].time)) / (rotationKeys[nextIndex].time - rotationKeys[rotationKeyIndex].time)
			);
			
			inOutLastKeyIndex.rotation = rotationKeyIndex;

			break;
		}

		rotationKeyIndex = nextIndex;
	}

	while (true)
	{
		size_t nextIndex = (scaleKeyIndex + 1) % scaleKeys.size();

		if (scaleKeys[nextIndex].time > time)
		{
			outScale = DirectX::SimpleMath::Vector3::Lerp(
				scaleKeys[scaleKeyIndex].value,
				scaleKeys[nextIndex].value,
				(time - (scaleKeys[scaleKeyIndex].time)) / (scaleKeys[nextIndex].time - scaleKeys[scaleKeyIndex].time)
			);

			inOutLastKeyIndex.scale = scaleKeyIndex;

			break;
		}

		scaleKeyIndex = nextIndex;
	}
}

Animation::Animation(const aiAnimation* animation, const SkeletonInfo* skeletonInfo)
	: m_name{ ToWideCharStr(animation->mName.C_Str()) },
	m_duration{ static_cast<float>(animation->mDuration / animation->mTicksPerSecond) },
	m_boneAnimations(animation->mNumChannels)
{
	for (unsigned int i = 0; i < animation->mNumChannels; ++i)
	{
		const aiNodeAnim* anim = animation->mChannels[i];
		const std::wstring boneName{ ToWideCharStr(anim->mNodeName.C_Str()) };

		m_boneAnimations[i].boneIndex = skeletonInfo->GetBoneIndexByBoneName(boneName);
		m_animMappingTable[boneName] = i;

		m_boneAnimations[i].positionKeys.reserve(anim->mNumPositionKeys);
		m_boneAnimations[i].rotationKeys.reserve(anim->mNumRotationKeys);
		m_boneAnimations[i].scaleKeys.reserve(anim->mNumScalingKeys);

		for (unsigned int j = 0; j < anim->mNumPositionKeys; ++j)
		{
			m_boneAnimations[i].positionKeys.emplace_back(static_cast<float>(anim->mPositionKeys[j].mTime / animation->mTicksPerSecond),
				&anim->mPositionKeys[j].mValue.x);
		}
		
		for (unsigned int j = 0; j < anim->mNumRotationKeys; ++j)
		{
			m_boneAnimations[i].rotationKeys.emplace_back(static_cast<float>(anim->mRotationKeys[j].mTime / animation->mTicksPerSecond),
				&anim->mRotationKeys[j].mValue.w);
		}

		for (unsigned int j = 0; j < anim->mNumScalingKeys; ++j)
		{
			m_boneAnimations[i].scaleKeys.emplace_back(static_cast<float>(anim->mScalingKeys[j].mTime / animation->mTicksPerSecond),
				&anim->mScalingKeys[j].mValue.x);
		}
	}
}

const BoneAnimation* Animation::GetBoneAnimationByBoneName(const std::wstring& boneName) const
{
	auto find = m_animMappingTable.find(boneName);

	assert(find != m_animMappingTable.end());

	return &m_boneAnimations[find->second];
}

float Animation::GetDuration() const
{
	return m_duration;
}

void Animation::SetupBoneAnimation(std::vector<Bone>& out) const
{
	for (auto& bone : out)
	{
		auto find = m_animMappingTable.find(bone.name);

		if (find != m_animMappingTable.end())
		{
			bone.boneAnimation = &m_boneAnimations[find->second];
		}
		else
		{
			bone.boneAnimation = nullptr;
		}
	}
}