#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <string>

#include "D3DResource.h"

class InputLayout :
    public D3DResource
{
private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

public:
	void Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filePath,
		const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements);

public:
	const Microsoft::WRL::ComPtr<ID3D11InputLayout>& GetInputLayout() const;
};

