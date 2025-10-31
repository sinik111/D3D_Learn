#pragma once

#include <directxtk/SimpleMath.h>
#include <cassert>

struct Vertex
{
	using Vector2 = DirectX::SimpleMath::Vector2;
	using Vector3 = DirectX::SimpleMath::Vector3;

	Vector3 position;
	Vector2 texCoord;
	Vector3 normal;
	Vector3 tangent;
	Vector3 binormal;

	Vertex() noexcept = default;

	Vertex(const float* position,
		const float* texCoord,
		const float* normal,
		const float* tangent,
		const float* binormal) noexcept
		: position{ position }, texCoord{ texCoord }, normal{ normal }, tangent{ tangent }, binormal{ binormal }
	{

	}

	Vertex(const Vector3& position,
		const Vector2& texCoord,
		const Vector3& normal,
		const Vector3& tangent,
		const Vector3& binormal) noexcept
		: position{ position }, texCoord{ texCoord }, normal{ normal }, tangent{ tangent }, binormal{ binormal }
	{

	}
};

struct PosVertex
{
	DirectX::SimpleMath::Vector3 position;
};

struct BoneWeightVertex
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

	BoneWeightVertex() noexcept = default;

	BoneWeightVertex(const float* position,
		const float* texCoord,
		const float* normal,
		const float* tangent,
		const float* binormal) noexcept
		: position{ position }, texCoord{ texCoord }, normal{ normal }, tangent{ tangent }, binormal{ binormal }
	{

	}

	BoneWeightVertex(const Vector3& position,
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
};