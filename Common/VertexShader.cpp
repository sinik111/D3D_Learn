#include "VertexShader.h"

#include <d3dcompiler.h>

void VertexShader::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filePath)
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

	device->CreateVertexShader(
		vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(),
		nullptr,
		&m_vertexShader
	);
}

const Microsoft::WRL::ComPtr<ID3D11VertexShader>& VertexShader::GetShader() const
{
	return m_vertexShader;
}

ID3D11VertexShader* VertexShader::GetRawShader() const
{
	return m_vertexShader.Get();
}