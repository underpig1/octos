#define UNICODE 1

#include <windows.h>
#include <tchar.h>

VOID HideTB(BOOL state = TRUE)
{
    HWND hShellWnd = FindWindow(L"Shell_TrayWnd", NULL);
    ShowWindow(hShellWnd, state ? SW_HIDE : SW_SHOW);
}