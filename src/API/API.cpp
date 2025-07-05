#include <windows.h>
#include <iostream>
#include <shlobj.h>

constexpr UINT ID_VIEW_SHOWDESKTOPICONS = 0x7402;
bool show = true;

void SetDesktopIconsVisibility(BOOL show)
{
    SHELLSTATE shellState = {};
    SHGetSetSettings(&shellState, SSF_HIDEICONS, FALSE);
    shellState.fHideIcons = !show;
    SHGetSetSettings(&shellState, SSF_HIDEICONS, TRUE);
    HWND progman = FindWindow(L"Progman", nullptr);
    if (progman)
        PostMessage(progman, WM_COMMAND, 41504, 0);
}