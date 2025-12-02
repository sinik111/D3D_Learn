#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "D3DResource.h"

class RasterizerState :
    public D3DResource
{
private:
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

public:
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const D3D11_RASTERIZER_DESC& rsDesc);

public:
    const Microsoft::WRL::ComPtr<ID3D11RasterizerState>& GetRasterizerState() const;
    ID3D11RasterizerState* GetRawRasterizerState() const;
};