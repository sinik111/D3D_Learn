#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>

#include "D3DResource.h"

enum class TextureType
{
    Texture2D,
    TextureCube
};

class ShaderResourceView :
    public D3DResource
{
private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResourceView;

public:
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filePath, TextureType type);
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device,const D3D11_TEXTURE2D_DESC& textureDesc,
        const D3D11_SUBRESOURCE_DATA& subData);

public:
    const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetShaderResourceView() const;
};

