#include <windows.h>

#include "../Common/Helper.h"

#include "PBRApp.h"

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
#ifdef _DEBUG
	LeakCheck lc;
#endif // _DEBUG

	PBRApp app;

	int argc;
	// 1. 명령줄 문자열을 인자 배열(argv)로 분리
	LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);
	if (argv != NULL)
	{
		for (int i = 0; i < argc; i++)
		{
			if (_wcsicmp(argv[i], L"-LDR") == 0)
			{
				app.SetForceLDR(true);
			}
		}

		// 3. 메모리 해제
		LocalFree(argv);
	}

	app.Initialize();
	app.Run();
	app.Shutdown();
}