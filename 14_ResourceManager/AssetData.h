#pragma once

#include <string>

class AssetData
{
protected:
    std::wstring m_name;

public:
    virtual ~AssetData() = default;
};