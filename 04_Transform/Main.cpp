#include <windows.h>

#include "TransformApp.h"

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    TransformApp app;

    app.Initialize();

    app.Run();

    app.Shutdown();
}