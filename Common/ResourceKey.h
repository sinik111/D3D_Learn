#pragma once

#include <string>

#include "Vertex.h"

struct VertexBufferKey
{
	std::wstring filePath;
	VertexFormat format;

	bool operator==(const VertexBufferKey& other) const
	{
		return filePath == other.filePath && format == other.format;
	}
};

namespace std
{
	inline void HashCombine(size_t& seed, size_t hashValue)
	{
		seed ^= hashValue + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template <>
	struct hash<VertexFormat>
	{
		size_t operator()(const VertexFormat& format) const
		{
			return hash<size_t>()(static_cast<size_t>(format));
		}
	};

	template <>
	struct hash<VertexBufferKey>
	{
		size_t operator()(const VertexBufferKey& key) const
		{
			size_t seed = 0;

			HashCombine(seed, hash<std::wstring>()(key.filePath));
			HashCombine(seed, hash<VertexFormat>()(key.format));

			return seed;
		}
	};
}