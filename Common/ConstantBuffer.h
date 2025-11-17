#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "D3DResource.h"

class ConstantBuffer :
    public D3DResource
{
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

public:
	void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, UINT byteWidth);

public:
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetBuffer() const;
};