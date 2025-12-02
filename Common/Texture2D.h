#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "D3DResource.h"

class Texture2D :
    public D3DResource
{
private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture2D;

public:
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const D3D11_TEXTURE2D_DESC& texDesc);

public:
    const Microsoft::WRL::ComPtr<ID3D11Texture2D>& GetTexture2D() const;
    ID3D11Texture2D* GetRawTexture2D() const;
};