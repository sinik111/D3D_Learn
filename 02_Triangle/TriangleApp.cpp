#include "TriangleApp.h"

#include <directxtk/SimpleMath.h>

using namespace DirectX::SimpleMath;
using Microsoft::WRL::ComPtr;

#define USE_FLIPMODE

struct Vertex
{
	Vector3 position;
};

void TriangleApp::Initialize()
{
	WinApp::Initialize();

	CreateTriangle();
}

void TriangleApp::OnRender()
{
	m_graphicsDevice.BeginDraw({ 0.9f, 0.9f, 1.0f, 1.0f });

	auto deviceContext = m_graphicsDevice.GetDeviceContext();

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &m_vertexBufferStride, &m_vertexBufferOffset);
	deviceContext->IASetInputLayout(m_inputLayout.Get());
	deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	deviceContext->Draw(m_vertexCount, 0);

	m_graphicsDevice.EndDraw();
}

void TriangleApp::CreateTriangle()
{
	auto device = m_graphicsDevice.GetDevice();

	Vertex vertices[] {
		Vector3{ -0.5f, -0.5f, 0.5f },
		Vector3{ 0.0f, 0.5f, 0.5f },
		Vector3{ 0.5f, -0.5f, 0.5f }
	};

	m_vertexCount = ARRAYSIZE(vertices);

	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * m_vertexCount;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA vertexBufferData{};
	vertexBufferData.pSysMem = vertices;

	device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &m_vertexBuffer);

	m_vertexBufferStride = sizeof(Vertex);
	m_vertexBufferOffset = 0;


	D3D11_INPUT_ELEMENT_DESC layout[]{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ComPtr<ID3DBlob> vertexShaderBuffer;
	GraphicsDevice::CompileShaderFromFile(L"BasicVertexShader.hlsl", "main", "vs_4_0", vertexShaderBuffer);

	device->CreateInputLayout(
		layout,
		ARRAYSIZE(layout),
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		&m_inputLayout
	);

	device->CreateVertexShader(
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		nullptr,
		&m_vertexShader
	);

	ComPtr<ID3DBlob> pixelShaderBuffer;
	GraphicsDevice::CompileShaderFromFile(L"BasicPixelShader.hlsl", "main", "ps_4_0", pixelShaderBuffer);

	device->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(),
		nullptr,
		&m_pixelShader
	);
}