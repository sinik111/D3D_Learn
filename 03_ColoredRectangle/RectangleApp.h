#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi1_3.h>

#include "../Common/WinApp.h"

class RectangleApp :
	public WinApp
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	UINT m_vertexBufferStride = 0;
	UINT m_vertexBufferOffset = 0;
	UINT m_indexCount = 0;

public:
	void Initialize() override;

private:
	void Update() override;
	void Render() override;

private:
	void CreateRectangle();
};