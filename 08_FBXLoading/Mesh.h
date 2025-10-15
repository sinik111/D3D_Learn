#pragma once

#include <string>
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>
#include <directxtk/SimpleMath.h>

struct aiMesh;

struct Vertex
{
	using Vector2 = DirectX::SimpleMath::Vector2;
	using Vector3 = DirectX::SimpleMath::Vector3;

	Vector3 position;
	Vector2 texCoord;
	Vector3 normal;
	Vector3 tangent;
	Vector3 binormal;

	Vertex(const float* position,
		const float* texCoord,
		const float* normal,
		const float* tangent,
		const float* binormal)
		: position{ position }, texCoord{ texCoord }, normal{ normal }, tangent{ tangent }, binormal{ binormal }
	{

	}

	Vertex(const Vector3& position,
		const Vector2& texCoord,
		const Vector3& normal,
		const Vector3& tangent,
		const Vector3& binormal)
		: position{ position }, texCoord{ texCoord }, normal{ normal }, tangent{ tangent }, binormal{ binormal }
	{

	}
};

class Mesh
{
private:
	std::string m_name;
	std::vector<Vertex> m_vertices;
	std::vector<DWORD> m_indices;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	unsigned int m_materialIndex;

public:
	Mesh(const Microsoft::WRL::ComPtr<ID3D11Device>& device, aiMesh* mesh);

public:
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetVertexBuffer() const;
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetIndexBuffer() const;
	const UINT GetIndexCount() const;
	const unsigned int GetMaterialIndex() const;
};