#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>

#include "D3DResource.h"

enum class RenderFlag : unsigned long long
{
    ALPHA_TEST          = 1ULL << 0,
    ALPHA_BLEND         = 1ULL << 1,
    USE_SHADOW_PCF      = 1ULL << 2
};

class PixelShader :
    public D3DResource
{
private:
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

public:
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filePath,
        unsigned long long materialFlags, unsigned long long renderFlags);

public:
    const Microsoft::WRL::ComPtr<ID3D11PixelShader>& GetShader() const;
};

