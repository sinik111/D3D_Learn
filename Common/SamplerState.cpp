#include "SamplerState.h"

void SamplerState::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const D3D11_SAMPLER_DESC& samplerDesc)
{
	device->CreateSamplerState(&samplerDesc, &m_samplerState);
}

const Microsoft::WRL::ComPtr<ID3D11SamplerState>& SamplerState::GetSamplerState() const
{
	return m_samplerState;
}
