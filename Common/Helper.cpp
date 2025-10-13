#include "Helper.h"

#include <Windows.h>
#include <vector>

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#include <dxgidebug.h>
#include <dxgi1_3.h>

#pragma comment(lib, "dxguid.lib")

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
