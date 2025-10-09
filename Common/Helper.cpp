#include "Helper.h"

#include <Windows.h>
#include <vector>

std::wstring ToWideCharStr(const std::string& multibyteStr)
{
	if (multibyteStr.empty())
	{
		return L"";
	}

	int count = MultiByteToWideChar(
		CP_UTF8,
		0,
		multibyteStr.c_str(),
		(int)multibyteStr.length(),
		nullptr,
		0
	);

	std::vector<wchar_t> wide(count);

	MultiByteToWideChar(
		CP_UTF8,
		0,
		multibyteStr.c_str(),
		(int)multibyteStr.length(),
		wide.data(),
		count
	);

	return std::wstring(wide.data(), wide.size());
}

std::string ToMultibyteStr(const std::wstring& wideCharStr)
{
	if (wideCharStr.empty())
	{
		return "";
	}

	int count = WideCharToMultiByte(
		CP_UTF8,
		0,
		wideCharStr.c_str(),
		(int)wideCharStr.length(),
		nullptr,
		0,
		nullptr,
		nullptr
	);

	std::vector<char> multibyte(count);

	WideCharToMultiByte(
		CP_UTF8,
		0,
		wideCharStr.c_str(),
		(int)wideCharStr.length(),
		multibyte.data(),
		count,
		nullptr,
		nullptr
	);

	return std::string(multibyte.data(), multibyte.size());
}
