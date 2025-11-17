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

public:
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::vector<CommonVertex3D>& vertices);

public:
    const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetBuffer() const;
};