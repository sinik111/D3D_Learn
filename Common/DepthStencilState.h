#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "D3DResource.h"

class DepthStencilState :
    public D3DResource
{
private:
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

public:
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const D3D11_DEPTH_STENCIL_DESC& dsDesc);

public:
    const Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& GetDepthStencilState() const;
    ID3D11DepthStencilState* GetRawDepthStencilState() const;
};