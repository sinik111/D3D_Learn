#include "Mesh.h"

#include <assimp/mesh.h>

Mesh::Mesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, aiMesh* mesh)
	: m_name{ mesh->mName.C_Str() }, m_materialIndex{ mesh->mMaterialIndex }
{
	// primitive type은 triangle list만 고려함.
	// 그래서 indices = faces * 3
	// 한 mesh에 필요한 texCoord는 다 똑같다고 고려함.
	// texCoord는 2d 만 고려함.

	const unsigned int numVertices = mesh->mNumVertices;
	const unsigned int numFaces = mesh->mNumFaces;

	m_vertices.reserve(numVertices);
	m_indices.reserve(numFaces * 3);

	for (unsigned int i = 0; i < numVertices; ++i)
	{
		m_vertices.emplace_back(
			&mesh->mVertices[i].x,
			&mesh->mTextureCoords[0][i].x,
			&mesh->mNormals[i].x,
			&mesh->mTangents[i].x,
			&mesh->mBitangents[i].x);
	}

	for (unsigned int i = 0; i < numFaces; ++i)
	{
		m_indices.push_back(mesh->mFaces[i].mIndices[0]);
		m_indices.push_back(mesh->mFaces[i].mIndices[1]);
		m_indices.push_back(mesh->mFaces[i].mIndices[2]);
	}


	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(CommonVertex3D) * numVertices);
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vertexBufferData{};
	vertexBufferData.pSysMem = m_vertices.data();

	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer);


	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(DWORD) * numFaces * 3);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA indexBufferData{};
	indexBufferData.pSysMem = m_indices.data();

	device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer);
}

const Microsoft::WRL::ComPtr<ID3D11Buffer>& Mesh::GetVertexBuffer() const
{
	return m_vertexBuffer;
}

const Microsoft::WRL::ComPtr<ID3D11Buffer>& Mesh::GetIndexBuffer() const
{
	return m_indexBuffer;
}

const UINT Mesh::GetIndexCount() const
{
	return static_cast<UINT>(m_indices.size());
}

const unsigned int Mesh::GetMaterialIndex() const
{
	return m_materialIndex;
}
