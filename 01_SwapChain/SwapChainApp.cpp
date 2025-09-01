#include "SwapChainApp.h"

#include <directxtk/SimpleMath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE

void SwapChainApp::Initialize()
{
	WinApp::Initialize();

	InitializeD3D();
}

void SwapChainApp::Update()
{

}

void SwapChainApp::Render()
{
#ifdef USE_FLIPMODE
	m_d3d11DeviceContext->OMSetRenderTargets(1, m_d3d11RenderTargetView.GetAddressOf(), nullptr);
#endif // USE_FLIPMODE

	Color color(0.9f, 1.0f, 0.9f, 1.0f);

	m_d3d11DeviceContext->ClearRenderTargetView(m_d3d11RenderTargetView.Get(), color);

	m_dxgiSwapChain->Present(0, 0);
}

void SwapChainApp::InitializeD3D()
{
	UINT d3dCreationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	d3dCreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

	D3D_FEATURE_LEVEL featureLevels[]{
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL actualFeatureLevel;

	D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		d3dCreationFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&m_d3d11Device,
		&actualFeatureLevel,
		&m_d3d11DeviceContext
	);


	UINT dxgiFactoryCreationFlags = 0;
#ifdef _DEBUG
	dxgiFactoryCreationFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // _DEBUG

	ComPtr<IDXGIFactory2> dxgiFactory;
	CreateDXGIFactory2(dxgiFactoryCreationFlags, IID_PPV_ARGS(&dxgiFactory));

	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc{};
#ifdef USE_FLIPMODE
	dxgiSwapChainDesc.BufferCount = 2;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
#else
	dxgiSwapChainDesc.BufferCount = 1;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
#endif // USE_FLIPMODE
	dxgiSwapChainDesc.Width = m_width;
	dxgiSwapChainDesc.Height = m_height;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	dxgiSwapChainDesc.Stereo = FALSE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;

	dxgiFactory->CreateSwapChainForHwnd(
		m_d3d11Device.Get(),
		m_hWnd,
		&dxgiSwapChainDesc,
		nullptr,
		nullptr,
		&m_dxgiSwapChain
	);


	ComPtr<ID3D11Texture2D> backBufferTexture;
	m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBufferTexture);
	m_d3d11Device->CreateRenderTargetView(backBufferTexture.Get(), nullptr, &m_d3d11RenderTargetView);

#ifndef USE_FLIPMODE
	m_d3d11DeviceContext->OMSetRenderTargets(1, m_d3d11RenderTargetView.GetAddressOf(), nullptr);
#endif // USE_FLIPMODE
}