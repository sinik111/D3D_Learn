#include "PixelShader.h"

#include <d3dcompiler.h>
#include <vector>

#include "MaterialData.h"

void PixelShader::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filePath)
{
	HRESULT hr = S_OK;

	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // _DEBUG

	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBuffer;

	hr = D3DCompileFromFile(
		filePath.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",
		shaderFlags,
		0,
		&pixelShaderBuffer,
		&errorBlob);

	if (FAILED(hr))
	{
		if (errorBlob)
		{
			MessageBoxA(NULL, (char*)errorBlob->GetBufferPointer(), "D3DCompileFromFile", MB_OK);
		}
	}

	device->CreatePixelShader(
		pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(),
		nullptr,
		&m_pixelShader
	);
}

const Microsoft::WRL::ComPtr<ID3D11PixelShader>& PixelShader::GetShader() const
{
	return m_pixelShader;
}