#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "D3DResource.h"

enum class SamplerType
{
    Point,
    Linear,
    Comparison
};

class SamplerState :
    public D3DResource
{
private:
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

public:
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, SamplerType type);

public:
    const Microsoft::WRL::ComPtr<ID3D11SamplerState>& GetSamplerState() const;
};

