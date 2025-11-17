#pragma once

#include <memory>

class D3DResource
{
public:
	virtual ~D3DResource() = default;
};

template <typename T>
std::shared_ptr<T> SafeCast(std::shared_ptr<D3DResource> basePtr)
{
    static_assert(std::is_base_of_v<D3DResource, T>, "T must be derived from D3DResource.");

    return std::dynamic_pointer_cast<T>(basePtr);
}