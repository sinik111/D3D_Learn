#include <windows.h>

#include "FBXLoadingApp.h"

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    FBXLoadingApp app;

    app.Initialize();

    app.Run();

    app.Shutdown();
}