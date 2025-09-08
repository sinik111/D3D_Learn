#pragma once

#include "../Common/WinApp.h"

class TriangleApp :
	public WinApp
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	UINT m_vertexBufferStride = 0;
	UINT m_vertexBufferOffset = 0;
	UINT m_vertexCount = 0;

public:
	void Initialize() override;

private:
	void OnRender() override;

private:
	void CreateTriangle();
};