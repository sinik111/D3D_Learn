#pragma once

#include <string>
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>
#include <directxtk/SimpleMath.h>

#include "../Common/Vertex.h"

struct aiMesh;

class StaticMeshSection
{
private:
	std::string m_name;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	unsigned int m_materialIndex;
	UINT m_indexCount = 0;

public:
	StaticMeshSection(const Microsoft::WRL::ComPtr<ID3D11Device>& device, aiMesh* mesh);

public:
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetVertexBuffer() const;
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetIndexBuffer() const;
	const UINT GetIndexCount() const;
	const unsigned int GetMaterialIndex() const;
};