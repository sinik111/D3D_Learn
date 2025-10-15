#pragma once

#include <string>

// std::string�� std::wstring���� ��ȯ (Windows API ���)
std::wstring ToWideCharStr(const std::string& multibyteStr);

// std::wstring�� std::string���� ��ȯ (Windows API ���)
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