#include <windows.h>

#include "../Common/Helper.h"

#include "SkinningAnimationApp.h"

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
#ifdef _DEBUG
	LeakCheck lc;
#endif // _DEBUG

	SkinningAnimationApp app;

	app.Initialize();
	app.Run();
	app.Shutdown();
}