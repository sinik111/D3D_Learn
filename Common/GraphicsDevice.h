#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <string>
#include <directxtk/SimpleMath.h>

class GraphicsDevice
{
private:
	Microsoft::WRL::ComPtr<ID3D11Device> m_d3d11Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> m_dxgiSwapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_d3d11RenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_d3d11DepthStencilView;

	HWND m_hWnd = nullptr;
	UINT m_width = 0;
	UINT m_height = 0;

public:
	void Initialize(HWND hWnd, UINT width, UINT height);

	Microsoft::WRL::ComPtr<ID3D11Device> GetDevice() const;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetDeviceContext() const;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> GetSwapChain() const;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> GetRenderTargetView() const;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> GetDepthStencilView() const;

	void BeginDraw(const DirectX::SimpleMath::Color& clearColor);
	void EndDraw();

	static HRESULT CompileShaderFromFile(
		const std::wstring& fileName, 
		const std::string& entryPoint,
		const std::string& shaderModel,
		Microsoft::WRL::ComPtr<ID3DBlob>& blobOut);
};