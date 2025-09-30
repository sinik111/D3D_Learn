#include <windows.h>

#include "NormalMappingApp.h"

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    NormalMappingApp app;

    app.Initialize();

    app.Run();

    app.Shutdown();
}