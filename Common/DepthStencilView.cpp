#include "DepthStencilView.h"

void DepthStencilView::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device,
	const Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture2D,
	const D3D11_DEPTH_STENCIL_VIEW_DESC& dsvDesc)
{
	device->CreateDepthStencilView(texture2D.Get(), &dsvDesc, &m_depthStencilView);
}

const Microsoft::WRL::ComPtr<ID3D11DepthStencilView>& DepthStencilView::GetDepthStencilView() const
{
	return m_depthStencilView;
}

ID3D11DepthStencilView* DepthStencilView::GetRawDepthStencilView() const
{
	return m_depthStencilView.Get();
}