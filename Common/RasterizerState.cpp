#include "RasterizerState.h"

void RasterizerState::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const D3D11_RASTERIZER_DESC& rsDesc)
{
    device->CreateRasterizerState(&rsDesc, &m_rasterizerState);
}

const Microsoft::WRL::ComPtr<ID3D11RasterizerState>& RasterizerState::GetRasterizerState() const
{
    return m_rasterizerState;
}

ID3D11RasterizerState* RasterizerState::GetRawRasterizerState() const
{
    return m_rasterizerState.Get();
}