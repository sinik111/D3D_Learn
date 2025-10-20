#pragma once

#include <string>
#include <vector>
#include <d3d11.h>
#include <wrl/client.h>
#include <directxtk/SimpleMath.h>

#include "../Common/Vertex.h"

struct aiMesh;

class SkeletalMeshSection
{
private:
	std::string m_name;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	unsigned int m_materialIndex;
	unsigned int m_boneIndex;
	UINT m_indexCount = 0;
};