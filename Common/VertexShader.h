#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>

#include "D3DResource.h"

class VertexShader :
    public D3DResource
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;

public:
	void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filePath);

public:
	const Microsoft::WRL::ComPtr<ID3D11VertexShader>& GetShader() const;
};