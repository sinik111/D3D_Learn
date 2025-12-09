#include "BVH.h"

#include <algorithm>
#include <numeric>
#include <cfloat>

using DirectX::SimpleMath::Vector3;
using DirectX::BoundingBox;

constexpr std::uint32_t LEAF_LIMITS = 1;

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

std::uint32_t BVH::Insert(const DirectX::BoundingBox& aabb)
{
	m_objectAABBs.push_back(aabb);
	std::uint32_t objectIndex = static_cast<std::uint32_t>(m_objectAABBs.size() - 1);

	m_objectIndices.push_back(objectIndex);
	m_objectToLeafIndices.push_back(0);

	std::uint32_t nodeIndex = AllocateNode();
	m_nodes[nodeIndex].aabb = aabb;
	m_nodes[nodeIndex].firstObject = objectIndex;
	m_nodes[nodeIndex].objectCount = 1;

	m_objectToLeafIndices[objectIndex] = nodeIndex;

	InsertLeaf(nodeIndex);

	return objectIndex;
}

std::vector<std::uint32_t> BVH::Insert(const std::vector<DirectX::BoundingBox>& aabbs)
{
	std::vector<std::uint32_t> resultIDs;

	if (aabbs.empty())
	{
		return resultIDs;
	}

	resultIDs.reserve(aabbs.size());

	size_t currentSize = m_objectAABBs.size();
	size_t newSize = aabbs.size();

	m_objectAABBs.insert(m_objectAABBs.end(), aabbs.begin(), aabbs.end());
	m_objectIndices.resize(currentSize + newSize);
	m_objectToLeafIndices.resize(currentSize + newSize);

	for (size_t i = 0; i < newSize; ++i)
	{
		std::uint32_t assignedID = static_cast<std::uint32_t>(currentSize + i);

		m_objectIndices[currentSize + i] = assignedID;

		resultIDs.push_back(assignedID);
	}

	FullyRebuild();

	return resultIDs;
}

void BVH::ChangeAABB(std::uint32_t index, const DirectX::BoundingBox& newAABB)
{
	m_objectAABBs[index] = newAABB;
	m_changedObjectIndices.push_back(index);
}

void BVH::FullyRebuild(bool useSAH)
{
	m_nodes.clear();
	m_freeNodeIndices.clear();

	if (!m_objectAABBs.empty())
	{
		if (useSAH)
		{
			m_rootIndex = BuildBVHWithSAH(0, static_cast<std::uint32_t>(m_objectAABBs.size()));
		}
		else
		{
			m_rootIndex = BuildBVH(0, static_cast<std::uint32_t>(m_objectAABBs.size()));
		}
	}

	m_changedObjectIndices.clear();
}

void BVH::Refit()
{
	for (auto objIndex : m_changedObjectIndices)
	{
		std::uint32_t leafIndex = m_objectToLeafIndices[objIndex];
		if (leafIndex == -1)
		{
			continue;
		}

		auto& leafNode = m_nodes[leafIndex];

		Vector3 min{ FLT_MAX, FLT_MAX, FLT_MAX };
		Vector3 max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

		for (std::uint32_t i = 0; i < leafNode.objectCount; ++i)
		{
			std::uint32_t currentObjIdx = m_objectIndices[leafNode.firstObject + i];
			const auto& bounds = m_objectAABBs[currentObjIdx];

			min = Vector3::Min(min, CalcAABBMin(bounds));
			max = Vector3::Max(max, CalcAABBMax(bounds));
		}

		leafNode.aabb = MakeWithMinMax(min, max);

		std::int32_t currentNodeIndex = leafNode.parent;

		while (currentNodeIndex != -1)
		{
			auto& node = m_nodes[currentNodeIndex];

			const auto& leftChild = m_nodes[node.left];
			const auto& rightChild = m_nodes[node.right];
			BoundingBox::CreateMerged(node.aabb, leftChild.aabb, rightChild.aabb);

			currentNodeIndex = node.parent;
		}
	}

	m_changedObjectIndices.clear();
}

void BVH::RefitWithRotation()
{
	for (auto objIndex : m_changedObjectIndices)
	{
		std::uint32_t leafIndex = m_objectToLeafIndices[objIndex];
		if (leafIndex == -1)
		{
			continue;
		}

		auto& leafNode = m_nodes[leafIndex];

		Vector3 min{ FLT_MAX, FLT_MAX, FLT_MAX };
		Vector3 max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

		for (std::uint32_t i = 0; i < leafNode.objectCount; ++i)
		{
			std::uint32_t currentObjIdx = m_objectIndices[leafNode.firstObject + i];
			const auto& bounds = m_objectAABBs[currentObjIdx];

			min = Vector3::Min(min, CalcAABBMin(bounds));
			max = Vector3::Max(max, CalcAABBMax(bounds));
		}

		leafNode.aabb = MakeWithMinMax(min, max);

		std::int32_t currentNodeIndex = leafNode.parent;

		while (currentNodeIndex != -1)
		{
			auto& node = m_nodes[currentNodeIndex];

			const auto& leftChild = m_nodes[node.left];
			const auto& rightChild = m_nodes[node.right];
			BoundingBox::CreateMerged(node.aabb, leftChild.aabb, rightChild.aabb);

			TryRotation(currentNodeIndex);

			const auto& newLeftChild = m_nodes[node.left];
			const auto& newRightChild = m_nodes[node.right];
			BoundingBox::CreateMerged(node.aabb, newLeftChild.aabb, newRightChild.aabb);

			currentNodeIndex = node.parent;
		}
	}

	m_changedObjectIndices.clear();
}

