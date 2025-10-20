#pragma once

#include <directxtk/SimpleMath.h>

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