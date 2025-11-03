#pragma once

#include <vector>
#include <string>
#include <d3d11.h>
#include <wrl/client.h>

struct aiMesh;
class SkeletonInfo;

class SkeletalMeshSection
{
private:
	std::wstring m_name;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	std::vector<unsigned int> m_boneReferences;
	unsigned int m_materialIndex;
	UINT m_indexCount = 0;

public:
	SkeletalMeshSection(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const aiMesh* mesh, SkeletonInfo* skeletonInfo);

public:
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetVertexBuffer() const;
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetIndexBuffer() const;
	const UINT GetIndexCount() const;
	const unsigned int GetMaterialIndex() const;
	const std::vector<unsigned int>& GetBoneReferences() const;
};