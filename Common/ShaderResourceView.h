#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>
#include <array>

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
    void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture2D,
        const D3D11_SHADER_RESOURCE_VIEW_DESC& srvDesc);

public:
    const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetShaderResourceView() const;
    ID3D11ShaderResourceView* GetRawShaderResourceView() const;
};

struct TextureSRVs
{
    std::shared_ptr<ShaderResourceView> diffuseTextureSRV;
    std::shared_ptr<ShaderResourceView> normalTextureSRV;
    std::shared_ptr<ShaderResourceView> specularTextureSRV;
    std::shared_ptr<ShaderResourceView> emissiveTextureSRV;
    std::shared_ptr<ShaderResourceView> opacityTextureSRV;
    std::shared_ptr<ShaderResourceView> metalnessTextureSRV;
    std::shared_ptr<ShaderResourceView> roughnessTextureSRV;

    std::array<ID3D11ShaderResourceView*, 7> AsRawArray() const
    {
        return {
            diffuseTextureSRV->GetRawShaderResourceView(),
            normalTextureSRV->GetRawShaderResourceView(),
            specularTextureSRV->GetRawShaderResourceView(),
            emissiveTextureSRV->GetRawShaderResourceView(),
            opacityTextureSRV->GetRawShaderResourceView(),
            metalnessTextureSRV->GetRawShaderResourceView(),
            roughnessTextureSRV->GetRawShaderResourceView(),
        };
    }
};