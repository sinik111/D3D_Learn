#pragma once

#include <string>
#include <sstream>

// std::string을 std::wstring으로 변환 (Windows API 기반)
std::wstring ToWideCharStr(const std::string& multibyteStr);
std::wstring ToWideCharStr(const char* multibyteStr);

// std::wstring을 std::string으로 변환 (Windows API 기반)
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

void __Log(const std::string& log);
void Log(const std::string& log);

template<typename T>
inline void LogImpl(std::ostringstream& oss, T&& arg)
{
    oss << std::forward<T>(arg);
}

template<typename T, typename...Args>
inline void LogImpl(std::ostringstream& oss, T&& arg, Args&&...args)
{
    oss << std::forward<T>(arg);

    LogImpl(oss, std::forward<Args>(args)...);
}

template<typename...Args>
inline void Log(Args&&...args)
{
    std::ostringstream oss;

    LogImpl(oss, std::forward<Args>(args)...);

    oss << '\n';

    __Log(oss.str());
}

float ToRadian(float degree);
float ToDegree(float radian);