#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi1_6.h>
#include <string>
#include <directxtk/SimpleMath.h>

class GraphicsDevice
{
private:
	Microsoft::WRL::ComPtr<ID3D11Device> m_d3d11Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_dxgiSwapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_backBufferRTV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_gameRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_gameDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_gameSRV;

	// game render target quad
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_quadVS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_quadLDRPS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_quadHDRPS;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_quadInputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_quadVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_quadIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerLinear;
	UINT m_quadVertexBufferStride = 0;
	UINT m_quadVertexBufferOffset = 0;
	UINT m_quadIndexCount = 0;

	HWND m_hWnd = nullptr;
	UINT m_width = 0;
	UINT m_height = 0;
	D3D11_VIEWPORT m_viewport{};
	DXGI_FORMAT m_format;
	float m_monitorMaxNits = 0.0f;
	bool m_forceLDR = false;
	bool m_isHDRSupported = false;

public:
	void Initialize(HWND hWnd, UINT width, UINT height);

	Microsoft::WRL::ComPtr<ID3D11Device> GetDevice() const;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetDeviceContext() const;
	Microsoft::WRL::ComPtr<IDXGISwapChain> GetSwapChain() const;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetRenderTargetView() const;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GetDepthStencilView() const;
	const D3D11_VIEWPORT& GetViewport() const;
	float GetMonitorMaxNits() const;

	void SetForceLDR(bool forceLDR);

	void BeginDraw(const DirectX::SimpleMath::Color& clearColor);
	void BackBufferDraw();
	void EndDraw();

private:
	void CheckHDRSupportedAndGetMaxNits();

public:
	static HRESULT CompileShaderFromFile(
		const std::wstring& fileName, 
		const std::string& entryPoint,
		const std::string& shaderModel,
		Microsoft::WRL::ComPtr<ID3DBlob>& blobOut);
};