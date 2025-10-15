#pragma once

#include <string>

// std::string을 std::wstring으로 변환 (Windows API 기반)
std::wstring ToWideCharStr(const std::string& multibyteStr);

// std::wstring을 std::string으로 변환 (Windows API 기반)
std::string ToMultibyteStr(const std::wstring& wideCharStr);

#ifdef _DEBUG
class LeakCheck
{
public:
	LeakCheck();
	~LeakCheck();
};
#endif // _DEBUG

void UpdateFPS();

int GetLastFPS();

float RandomFloat(float min, float max);