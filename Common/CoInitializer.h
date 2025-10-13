#pragma once

#include <wrl/client.h>
#include <cassert>

class CoInitializer
{
//private: // 아무도 생성 못하게 생성자를 private으로 만듦
public:
	CoInitializer(DWORD dwCoInit = COINIT_APARTMENTTHREADED)
	{
		HRESULT hr = CoInitializeEx(nullptr, dwCoInit);
		assert(SUCCEEDED(hr) && "Com 초기화 실패");
	}

	~CoInitializer()
	{
		CoUninitialize();
	}

	CoInitializer(const CoInitializer&) = delete;
	CoInitializer& operator=(const CoInitializer&) = delete;

	//friend class WinApp; // WinApp에서만 생성할 수 있음 
};