#include "VertexBuffer.h"

void VertexBuffer::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::vector<CommonVertex3D>& vertices)
{
	m_bufferStride = sizeof(CommonVertex3D);

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(CommonVertex3D) * vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vertexBufferData{};
	vertexBufferData.pSysMem = vertices.data();

	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_buffer);
}

void VertexBuffer::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::vector<BoneWeightVertex3D>& vertices)
{
	m_bufferStride = sizeof(BoneWeightVertex3D);

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(BoneWeightVertex3D) * vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vertexBufferData{};
	vertexBufferData.pSysMem = vertices.data();

	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_buffer);
}

const Microsoft::WRL::ComPtr<ID3D11Buffer>& VertexBuffer::GetBuffer() const
{
	return m_buffer;
}

UINT VertexBuffer::GetBufferStride() const
{
	return m_bufferStride;
}
