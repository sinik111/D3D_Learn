#pragma once

#include <vector>
#include <DirectXCollision.h>
#include <directxtk/SimpleMath.h>
#include <cstdint>

struct BVHNode
{
	DirectX::BoundingBox aabb;
	std::uint32_t left;
	std::uint32_t right;
	std::uint32_t firstObject;
	std::uint32_t objectCount;

	bool IsLeaf() const
	{
		return objectCount != 0;
	}
};

class BVH
{
private:
	using Vector3 = DirectX::SimpleMath::Vector3;

private:
	std::vector<BVHNode> m_nodes;
	std::vector<std::uint32_t> m_objectIndices;
	std::vector<DirectX::BoundingBox> m_objectAABBs;

public:
	void Insert(DirectX::BoundingBox aabb);
	void Insert(const std::vector<DirectX::BoundingBox>& aabbs);
	const std::vector<BVHNode>& GetNodes() const;

private:
	std::uint32_t BuildBVH(std::uint32_t begin, std::uint32_t end);
};