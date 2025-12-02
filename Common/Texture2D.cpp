#include "Texture2D.h"

void Texture2D::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const D3D11_TEXTURE2D_DESC& texDesc)
{
	device->CreateTexture2D(&texDesc, nullptr, &m_texture2D);
}

const Microsoft::WRL::ComPtr<ID3D11Texture2D>& Texture2D::GetTexture2D() const
{
	return m_texture2D;
}

ID3D11Texture2D* Texture2D::GetRawTexture2D() const
{
	return m_texture2D.Get();
}
