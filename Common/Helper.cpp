#include "Helper.h"

#include <Windows.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#include <dxgidebug.h>
#include <dxgi1_3.h>

#pragma comment(lib, "dxguid.lib")
#endif // _DEBUG

#include "MyTime.h"

std::wstring ToWideCharStr(const std::string& multibyteStr)
{
	if (multibyteStr.empty())
	{
		return std::wstring();
	}

	int count = MultiByteToWideChar(
		CP_UTF8,
		0,
		multibyteStr.c_str(),
		static_cast<int>(multibyteStr.size()),
		nullptr,
		0
	);

	std::wstring str(count, L'\0');

	MultiByteToWideChar(
		CP_UTF8,
		0,
		multibyteStr.c_str(),
		static_cast<int>(multibyteStr.size()),
		&str[0],
		count
	);

	return str;
}

std::string ToMultibyteStr(const std::wstring& wideCharStr)
{
	if (wideCharStr.empty())
	{
		return std::string();
	}

	int count = WideCharToMultiByte(
		CP_UTF8,
		0,
		wideCharStr.c_str(),
		static_cast<int>(wideCharStr.size()),
		nullptr,
		0,
		nullptr,
		nullptr
	);

	std::string str(count, '\0');

	WideCharToMultiByte(
		CP_UTF8,
		0,
		wideCharStr.c_str(),
		static_cast<int>(wideCharStr.size()),
		&str[0],
		count,
		nullptr,
		nullptr
	);

	return str;
}

static MyTime::TimePoint s_lastTimestamp = MyTime::Clock::now();
static int s_frameCount;
static int s_lastFPS;

void UpdateFPS()
{
	++s_frameCount;

	if (MyTime::GetElapsedSeconds(s_lastTimestamp) > 1.0f)
	{
		s_lastTimestamp = MyTime::GetAccumulatedTime(s_lastTimestamp, 1);

		s_lastFPS = s_frameCount;

		s_frameCount = 0;
	}
}

int GetLastFPS()
{
	return s_lastFPS;
}

#include <random>

static std::mt19937 gen{ std::random_device{}() };

float RandomFloat(float min, float max)
{
	return std::uniform_real_distribution<float>(min, max)(gen);
}

#ifdef _DEBUG
LeakCheck::LeakCheck()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}

LeakCheck::~LeakCheck()
{
	IDXGIDebug1* pDebug = nullptr;

	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
	{
		// 현재 살아있는 DXGI/D3D 객체 출력
		pDebug->ReportLiveObjects(
			DXGI_DEBUG_ALL,                 // 모든 DXGI/D3D 컴포넌트
			DXGI_DEBUG_RLO_ALL              // 전체 리포트 옵션
		);

		pDebug->Release();
	}
}
#endif // _DEBUG

void Log(const std::string& log)
{
	OutputDebugStringA((log + '\n').c_str());
}