#pragma once

#include <string>
#include <sstream>

// std::string�� std::wstring���� ��ȯ (Windows API ���)
std::wstring ToWideCharStr(const std::string& multibyteStr);
std::wstring ToWideCharStr(const char* multibyteStr);

// std::wstring�� std::string���� ��ȯ (Windows API ���)
std::string ToMultibyteStr(const std::wstring& wideCharStr);
std::string ToMultibyteStr(const wchar_t* wideCharStr);

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

void Log(const std::string& log);

template<typename T>
inline void LogImpl(std::ostringstream& oss, T arg)
{
    oss << arg;
}

template<typename T, typename...Args>
inline void LogImpl(std::ostringstream& oss, T arg, Args...args)
{
    oss << arg << ' ';

    LogImpl(oss, args...);
}

template<typename...Args>
inline void Log(Args...args)
{
    std::ostringstream oss;

    LogImpl(oss, args...);

    Log(oss.str());
}

float ToRadian(float degree);
float ToDegree(float radian);