#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "D3DResource.h"

class DepthStencilView :
    public D3DResource
{
private:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;

public:
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device,
        const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture2D,
        const D3D11_DEPTH_STENCIL_VIEW_DESC& dsvDesc);

public:
    const Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& GetDepthStencilView() const;
    ID3D11DepthStencilView* GetRawDepthStencilView() const;
};