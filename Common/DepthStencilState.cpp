#include "DepthStencilState.h"

void DepthStencilState::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const D3D11_DEPTH_STENCIL_DESC& dsDesc)
{
    device->CreateDepthStencilState(&dsDesc, &m_depthStencilState);
}

const Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& DepthStencilState::GetDepthStencilState() const
{
    return m_depthStencilState;
}

ID3D11DepthStencilState* DepthStencilState::GetRawDepthStencilState() const
{
    return m_depthStencilState.Get();
}
