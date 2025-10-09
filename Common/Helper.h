#pragma once

#include <string>

// std::string�� std::wstring���� ��ȯ (Windows API ���)
std::wstring ToWideCharStr(const std::string& utf8_str);

// std::wstring�� std::string���� ��ȯ (Windows API ���)
std::string ToMultibyteStr(const std::wstring& wide_str);