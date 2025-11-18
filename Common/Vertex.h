#pragma once

#include <directxtk/SimpleMath.h>
#include <cassert>
#include <d3d11.h>
#include <array>

struct CommonVertex3D
{
	using Vector2 = DirectX::SimpleMath::Vector2;
	using Vector3 = DirectX::SimpleMath::Vector3;

	Vector3 position;
	Vector2 texCoord;
	Vector3 normal;
	Vector3 tangent;
	Vector3 binormal;

	CommonVertex3D() noexcept = default;

	CommonVertex3D(const float* position,
		const float* texCoord,
		const float* normal,
		const float* tangent,
		const float* binormal) noexcept
		: position{ position }, texCoord{ texCoord }, normal{ normal }, tangent{ tangent }, binormal{ binormal }
	{

	}

	CommonVertex3D(const Vector3& position,
		const Vector2& texCoord,
		const Vector3& normal,
		const Vector3& tangent,
		const Vector3& binormal) noexcept
		: position{ position }, texCoord{ texCoord }, normal{ normal }, tangent{ tangent }, binormal{ binormal }
	{

	}

	static std::array<D3D11_INPUT_ELEMENT_DESC, 5> GetLayout()
	{
		// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
		return {
			D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			D3D11_INPUT_ELEMENT_DESC{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			D3D11_INPUT_ELEMENT_DESC{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			D3D11_INPUT_ELEMENT_DESC{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	};
};

struct PositionVertex3D
{
	DirectX::SimpleMath::Vector3 position;

	static std::array<D3D11_INPUT_ELEMENT_DESC, 1> GetLayout()
	{
		// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
		return {
			D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	};
};

struct BoneWeightVertex3D
{
	using Vector2 = DirectX::SimpleMath::Vector2;
	using Vector3 = DirectX::SimpleMath::Vector3;

	Vector3 position;
	Vector2 texCoord;
	Vector3 normal;
	Vector3 tangent;
	Vector3 binormal;
	unsigned int blendIndices[4]{};
	float blendWeights[4]{};

	BoneWeightVertex3D() noexcept = default;

	BoneWeightVertex3D(const float* position,
		const float* texCoord,
		const float* normal,
		const float* tangent,
		const float* binormal) noexcept
		: position{ position }, texCoord{ texCoord }, normal{ normal }, tangent{ tangent }, binormal{ binormal }
	{

	}

	BoneWeightVertex3D(const Vector3& position,
		const Vector2& texCoord,
		const Vector3& normal,
		const Vector3& tangent,
		const Vector3& binormal) noexcept
		: position{ position }, texCoord{ texCoord }, normal{ normal }, tangent{ tangent }, binormal{ binormal }
	{

	}

	void AddBoneData(unsigned int boneIndex, float weight)
	{
		assert(blendWeights[0] == 0.0f || blendWeights[1] == 0.0f || blendWeights[2] == 0.0f || blendWeights[3] == 0.0f);

		for (int i = 0; i < 4; ++i)
		{
			if (blendWeights[i] == 0.0f)
			{
				blendIndices[i] = boneIndex;
				blendWeights[i] = weight;

				return;
			}
		}
	}

	static std::array<D3D11_INPUT_ELEMENT_DESC, 7> GetLayout()
	{
		// SemanticName , SemanticIndex , Format , InputSlot , AlignedByteOffset , InputSlotClass , InstanceDataStepRate
		return {
			D3D11_INPUT_ELEMENT_DESC{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			D3D11_INPUT_ELEMENT_DESC{ "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			D3D11_INPUT_ELEMENT_DESC{ "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			D3D11_INPUT_ELEMENT_DESC{ "TANGENT",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			D3D11_INPUT_ELEMENT_DESC{ "BINORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			D3D11_INPUT_ELEMENT_DESC{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			D3D11_INPUT_ELEMENT_DESC{ "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 72, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	};
};