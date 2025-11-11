#include "StaticMeshSection.h"

#include <assimp/mesh.h>

StaticMeshSection::StaticMeshSection(const Microsoft::WRL::ComPtr<ID3D11Device>& device, aiMesh* mesh)
	: m_name{ mesh->mName.C_Str() }, m_materialIndex{ mesh->mMaterialIndex }
{
	const unsigned int numVertices = mesh->mNumVertices;
	const unsigned int numFaces = mesh->mNumFaces;
	m_indexCount = numFaces * 3;

	std::vector<CommonVertex3D> vertices;
	std::vector<DWORD> indices;

	vertices.reserve(numVertices);
	indices.reserve(m_indexCount);

	for (unsigned int i = 0; i < numVertices; ++i)
	{
		vertices.emplace_back(
			&mesh->mVertices[i].x,
			&mesh->mTextureCoords[0][i].x,
			&mesh->mNormals[i].x,
			&mesh->mTangents[i].x,
			&mesh->mBitangents[i].x);
	}

	for (unsigned int i = 0; i < numFaces; ++i)
	{
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}


	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(CommonVertex3D) * numVertices);
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vertexBufferData{};
	vertexBufferData.pSysMem = vertices.data();

	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer);


	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(DWORD) * m_indexCount);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA indexBufferData{};
	indexBufferData.pSysMem = indices.data();

	device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer);
}

const Microsoft::WRL::ComPtr<ID3D11Buffer>& StaticMeshSection::GetVertexBuffer() const
{
	return m_vertexBuffer;
}

const Microsoft::WRL::ComPtr<ID3D11Buffer>& StaticMeshSection::GetIndexBuffer() const
{
	return m_indexBuffer;
}

const UINT StaticMeshSection::GetIndexCount() const
{
	return m_indexCount;
}

const unsigned int StaticMeshSection::GetMaterialIndex() const
{
	return m_materialIndex;
}
