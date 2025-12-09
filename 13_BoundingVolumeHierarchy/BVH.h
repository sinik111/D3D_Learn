#pragma once

#include <vector>
#include <DirectXCollision.h>
#include <directxtk/SimpleMath.h>
#include <cstdint>

struct BVHNode
{
	DirectX::BoundingBox aabb;
	std::int32_t parent = -1;
	std::uint32_t left = 0;
	std::uint32_t right = 0;
	std::uint32_t firstObject = 0;
	std::uint32_t objectCount = 0;

	bool IsLeaf() const
	{
		return objectCount != 0;
	}

	bool IsRoot() const
	{
		return parent == -1;
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
	std::vector<std::uint32_t> m_changedObjectIndices;
	std::vector<std::uint32_t> m_objectToLeafIndices;
	std::vector<std::uint32_t> m_freeNodeIndices;
	std::uint32_t m_rootIndex = 0;

public:
	std::uint32_t Insert(const DirectX::BoundingBox& aabb);
	std::vector<std::uint32_t> Insert(const std::vector<DirectX::BoundingBox>& aabbs);
	void ChangeAABB(std::uint32_t index, const DirectX::BoundingBox& newAABB);
	void FullyRebuild(bool useSAH = true);
	void Refit();
	void RefitWithRotation();
	void OptimizeObject(std::uint32_t index);
	const std::vector<BVHNode>& GetNodes() const;

private:
	std::uint32_t BuildBVH(std::uint32_t begin, std::uint32_t end);
	std::uint32_t BuildBVHWithSAH(std::uint32_t begin, std::uint32_t end);
	float CalculateSurfaceArea(const DirectX::BoundingBox& box) const;

	void TryRotation(std::int32_t nodeIndex);

	void RemoveLeaf(std::int32_t leafNodeIndex);
	void InsertLeaf(std::uint32_t leafNodeIndex);
	std::uint32_t AllocateNode();
};