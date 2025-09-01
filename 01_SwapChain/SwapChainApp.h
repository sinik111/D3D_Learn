#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi1_3.h>

#include "../Common/WinApp.h"

class SwapChainApp :
	public WinApp
{
private:
	Microsoft::WRL::ComPtr<ID3D11Device> m_d3d11Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> m_dxgiSwapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_d3d11RenderTargetView;

public:
	void Initialize() override;
	
private:
	void Update() override;
	void Render() override;

private:
	void InitializeD3D();
};