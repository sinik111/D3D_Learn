#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

#include "D3DResource.h"
#include "Vertex.h"

class VertexBuffer :
    public D3DResource
{
private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;
    UINT m_bufferStride = 0;

public:
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::vector<CommonVertex3D>& vertices);
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::vector<BoneWeightVertex3D>& vertices);
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::vector<PositionNormalVertex3D>& vertices);
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::vector<PositionVertex3D>& vertices);

public:
    const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetBuffer() const;
    ID3D11Buffer* GetRawBuffer() const;
    UINT GetBufferStride() const;
};