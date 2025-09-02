#include "RectangleApp.h"

#include <comdef.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE

static HRESULT CompileShaderFromFile(const std::wstring& fileName, const std::string& entryPoint, const std::string& shaderModel, ComPtr<ID3DBlob>* blobOut);

struct Vertex
{
	Vector3 position;
	Vector4 color;
};

void RectangleApp::Initialize()
{
	WinApp::Initialize();

	InitializeD3D();

	CreateRectangle();
}

void RectangleApp::Update()
{

}

void RectangleApp::Render()
{
#ifdef USE_FLIPMODE
	m_d3d11DeviceContext->OMSetRenderTargets(1, m_d3d11RenderTargetView.GetAddressOf(), nullptr);
#endif // USE_FLIPMODE

	Color color(1.0f, 0.9f, 0.9f, 1.0f);

	m_d3d11DeviceContext->ClearRenderTargetView(m_d3d11RenderTargetView.Get(), color);

	m_d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_d3d11DeviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
	m_d3d11DeviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	m_d3d11DeviceContext->IASetInputLayout(m_inputLayout.Get());
	m_d3d11DeviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_d3d11DeviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	m_d3d11DeviceContext->DrawIndexed(m_indexCount, 0, 0);

	m_dxgiSwapChain->Present(0, 0);
}

void RectangleApp::InitializeD3D()
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

	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(m_width);
	viewport.Height = static_cast<float>(m_height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_d3d11DeviceContext->RSSetViewports(1, &viewport);
}

void RectangleApp::CreateRectangle()
{
	Vertex vertices[] {
		{
			Vector3{ -0.5f, 0.5f, 0.5f },
			Vector4{ 1.0f, 0.8f, 0.8f, 1.0f }
		},
		{
			Vector3{ 0.5f, 0.5f, 0.5f },
			Vector4{ 0.5f, 0.5f, 1.0f, 1.0f }
		},
		{
			Vector3{ -0.5f, -0.5f, 0.5f },
			Vector4{ 0.5f, 0.9f, 0.2f, 1.0f }
		},
		{
			Vector3{ 0.5f, -0.5f, 0.5f },
			Vector4{ 0.4f, 1.0f, 1.0f, 1.0f }
		}
	};

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vertices);
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vertexBufferData{};
	vertexBufferData.pSysMem = vertices;

	m_d3d11Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer);

	m_vertexBufferStride = sizeof(Vertex);
	m_vertexBufferOffset = 0;

	D3D11_INPUT_ELEMENT_DESC layout[]{
		// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};


	ComPtr<ID3DBlob> vertexShaderBuffer;
	CompileShaderFromFile(L"BasicVertexShader.hlsl", "main", "vs_4_0", &vertexShaderBuffer);

	m_d3d11Device->CreateInputLayout(
		layout,
		ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		&m_inputLayout
	);

	m_d3d11Device->CreateVertexShader(
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		nullptr,
		&m_vertexShader
	);


	WORD indices[]{
		0, 1, 2,
		2, 1, 3
	};

	m_indexCount = ARRAYSIZE(indices);

	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.ByteWidth = sizeof(WORD) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA indexBufferData{};
	indexBufferData.pSysMem = indices;
	m_d3d11Device->CreateBuffer(&indexBufferDesc, &indexBufferData, &m_indexBuffer);


	ComPtr<ID3DBlob> pixelShaderBuffer;
	CompileShaderFromFile(L"BasicPixelShader.hlsl", "main", "ps_4_0", &pixelShaderBuffer);

	m_d3d11Device->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(),
		nullptr,
		&m_pixelShader
	);
}

static HRESULT CompileShaderFromFile(const std::wstring& fileName, const std::string& entryPoint, const std::string& shaderModel, ComPtr<ID3DBlob>* blobOut)
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
		blobOut->GetAddressOf(),
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