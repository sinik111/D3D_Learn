#pragma once

#include <string>
#include <type_traits>
#include <memory>

class AssetData
{
protected:
    std::wstring m_name;

public:
    virtual ~AssetData() = default;
};

template <typename T>
std::shared_ptr<T> SafeCast(std::shared_ptr<AssetData> basePtr)
{
    static_assert(std::is_base_of_v<AssetData, T>, "T must be derived from AssetData.");

    return std::dynamic_pointer_cast<T>(basePtr);
}