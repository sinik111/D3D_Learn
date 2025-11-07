#include "SkeletalMeshSection.h"

#include <assimp/mesh.h>

#include "../Common/Vertex.h"
#include "../Common/Helper.h"

#include "Skeleton.h"

SkeletalMeshSection::SkeletalMeshSection(const Microsoft::WRL::ComPtr<ID3D11Device>& device,
	const aiMesh* mesh, SkeletonInfo* skeletonInfo, bool isRigid)
	: m_name{ ToWideCharStr(mesh->mName.C_Str()) }, m_materialIndex{ mesh->mMaterialIndex }
{
	const unsigned int numVertices = mesh->mNumVertices;
	const unsigned int numFaces = mesh->mNumFaces;

	std::vector<DWORD> indices;
	indices.reserve(m_indexCount);
	m_indexCount = numFaces * 3;

	for (unsigned int i = 0; i < numFaces; ++i)
	{
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}

	if (!isRigid)
	{
		std::vector<BoneWeightVertex> vertices;

		vertices.reserve(numVertices);

		for (unsigned int i = 0; i < numVertices; ++i)
		{
			vertices.emplace_back(
				&mesh->mVertices[i].x,
				&mesh->mTextureCoords[0][i].x,
				&mesh->mNormals[i].x,
				&mesh->mTangents[i].x,
				&mesh->mBitangents[i].x);
		}

		const unsigned int numBones = mesh->mNumBones;
		m_boneReferences.reserve(numBones);

		for (unsigned int i = 0; i < numBones; ++i)
		{
			const aiBone* bone = mesh->mBones[i];
			const std::wstring boneName = ToWideCharStr(bone->mName.C_Str());

			const unsigned int boneIndex = skeletonInfo->GetBoneIndexByBoneName(boneName);

			skeletonInfo->SetBoneOffset(DirectX::SimpleMath::Matrix(&bone->mOffsetMatrix.a1), boneIndex);

			m_boneReferences.push_back(boneIndex);

			for (unsigned int j = 0; j < bone->mNumWeights; ++j)
			{
				unsigned int vertexId = bone->mWeights[j].mVertexId;
				float weight = bone->mWeights[j].mWeight;

				vertices[vertexId].AddBoneData(boneIndex, weight);
			}
		}

		D3D11_BUFFER_DESC vertexBufferDesc{};
		vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(BoneWeightVertex) * numVertices);
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA vertexBufferData{};
		vertexBufferData.pSysMem = vertices.data();

		device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer);
	}
	else
	{
		std::vector<Vertex> vertices;

		vertices.reserve(numVertices);

		for (unsigned int i = 0; i < numVertices; ++i)
		{
			vertices.emplace_back(
				&mesh->mVertices[i].x,
				&mesh->mTextureCoords[0][i].x,
				&mesh->mNormals[i].x,
				&mesh->mTangents[i].x,
				&mesh->mBitangents[i].x);
		}

		m_boneReference = skeletonInfo->GetBoneIndexByMeshName(m_name);

		D3D11_BUFFER_DESC vertexBufferDesc{};
		vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * numVertices);
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

		D3D11_SUBRESOURCE_DATA vertexBufferData{};
		vertexBufferData.pSysMem = vertices.data();

		device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer);
	}

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(DWORD) * numFaces * 3);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA indexBufferData{};
	indexBufferData.pSysMem = indices.data();

	device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer);
}

const Microsoft::WRL::ComPtr<ID3D11Buffer>& SkeletalMeshSection::GetVertexBuffer() const 
{
	return m_vertexBuffer;
}

const Microsoft::WRL::ComPtr<ID3D11Buffer>& SkeletalMeshSection::GetIndexBuffer() const 
{
	return m_indexBuffer;
}

UINT SkeletalMeshSection::GetIndexCount() const 
{
	return m_indexCount;
}

unsigned int SkeletalMeshSection::GetMaterialIndex() const 
{
	return m_materialIndex;
}

const std::vector<unsigned int>& SkeletalMeshSection::GetBoneReferences() const
{
	return m_boneReferences;
}

unsigned int SkeletalMeshSection::GetBoneReference() const
{
	return m_boneReference;
}
