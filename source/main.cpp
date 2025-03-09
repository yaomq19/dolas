#include <Windows.h>
#include "Function/FrameWork/Dolas.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
    if(Dolas::g_dolas_context.Initialize(hInstance, nCmdShow, 800, 600, L"Dolas Engine"))
    {
        Dolas::g_dolas_context.Run();
    }
    Dolas::g_dolas_context.Shutdown();
    return 0;
}