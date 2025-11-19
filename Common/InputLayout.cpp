#include "InputLayout.h"

#include <d3dcompiler.h>
#include <d3d11.h>

#include "Vertex.h"

void InputLayout::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filePath,
	const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements)
{
	HRESULT hr = S_OK;

	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // _DEBUG

	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBuffer;

	hr = D3DCompileFromFile(
		filePath.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",
		shaderFlags,
		0,
		&vertexShaderBuffer,
		&errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			MessageBoxA(NULL, (char*)errorBlob->GetBufferPointer(), "D3DCompileFromFile", MB_OK);
		}
	}

	device->CreateInputLayout(
		layoutDesc,
		numElements,
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		&m_inputLayout
	);
}

const Microsoft::WRL::ComPtr<ID3D11InputLayout>& InputLayout::GetInputLayout() const
{
	return m_inputLayout;
}

ID3D11InputLayout* InputLayout::GetRawInputLayout() const
{
	return m_inputLayout.Get();
}