void BVH::OptimizeObject(std::uint32_t index)
{
	std::int32_t leafIndex = m_objectToLeafIndices[index];
	if (leafIndex == -1)
	{
		return;
	}

	m_nodes[leafIndex].aabb = m_objectAABBs[index];

	RemoveLeaf(leafIndex);

	InsertLeaf(leafIndex);
}

const std::vector<BVHNode>& BVH::GetNodes() const
{
    return m_nodes;
}

std::uint32_t BVH::BuildBVH(std::uint32_t begin, std::uint32_t end)
{
	Vector3 centerMin{ FLT_MAX, FLT_MAX, FLT_MAX };
	Vector3 centerMax{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
	Vector3 min{ FLT_MAX, FLT_MAX, FLT_MAX };
	Vector3 max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

	for (std::uint32_t i = begin; i < end; ++i)
	{
		const auto& bounds = m_objectAABBs[m_objectIndices[i]];
		centerMin = Vector3::Min(centerMin, bounds.Center);
		centerMax = Vector3::Max(centerMax, bounds.Center);
		min = Vector3::Min(min, CalcAABBMin(bounds));
		max = Vector3::Max(max, CalcAABBMax(bounds));
	}

	std::uint32_t myIndex = AllocateNode();
	m_nodes[myIndex].aabb = MakeWithMinMax(min, max);

	if (end - begin <= LEAF_LIMITS)
	{
		m_nodes[myIndex].firstObject = begin;
		m_nodes[myIndex].objectCount = end - begin;

		for (std::uint32_t i = begin; i < end; ++i)
		{
			m_objectToLeafIndices[m_objectIndices[i]] = myIndex;
		}

		return myIndex;
	}

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

	m_nodes[myIndex].left = leftIndex;
	m_nodes[myIndex].right = rightIndex;

	m_nodes[leftIndex].parent = myIndex;
	m_nodes[rightIndex].parent = myIndex;

	return myIndex;
}

std::uint32_t BVH::BuildBVHWithSAH(std::uint32_t begin, std::uint32_t end)
{
	Vector3 centerMin{ FLT_MAX, FLT_MAX, FLT_MAX };
	Vector3 centerMax{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
	Vector3 min{ FLT_MAX, FLT_MAX, FLT_MAX };
	Vector3 max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

	for (std::uint32_t i = begin; i < end; ++i)
	{
		const auto& bounds = m_objectAABBs[m_objectIndices[i]];
		centerMin = Vector3::Min(centerMin, bounds.Center);
		centerMax = Vector3::Max(centerMax, bounds.Center);
		min = Vector3::Min(min, CalcAABBMin(bounds));
		max = Vector3::Max(max, CalcAABBMax(bounds));
	}

	std::uint32_t myIndex = AllocateNode();
	m_nodes[myIndex].aabb = MakeWithMinMax(min, max);

	if (end - begin <= LEAF_LIMITS)
	{
		m_nodes[myIndex].firstObject = begin;
		m_nodes[myIndex].objectCount = end - begin;

		for (std::uint32_t i = begin; i < end; ++i)
		{
			m_objectToLeafIndices[m_objectIndices[i]] = myIndex;
		}

		return myIndex;
	}

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

	std::uint32_t splitIndex = begin + (end - begin) / 2;
	float bestCost = FLT_MAX;

	std::vector<float> leftAreas(end - begin);
	BoundingBox leftBox = m_objectAABBs[m_objectIndices[begin]];
	for (std::uint32_t i = 0; i < end - begin - 1; ++i)
	{
		DirectX::BoundingBox::CreateMerged(leftBox, leftBox, m_objectAABBs[m_objectIndices[begin + i]]);
		leftAreas[i] = CalculateSurfaceArea(leftBox);
	}

	BoundingBox rightBox = m_objectAABBs[m_objectIndices[end - 1]];

	for (std::int32_t i = (end - begin) - 2; i >= 0; --i)
	{
		DirectX::BoundingBox::CreateMerged(rightBox, rightBox, m_objectAABBs[m_objectIndices[begin + i + 1]]);
		float rightArea = CalculateSurfaceArea(rightBox);
		float leftArea = leftAreas[i];

		float countLeft = (float)(i + 1);
		float countRight = (float)((end - begin) - 1 - i);

		float cost = leftArea * countLeft + rightArea * countRight;

		if (cost < bestCost)
		{
			bestCost = cost;
			splitIndex = begin + i + 1;
		}
	}

	std::uint32_t mid = splitIndex;

	if (mid == begin || mid == end)
	{
		mid = begin + (end - begin) / 2;
	}

	std::uint32_t leftIndex = BuildBVH(begin, mid);
	std::uint32_t rightIndex = BuildBVH(mid, end);

	m_nodes[myIndex].left = leftIndex;
	m_nodes[myIndex].right = rightIndex;

	m_nodes[leftIndex].parent = myIndex;
	m_nodes[rightIndex].parent = myIndex;

	return myIndex;
}

float BVH::CalculateSurfaceArea(const DirectX::BoundingBox& box) const
{
	return 2.0f * (box.Extents.x * box.Extents.y + box.Extents.y * box.Extents.z + box.Extents.z * box.Extents.x);
}

void BVH::TryRotation(std::int32_t nodeIndex)
{
	if (nodeIndex == -1)
	{
		return;
	}

	BVHNode& parent = m_nodes[nodeIndex];
	if (parent.IsLeaf())
	{
		return;
	}

	std::uint32_t left = parent.left;
	std::uint32_t right = parent.right;

	if (m_nodes[left].IsLeaf() && m_nodes[right].IsLeaf())
	{
		return;
	}

	float currentCost = CalculateSurfaceArea(m_nodes[left].aabb) + CalculateSurfaceArea(m_nodes[right].aabb);
	float bestCost = currentCost;
	int bestCase = 0;

	auto CheckSwap = [&](std::uint32_t child, std::uint32_t grandChild1, std::uint32_t grandChild2, int caseIndex)
		{
			DirectX::BoundingBox merged;
			DirectX::BoundingBox::CreateMerged(merged, m_nodes[child].aabb, m_nodes[grandChild2].aabb);
			float cost = CalculateSurfaceArea(m_nodes[grandChild1].aabb) + CalculateSurfaceArea(merged);

			if (cost < bestCost)
			{
				bestCost = cost;
				bestCase = caseIndex;
			}
		};

	// Case 1: L <-> RL, Case 2: L <-> RR
	if (!m_nodes[right].IsLeaf())
	{
		CheckSwap(left, m_nodes[right].left, m_nodes[right].right, 1);
		CheckSwap(left, m_nodes[right].right, m_nodes[right].left, 2);
	}

	// Case 3: R <-> LL, Case 4: R <-> LR
	if (!m_nodes[left].IsLeaf())
	{
		CheckSwap(right, m_nodes[left].left, m_nodes[left].right, 3);
		CheckSwap(right, m_nodes[left].right, m_nodes[left].left, 4);
	}

	if (bestCase != 0)
	{
		std::uint32_t* parentChildPtr = &parent.left;
		std::uint32_t* otherPtr = &parent.right;

		std::uint32_t uncle = left;
		std::uint32_t sibling = right;

		// 대칭 상황이면 포인터 반대로
		if (bestCase >= 3)
		{
			std::swap(parentChildPtr, otherPtr);
			std::swap(uncle, sibling);
		}

		BVHNode& siblingNode = m_nodes[sibling];

		// 교체 대상 조카 식별
		bool isLeftNephew = (bestCase == 1 || bestCase == 3);
		std::uint32_t nephew = isLeftNephew ? siblingNode.left : siblingNode.right;

		// 삼촌 강등
		if (isLeftNephew)
		{
			siblingNode.left = uncle;
		}
		else
		{
			siblingNode.right = uncle;
		}

		m_nodes[uncle].parent = sibling;

		// 조카 승진
		*parentChildPtr = nephew;
		m_nodes[nephew].parent = nodeIndex;

		// AABB 갱신
		DirectX::BoundingBox::CreateMerged(siblingNode.aabb, m_nodes[siblingNode.left].aabb, m_nodes[siblingNode.right].aabb);
		DirectX::BoundingBox::CreateMerged(parent.aabb, m_nodes[parent.left].aabb, m_nodes[parent.right].aabb);
	}
}

void BVH::RemoveLeaf(std::int32_t leafNodeIndex)
{
	if (leafNodeIndex == -1)
	{
		return;
	}

	std::int32_t parentIndex = m_nodes[leafNodeIndex].parent;
	if (parentIndex == -1)
	{
		return;
	}

	std::int32_t grandParentIndex = m_nodes[parentIndex].parent;
	std::int32_t siblingIndex = (m_nodes[parentIndex].left == leafNodeIndex) ? m_nodes[parentIndex].right : m_nodes[parentIndex].left;

	m_nodes[siblingIndex].parent = grandParentIndex;

	if (grandParentIndex != -1)
	{
		if (m_nodes[grandParentIndex].left == parentIndex)
		{
			m_nodes[grandParentIndex].left = siblingIndex;
		}
		else
		{
			m_nodes[grandParentIndex].right = siblingIndex;
		}
	}
	else
	{
		m_rootIndex = siblingIndex;
	}

	m_freeNodeIndices.push_back(parentIndex);

	std::int32_t curr = grandParentIndex;
	while (curr != -1)
	{
		auto& node = m_nodes[curr];
		DirectX::BoundingBox::CreateMerged(node.aabb, m_nodes[node.left].aabb, m_nodes[node.right].aabb);
		curr = node.parent;
	}
}

void BVH::InsertLeaf(std::uint32_t leafNodeIndex)
{
	DirectX::BoundingBox leafBox = m_nodes[leafNodeIndex].aabb;
	float leafArea = CalculateSurfaceArea(leafBox);

	std::int32_t siblingIndex = m_rootIndex;

	while (!m_nodes[siblingIndex].IsLeaf())
	{
		float area = CalculateSurfaceArea(m_nodes[siblingIndex].aabb);

		DirectX::BoundingBox merged;
		DirectX::BoundingBox::CreateMerged(merged, m_nodes[siblingIndex].aabb, leafBox);
		float combinedArea = CalculateSurfaceArea(merged);

		// 비용 계산: 현재 노드에 병합 vs 자식으로 내려가기
		float costDirectMerge = combinedArea;
		float inheritanceCost = 2.0f * (combinedArea - area); // 휴리스틱

		float costLeft, costRight;

		// Left 자식으로 갈 때 비용
		BoundingBox mergedLeft;
		BoundingBox::CreateMerged(mergedLeft, m_nodes[m_nodes[siblingIndex].left].aabb, leafBox);
		float pushLeftCost = CalculateSurfaceArea(mergedLeft) - CalculateSurfaceArea(m_nodes[m_nodes[siblingIndex].left].aabb);
		costLeft = pushLeftCost + inheritanceCost;

		// Right 자식으로 갈 때 비용
		BoundingBox mergedRight;
		BoundingBox::CreateMerged(mergedRight, m_nodes[m_nodes[siblingIndex].right].aabb, leafBox);
		float pushRightCost = CalculateSurfaceArea(mergedRight) - CalculateSurfaceArea(m_nodes[m_nodes[siblingIndex].right].aabb);
		costRight = pushRightCost + inheritanceCost;

		// 현재 노드에 병합하는 게 더 싸면 멈춤
		if (costDirectMerge < costLeft && costDirectMerge < costRight)
		{
			break;
		}

		// 더 싼 쪽으로 이동
		siblingIndex = (costLeft < costRight) ? m_nodes[siblingIndex].left : m_nodes[siblingIndex].right;
	}

	// 새로운 부모 노드 생성
	std::int32_t oldParentIndex = m_nodes[siblingIndex].parent;
	std::int32_t newParentIndex = AllocateNode();

	// 연결 관계 설정
	m_nodes[newParentIndex].parent = oldParentIndex;
	m_nodes[newParentIndex].left = siblingIndex;
	m_nodes[newParentIndex].right = leafNodeIndex;

	// AABB 병합
	BoundingBox::CreateMerged(m_nodes[newParentIndex].aabb, m_nodes[siblingIndex].aabb, leafBox);

	m_nodes[siblingIndex].parent = newParentIndex;
	m_nodes[leafNodeIndex].parent = newParentIndex;

	// 기존 부모(또는 루트)에 새 부모 연결
	if (oldParentIndex != -1)
	{
		if (m_nodes[oldParentIndex].left == siblingIndex)
		{
			m_nodes[oldParentIndex].left = newParentIndex;
		}
		else
		{
			m_nodes[oldParentIndex].right = newParentIndex;
		}
	}
	else
	{
		// 루트가 변경된 경우 처리 (newParentIdx가 새로운 루트가 됨)
		m_rootIndex = newParentIndex;
	}

	// 루트까지 Refit
	std::int32_t curr = oldParentIndex;
	while (curr != -1)
	{
		auto& node = m_nodes[curr];
		BoundingBox::CreateMerged(node.aabb, m_nodes[node.left].aabb, m_nodes[node.right].aabb);
		curr = node.parent;
	}
}

std::uint32_t BVH::AllocateNode()
{
	if (!m_freeNodeIndices.empty())
	{
		std::uint32_t index = m_freeNodeIndices.back();
		m_freeNodeIndices.pop_back();
		return index;
	}

	m_nodes.push_back({});
	return static_cast<std::uint32_t>(m_nodes.size() - 1);
}
