#include "Helper.h"

#include <Windows.h>
#include <random>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#include <dxgidebug.h>
#include <dxgi1_3.h>

#pragma comment(lib, "dxguid.lib")
#endif // _DEBUG

#include <directxtk/SimpleMath.h>

#include "MyTime.h"

namespace
{
	MyTime::TimePoint g_lastTimestamp = MyTime::Clock::now();
	int g_frameCount;
	int g_lastFPS;

	std::mt19937 g_gen{ std::random_device{}() };
}

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

void UpdateFPS()
{
	++g_frameCount;

	if (MyTime::GetElapsedSeconds(g_lastTimestamp) > 1.0f)
	{
		g_lastTimestamp = MyTime::GetAccumulatedTime(g_lastTimestamp, 1);

		g_lastFPS = g_frameCount;

		g_frameCount = 0;
	}
}

int GetLastFPS()
{
	return g_lastFPS;
}

float RandomFloat(float min, float max)
{
	return std::uniform_real_distribution<float>(min, max)(g_gen);
}

void Log(const std::string& log)
{
	OutputDebugStringA((log + '\n').c_str());
}

float ToRadian(float degree)
{
	return DirectX::XMConvertToRadians(degree);
}

float ToDegree(float radian)
{
	return DirectX::XMConvertToDegrees(radian);
}