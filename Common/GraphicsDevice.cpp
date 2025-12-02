#include "GraphicsDevice.h"

#include <comdef.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include "DepthStencilView.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE

void GraphicsDevice::Initialize(HWND hWnd, UINT width, UINT height)
{
	m_hWnd = hWnd;
	m_width = width;
	m_height = height;


	// Device, DeviceContext

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


	// SwapChain

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


	// RenderTargetView

	ComPtr<ID3D11Texture2D> backBufferTexture;
	m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBufferTexture);
	m_d3d11Device->CreateRenderTargetView(backBufferTexture.Get(), nullptr, &m_d3d11RenderTargetView);

#ifndef USE_FLIPMODE
	m_d3d11DeviceContext->OMSetRenderTargets(1, m_d3d11RenderTargetView.GetAddressOf(), nullptr);
#endif // USE_FLIPMODE


	// Viewport
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = static_cast<float>(m_width);
	m_viewport.Height = static_cast<float>(m_height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_d3d11DeviceContext->RSSetViewports(1, &m_viewport);


	// Depth, Stencil View

	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_width;
	depthStencilDesc.Height = m_height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> depthStencilTexture;
	m_d3d11Device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilTexture);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	m_d3d11Device->CreateDepthStencilView(depthStencilTexture.Get(), &depthStencilViewDesc, &m_d3d11DepthStencilView);
}

Microsoft::WRL::ComPtr<ID3D11Device> GraphicsDevice::GetDevice() const
{
	return m_d3d11Device;
}

Microsoft::WRL::ComPtr<ID3D11DeviceContext> GraphicsDevice::GetDeviceContext() const
{
	return m_d3d11DeviceContext;
}

Microsoft::WRL::ComPtr<IDXGISwapChain1> GraphicsDevice::GetSwapChain() const
{
	return m_dxgiSwapChain;
}

Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GraphicsDevice::GetRenderTargetView() const
{
	return m_d3d11RenderTargetView;
}

Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GraphicsDevice::GetDepthStencilView() const
{
	return m_d3d11DepthStencilView;
}

const D3D11_VIEWPORT& GraphicsDevice::GetViewport() const
{
	return m_viewport;
}

void GraphicsDevice::BeginDraw(const DirectX::SimpleMath::Color& clearColor)
{
#ifdef USE_FLIPMODE
	m_d3d11DeviceContext->OMSetRenderTargets(1, m_d3d11RenderTargetView.GetAddressOf(), m_d3d11DepthStencilView.Get());
#endif // USE_FLIPMODE

	m_d3d11DeviceContext->ClearRenderTargetView(m_d3d11RenderTargetView.Get(), clearColor);
	m_d3d11DeviceContext->ClearDepthStencilView(m_d3d11DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void GraphicsDevice::EndDraw()
{
	m_dxgiSwapChain->Present(0, 0);
}

HRESULT GraphicsDevice::CompileShaderFromFile(const std::wstring& fileName, const std::string& entryPoint, const std::string& shaderModel, ComPtr<ID3DBlob>& blobOut)
{
	HRESULT hr = S_OK;

	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	shaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // _DEBUG

	ComPtr<ID3DBlob> errorBlob = nullptr;

	hr = D3DCompileFromFile(
		fileName.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint.c_str(),
		shaderModel.c_str(),
		shaderFlags,
		0,
		blobOut.GetAddressOf(),
		&errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			MessageBoxA(NULL, (char*)errorBlob->GetBufferPointer(), "CompileShaderFromFile", MB_OK);
		}

		return hr;
	}

	return S_OK;
}