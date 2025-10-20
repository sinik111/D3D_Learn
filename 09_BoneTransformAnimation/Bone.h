#pragma once

#include <string>
#include <directxtk/SimpleMath.h>

struct BoneInfo
{
	DirectX::SimpleMath::Matrix m_local;
	int m_parentIndex;

	BoneInfo(const float* matrix, int parentIndex) noexcept
		: m_local{ matrix }, m_parentIndex{ parentIndex }
	{

	}
};

class Bone
{
private:
	BoneInfo& m_boneInfo;
	DirectX::SimpleMath::Matrix m_model;
	int m_parentIndex;
};