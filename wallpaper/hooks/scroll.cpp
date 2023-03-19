#include <windows.h>
#include <stdio.h>

int scrollDelta = 0;

LRESULT CALLBACK mouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    MSLLHOOKSTRUCT *pMouseStruct = (MSLLHOOKSTRUCT *)lParam;

    if (wParam == WM_MOUSEWHEEL)
    {
        scrollDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
    HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, mouseHookProc, hInstance, GetCurrentThreadId());
    return 0;
}