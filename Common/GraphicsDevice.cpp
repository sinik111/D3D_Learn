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

	CheckHDRSupportedAndGetMaxNits();

	if (m_forceLDR)
	{
		m_format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	// Device, DeviceContext, SwapChain

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = m_hWnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.BufferDesc.Format = m_format;
	swapChainDesc.BufferDesc.Width = m_width;
	swapChainDesc.BufferDesc.Height = m_height;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

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

	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		d3dCreationFlags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_dxgiSwapChain,
		&m_d3d11Device,
		&actualFeatureLevel,
		&m_d3d11DeviceContext
	);

	// RenderTargetView - backbuffer

	{
		ComPtr<ID3D11Texture2D> backBufferTexture;
		m_dxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBufferTexture);
		m_d3d11Device->CreateRenderTargetView(backBufferTexture.Get(), nullptr, &m_backBufferRTV);


		Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3;
		m_dxgiSwapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&swapChain3);
		if (m_format == DXGI_FORMAT_R10G10B10A2_UNORM)
		{
			swapChain3->SetColorSpace1(DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
		}
	}

	// Viewport
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = static_cast<float>(m_width);
	m_viewport.Height = static_cast<float>(m_height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_d3d11DeviceContext->RSSetViewports(1, &m_viewport);


	// Depth, Stencil View - game

	{
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

		m_d3d11Device->CreateDepthStencilView(depthStencilTexture.Get(), &depthStencilViewDesc, &m_gameDSV);
	}

	// RenderTargetView - game
	{
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = m_width;
		texDesc.Height = m_height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		ComPtr<ID3D11Texture2D> gameRenderTargetTexture;
		m_d3d11Device->CreateTexture2D(&texDesc, nullptr, &gameRenderTargetTexture);
		m_d3d11Device->CreateRenderTargetView(gameRenderTargetTexture.Get(), nullptr, &m_gameRTV);
		m_d3d11Device->CreateShaderResourceView(gameRenderTargetTexture.Get(), nullptr, &m_gameSRV);
	}

	// quad 임시로 여기서 만듦
	{
		// vertex, vs
		{
			struct QuadVertex
			{
				DirectX::SimpleMath::Vector3 position;
				DirectX::SimpleMath::Vector2 texCoord;
			};

			QuadVertex quadVertices[]{
				{ { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
				{ { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
				{ { -1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f } },
				{ { 1.0f, -1.0f, 1.0f }, { 1.0f, 1.0f } }
			};

			D3D11_BUFFER_DESC desc{};
			desc.ByteWidth = sizeof(QuadVertex) * ARRAYSIZE(quadVertices);
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.Usage = D3D11_USAGE_DEFAULT;

			D3D11_SUBRESOURCE_DATA data{};
			data.pSysMem = quadVertices;

			m_d3d11Device->CreateBuffer(&desc, &data, &m_quadVertexBuffer);
			m_quadVertexBufferStride = sizeof(QuadVertex);
			m_quadVertexBufferOffset = 0;

			D3D11_INPUT_ELEMENT_DESC layout[]{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
			};

			ComPtr<ID3D10Blob> vsBuffer;
			CompileShaderFromFile(L"FullScreenQuadVS.hlsl", "main", "vs_5_0", vsBuffer);
			m_d3d11Device->CreateInputLayout(
				layout,
				ARRAYSIZE(layout),
				vsBuffer->GetBufferPointer(),
				vsBuffer->GetBufferSize(),
				&m_quadInputLayout);
			m_d3d11Device->CreateVertexShader(
				vsBuffer->GetBufferPointer(),
				vsBuffer->GetBufferSize(),
				nullptr,
				&m_quadVS);
		}

		// index
		{
			WORD indices[]{
				0, 1, 2,
				2, 1, 3
			};

			m_quadIndexCount = ARRAYSIZE(indices);

			D3D11_BUFFER_DESC desc{};
			desc.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			desc.Usage = D3D11_USAGE_DEFAULT;

			D3D11_SUBRESOURCE_DATA data{};
			data.pSysMem = indices;

			m_d3d11Device->CreateBuffer(&desc, &data, &m_quadIndexBuffer);
		}

		// ps
		{
			{
				ComPtr<ID3D10Blob> psBuffer;
				CompileShaderFromFile(L"QuadHDRPS.hlsl", "main", "ps_5_0", psBuffer);
				m_d3d11Device->CreatePixelShader(
					psBuffer->GetBufferPointer(),
					psBuffer->GetBufferSize(),
					nullptr,
					&m_quadHDRPS);
			}
			{
				ComPtr<ID3D10Blob> psBuffer;
				CompileShaderFromFile(L"QuadLDRPS.hlsl", "main", "ps_5_0", psBuffer);
				m_d3d11Device->CreatePixelShader(
					psBuffer->GetBufferPointer(),
					psBuffer->GetBufferSize(),
					nullptr,
					&m_quadLDRPS);
			}
		}

		// sampler
		{
			D3D11_SAMPLER_DESC desc{};
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			desc.MinLOD = 0;
			desc.MaxLOD = D3D11_FLOAT32_MAX;

			m_d3d11Device->CreateSamplerState(&desc, &m_samplerLinear);
		}
	}
}

Microsoft::WRL::ComPtr<ID3D11Device> GraphicsDevice::GetDevice() const
{
	return m_d3d11Device;
}

Microsoft::WRL::ComPtr<ID3D11DeviceContext> GraphicsDevice::GetDeviceContext() const
{
	return m_d3d11DeviceContext;
}

Microsoft::WRL::ComPtr<IDXGISwapChain> GraphicsDevice::GetSwapChain() const
{
	return m_dxgiSwapChain;
}

Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GraphicsDevice::GetRenderTargetView() const
{
	return m_gameRTV;
}

Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GraphicsDevice::GetDepthStencilView() const
{
	return m_gameDSV;
}

const D3D11_VIEWPORT& GraphicsDevice::GetViewport() const
{
	return m_viewport;
}

float GraphicsDevice::GetMonitorMaxNits() const
{
	return m_monitorMaxNits;
}

void GraphicsDevice::SetForceLDR(bool forceLDR)
{
	m_forceLDR = forceLDR;
}

void GraphicsDevice::BeginDraw(const DirectX::SimpleMath::Color& clearColor)
{
	m_d3d11DeviceContext->OMSetRenderTargets(1, m_gameRTV.GetAddressOf(), m_gameDSV.Get());

	m_d3d11DeviceContext->ClearRenderTargetView(m_gameRTV.Get(), clearColor);
	m_d3d11DeviceContext->ClearDepthStencilView(m_gameDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void GraphicsDevice::BackBufferDraw()
{
	m_d3d11DeviceContext->OMSetRenderTargets(1, m_backBufferRTV.GetAddressOf(), nullptr);
	m_d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_d3d11DeviceContext->IASetInputLayout(m_quadInputLayout.Get());
	m_d3d11DeviceContext->IASetVertexBuffers(0, 1, m_quadVertexBuffer.GetAddressOf(), &m_quadVertexBufferStride, &m_quadVertexBufferOffset);
	m_d3d11DeviceContext->IASetIndexBuffer(m_quadIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	m_d3d11DeviceContext->VSSetShader(m_quadVS.Get(), nullptr, 0);

	switch (m_format)
	{
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		m_d3d11DeviceContext->PSSetShader(m_quadHDRPS.Get(), nullptr, 0);
		break;
	default:
		m_d3d11DeviceContext->PSSetShader(m_quadLDRPS.Get(), nullptr, 0);
		break;
	}

	m_d3d11DeviceContext->PSSetShaderResources(0, 1, m_gameSRV.GetAddressOf());
	m_d3d11DeviceContext->PSSetSamplers(0, 1, m_samplerLinear.GetAddressOf());
	m_d3d11DeviceContext->DrawIndexed(m_quadIndexCount, 0, 0);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	m_d3d11DeviceContext->PSSetShaderResources(0, 1, &nullSRV);
}

void GraphicsDevice::EndDraw()
{
	m_dxgiSwapChain->Present(0, 0);
}

void GraphicsDevice::CheckHDRSupportedAndGetMaxNits()
{
	ComPtr<IDXGIFactory4> pFactory;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	
	// 2. 주 그래픽 어댑터 (0번) 열거
	ComPtr<IDXGIAdapter1> pAdapter;
	UINT adapterIndex = 0;
	while (pFactory->EnumAdapters1(adapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		pAdapter->GetDesc1(&desc);

		// WARP 어댑터(소프트웨어)를 건너뛰고 주 어댑터만 사용하도록 선택할 수 있습니다.
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			adapterIndex++;
			pAdapter.Reset();
			continue;
		}
		break;
	}

	// 3. 주 모니터 출력 (0번) 열거
	ComPtr<IDXGIOutput> pOutput;
	hr = pAdapter->EnumOutputs(0, &pOutput); // 0번 출력

	// 4. HDR 정보를 얻기 위해 IDXGIOutput6으로 쿼리
	ComPtr<IDXGIOutput6> pOutput6;
	hr = pOutput.As(&pOutput6);
	if (FAILED(hr))
	{
		// hdr 지원 x
		m_format = DXGI_FORMAT_R8G8B8A8_UNORM;
		m_monitorMaxNits = 100.0f;
		m_isHDRSupported = false;

		return;
	}

	// 5. DXGI_OUTPUT_DESC1에서 HDR 정보 확인
	DXGI_OUTPUT_DESC1 desc1 = {};
	hr = pOutput6->GetDesc1(&desc1);

	// 6. HDR 활성화 조건 분석
	bool isHDRColorSpace = (desc1.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020);
	m_monitorMaxNits = (float)desc1.MaxLuminance;

	// OS가 HDR을 켰을 때 MaxLuminance는 100 Nits(SDR 기준)를 초과합니다.
	bool isHDRActive = m_monitorMaxNits > 100.0f;

	if (isHDRColorSpace && isHDRActive)
	{
		// 최종 판단: HDR 지원 및 OS 활성화
		m_format = DXGI_FORMAT_R10G10B10A2_UNORM; // HDR 포맷 설정
		m_isHDRSupported = true;
	}
	else
	{
		// HDR 지원 안함 또는 OS에서 비활성화
		m_monitorMaxNits = 100.0f; // SDR 기본값
		m_format = DXGI_FORMAT_R8G8B8A8_UNORM; // SDR 포맷 설정
		m_isHDRSupported = false;
	}
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