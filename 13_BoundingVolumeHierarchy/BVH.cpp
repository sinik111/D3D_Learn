#include "BVH.h"

#include <algorithm>
#include <numeric>

using DirectX::SimpleMath::Vector3;
using DirectX::BoundingBox;

constexpr std::uint32_t LEAF_LIMITS = 2;

Vector3 CalcAABBMin(const BoundingBox& aabb)
{
	return { aabb.Center.x - aabb.Extents.x, aabb.Center.y - aabb.Extents.y, aabb.Center.z - aabb.Extents.z };
}

Vector3 CalcAABBMax(const BoundingBox& aabb)
{
	return { aabb.Center.x + aabb.Extents.x, aabb.Center.y + aabb.Extents.y, aabb.Center.z + aabb.Extents.z };
}

BoundingBox MakeWithMinMax(const Vector3& min, const Vector3& max)
{
	Vector3 extents{ (max.x - min.x) / 2, (max.y - min.y) / 2, (max.z - min.z) / 2 };
	Vector3 center{ min.x + extents.x, min.y + extents.y, min.z + extents.z };

	return { center, extents };
}

void BVH::Insert(DirectX::BoundingBox aabb)
{
}

void BVH::Insert(const std::vector<DirectX::BoundingBox>& aabbs)
{
	// todo: 이거 뒤에 추가하는 식으로 바꿔야함
	m_objectIndices.resize(aabbs.size());
	std::iota(m_objectIndices.begin(), m_objectIndices.end(), 0);
	m_objectAABBs.insert(m_objectAABBs.end(), aabbs.begin(), aabbs.end());

	BuildBVH(0, static_cast<std::uint32_t>(aabbs.size()));
}

const std::vector<BVHNode>& BVH::GetNodes() const
{
    return m_nodes;
}

std::uint32_t BVH::BuildBVH(std::uint32_t begin, std::uint32_t end)
{
	Vector3 centerMin{ INFINITY, INFINITY, INFINITY };
	Vector3 centerMax{ -INFINITY, -INFINITY, -INFINITY };
	Vector3 min{ INFINITY, INFINITY, INFINITY };
	Vector3 max{ -INFINITY, -INFINITY, -INFINITY };

	for (std::uint32_t i = begin; i < end; ++i)
	{
		const auto& bounds = m_objectAABBs[m_objectIndices[i]];
		centerMin = Vector3::Min(centerMin, bounds.Center);
		centerMax = Vector3::Max(centerMax, bounds.Center);
		min = Vector3::Min(min, CalcAABBMin(bounds));
		max = Vector3::Max(max, CalcAABBMax(bounds));
	}

	if (end - begin <= LEAF_LIMITS)
	{
		m_nodes.push_back({ MakeWithMinMax(min, max), 0, 0, begin, end - begin });

		return static_cast<std::uint32_t>(m_nodes.size() - 1);
	}

	std::uint32_t currentNodeIndex = static_cast<std::uint32_t>(m_nodes.size());
	m_nodes.push_back({ MakeWithMinMax(min, max), 0, 0, 0, 0 });

	float lengthX = centerMax.x - centerMin.x;
	float lengthY = centerMax.y - centerMin.y;
	float lengthZ = centerMax.z - centerMin.z;

	if (lengthX > lengthY && lengthX > lengthZ)
	{
		std::sort(m_objectIndices.begin() + begin, m_objectIndices.begin() + end,
			[this](auto a, auto b)
			{
				return m_objectAABBs[a].Center.x < m_objectAABBs[b].Center.x;
			}
		);
	}
	else if (lengthY > lengthX && lengthY > lengthZ)
	{
		std::sort(m_objectIndices.begin() + begin, m_objectIndices.begin() + end,
			[this](auto a, auto b)
			{
				return m_objectAABBs[a].Center.y < m_objectAABBs[b].Center.y;
			}
		);
	}
	else
	{
		std::sort(m_objectIndices.begin() + begin, m_objectIndices.begin() + end,
			[this](auto a, auto b)
			{
				return m_objectAABBs[a].Center.z < m_objectAABBs[b].Center.z;
			}
		);
	}

	std::uint32_t mid = begin + (end - begin) / 2;

	std::uint32_t leftIndex = BuildBVH(begin, mid);
	std::uint32_t rightIndex = BuildBVH(mid, end);

	m_nodes[currentNodeIndex].left = leftIndex;
	m_nodes[currentNodeIndex].right = rightIndex;

	return currentNodeIndex;
}