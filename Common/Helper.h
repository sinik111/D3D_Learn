#pragma once

#include <string>

// std::string을 std::wstring으로 변환 (Windows API 기반)
std::wstring ToWideCharStr(const std::string& utf8_str);

// std::wstring을 std::string으로 변환 (Windows API 기반)
std::string ToMultibyteStr(const std::wstring& wide_str);