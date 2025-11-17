#include "PixelShader.h"

#include <d3dcompiler.h>
#include <vector>

#include "MaterialData.h"

void PixelShader::Create(const Microsoft::WRL::ComPtr<ID3D11Device>& device, const std::wstring& filePath,
	unsigned long long materialFlags, unsigned long long renderFlags)
{
	HRESULT hr = S_OK;

	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // _DEBUG

	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBuffer;
	std::vector<D3D_SHADER_MACRO> shaderMacros;

	if (materialFlags & static_cast<unsigned long long>(MaterialKey::DIFFUSE_TEXTURE))
	{
		shaderMacros.push_back({ "USE_DIFFUSE_MAP", nullptr });
	}

	if (materialFlags & static_cast<unsigned long long>(MaterialKey::NORMAL_TEXTURE))
	{
		shaderMacros.push_back({ "USE_NORMAL_MAP", nullptr });
	}

	if (materialFlags & static_cast<unsigned long long>(MaterialKey::SPECULAR_TEXTURE))
	{
		shaderMacros.push_back({ "USE_SPECULAR_MAP", nullptr });
	}

	if (materialFlags & static_cast<unsigned long long>(MaterialKey::EMISSIVE_TEXTURE))
	{
		shaderMacros.push_back({ "USE_EMISSIVE_MAP", nullptr });
	}

	if (materialFlags & static_cast<unsigned long long>(MaterialKey::OPACITY_TEXTURE))
	{
		shaderMacros.push_back({ "USE_OPACITY_MAP", nullptr });
	}

	if (materialFlags & static_cast<unsigned long long>(MaterialKey::DIFFUSE_COLOR))
	{
		shaderMacros.push_back({ "HAS_DIFFUSE_COLOR", nullptr });
	}

	if (materialFlags & static_cast<unsigned long long>(MaterialKey::AMBIENT_COLOR))
	{
		shaderMacros.push_back({ "HAS_AMBIENT_COLOR", nullptr });
	}

	if (materialFlags & static_cast<unsigned long long>(MaterialKey::SPECULAR_COLOR))
	{
		shaderMacros.push_back({ "HAS_SPECULAR_COLOR", nullptr });
	}

	if (materialFlags & static_cast<unsigned long long>(MaterialKey::EMISSIVE_COLOR))
	{
		shaderMacros.push_back({ "HAS_EMISSIVE_COLOR", nullptr });
	}

	if (materialFlags & static_cast<unsigned long long>(MaterialKey::SHININESS_FACTOR))
	{
		shaderMacros.push_back({ "HAS_SHININESS_FACTOR", nullptr });
	}

	if (materialFlags & static_cast<unsigned long long>(MaterialKey::OPACITY_FACTOR))
	{
		shaderMacros.push_back({ "HAS_OPACITY_FACTOR", nullptr });
	}

	if (renderFlags & static_cast<unsigned long long>(RenderFlag::ALPHA_TEST))
	{
		shaderMacros.push_back({ "ALPHA_TEST", nullptr });
	}

	if (renderFlags & static_cast<unsigned long long>(RenderFlag::ALPHA_BLEND))
	{
		shaderMacros.push_back({ "ALPHA_BLEND", nullptr });
	}

	if (renderFlags & static_cast<unsigned long long>(RenderFlag::USE_SHADOW_PCF))
	{
		shaderMacros.push_back({ "USE_SHADOW_PCF", nullptr });
	}

	hr = D3DCompileFromFile(
		filePath.c_str(),
		shaderMacros.data(),
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