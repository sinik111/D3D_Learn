#include "Skeleton.h"

#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <queue>
#include <cassert>

#include "../Common/Helper.h"

static size_t GetNodeCount(const aiNode* node)
{
	size_t count = 1;

	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		count += GetNodeCount(node->mChildren[i]);
	}

	return count;
}

SkeletonInfo::SkeletonInfo(const aiScene* scene)
{
	// todo: Mesh root 노드라고 간주함 (pelvis), 실제 스켈레톤 root 찾는 로직 필요
	const aiNode* root = scene->mRootNode->mChildren[0];

	const size_t nodeCount = GetNodeCount(root);
	m_bones.reserve(nodeCount);

	std::queue<std::pair<const aiNode*, int>> nodeQueue; // 상위 노드 순으로 트리 만들기 위한 큐
	nodeQueue.push({ root, -1 });

	while (!nodeQueue.empty())
	{
		const auto& [node, parentIndex] = nodeQueue.front();
		const std::wstring name{ ToWideCharStr(node->mName.C_Str()) };
		const BoneIndex index = static_cast<unsigned int>(m_bones.size());

		m_bones.emplace_back(name, DirectX::SimpleMath::Matrix(&node->mTransformation.a1).Transpose(), index, parentIndex);
		m_boneMappingTable[name] = index;

		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
		{
			const std::wstring meshName{ ToWideCharStr(scene->mMeshes[node->mMeshes[i]]->mName.C_Str()) };
			
			m_meshMappingTable[meshName] = index;
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			nodeQueue.push({ node->mChildren[i] , index });
		}

		nodeQueue.pop();
	}
}

const std::vector<BoneInfo>& SkeletonInfo::GetBones() const
{
	return m_bones;
}

unsigned int SkeletonInfo::GetBoneIndexByBoneName(const std::wstring& boneName) const
{
	auto find = m_boneMappingTable.find(boneName);

	assert(find != m_boneMappingTable.end());

	return find->second;
}

unsigned int SkeletonInfo::GetBoneIndexByMeshName(const std::wstring& meshName) const
{
	auto find = m_meshMappingTable.find(meshName);

	assert(find != m_meshMappingTable.end());

	return find->second;
}

void SkeletonInfo::SetupSkeletonInstance(std::vector<Bone>& out) const
{
	out.reserve(m_bones.size());

	for (const auto& bone : m_bones)
	{
		out.emplace_back(bone.name, bone.parentIndex, bone.index, bone.relative);
	}
}