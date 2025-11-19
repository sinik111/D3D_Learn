#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

#include "D3DResource.h"

class IndexBuffer :
    public D3DResource
{
private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

public:
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::vector<DWORD>& indices);

public:
    const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetBuffer() const;
    ID3D11Buffer* GetRawBuffer() const;
};