#pragma once

#include <windows.h>
#include <string>

#include "CoInitializer.h"

class WinApp
{
protected:
	HWND m_hWnd = nullptr;
	HINSTANCE m_hInstance = nullptr;
	HICON m_hIcon = nullptr;
	HCURSOR m_hCursor = nullptr;
	HICON m_hIconSmall = nullptr;

	// default settings
	std::wstring m_className = L"DefaultClassName";
	std::wstring m_windowName = L"DefaultWindowName";

	UINT m_classStyle = CS_HREDRAW | CS_VREDRAW;
	DWORD m_windowStyle = WS_OVERLAPPEDWINDOW;
	int m_x = CW_USEDEFAULT;
	int m_y = CW_USEDEFAULT;
	int m_width = 800;
	int m_height = 600;

	CoInitializer m_coInitializer;

public:
	virtual ~WinApp() = default;

public:
	virtual void Initialize();
	void Shutdown();

	void Run();

protected:
	virtual void Update() = 0;
	virtual void Render() = 0;

protected:
	virtual void MessageProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	friend LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
};