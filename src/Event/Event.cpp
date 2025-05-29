#include "Event.h"
#include "../Core/Core.h"
#include "../Dispatch/Dispatch.h"

static HHOOK g_mouseHook = nullptr;
HHOOK g_keyboardHook = nullptr;

LRESULT CALLBACK MouseEventProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    auto *mouse = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
    if (wParam == WM_MOUSEMOVE)
    {
        for (auto &mw : ms)
        {
            if (IsWindow(mw.hwnd))
            {
                RECT rc;
                if (GetWindowRect(mw.hwnd, &rc) && PtInRect(&rc, mouse->pt))
                {
                    HWND targetHwnd = mw.hwnd;
                    POINT screenPos = mouse->pt;
                    RECT rc;
                    if (!GetWindowRect(targetHwnd, &rc))
                        return;
                    int relX = screenPos.x - rc.left;
                    int relY = screenPos.y - rc.top;
                    static POINT lastRel = {-1, -1};
                    if (lastRel.x != -1 && lastRel.y != -1)
                    {
                        int dx = relX - lastRel.x;
                        int dy = relY - lastRel.y;

                        if (dx != 0 || dy != 0)
                        {
                            INPUT input = {};
                            input.type = INPUT_MOUSE;
                            input.mi.dwFlags = MOUSEEVENTF_MOVE;
                            input.mi.dx = dx;
                            input.mi.dy = dy;

                            SendInput(1, &input, sizeof(INPUT));
                        }
                    }

                    lastRel = {relX, relY};
                    SetFocus(targetHwnd);
                    break;
                }
            }
        }
    }

    // if (nCode == HC_ACTION)
    // {
    //     auto *mouse = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
    //     POINT screenMouse = mouse->pt;
    //     HWND hwnd;
    //     for (auto &mw : ms)
    //     {
    //         RECT rc;
    //         if (IsWindow(mw.hwnd))
    //         {
    //             if (GetWindowRect(mw.hwnd, &rc))
    //             {
    //                 if (PtInRect(&rc, screenMouse))
    //                 {
    //                     hwnd = mw.hwnd;
    //                 }
    //             }
    //         }
    //     }
    //     if (!hwnd)
    //         return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    //     POINT clientMouse = screenMouse;
    //     ::ScreenToClient(hwnd, &clientMouse);
    //     WPARAM wmMessage = 0;
    //     WPARAM wmFlags = 0;
    //     LPARAM wmLParam = MAKELPARAM(clientMouse.x, clientMouse.y);
    //     switch (wParam)
    //     {
    //     case WM_MOUSEMOVE:
    //         wmMessage = WM_MOUSEMOVE;
    //         break;

    //     case WM_LBUTTONDOWN:
    //     case WM_LBUTTONUP:
    //         wmMessage = wParam;
    //         wmFlags = (wParam == WM_LBUTTONDOWN) ? MK_LBUTTON : 0;
    //         break;

    //     case WM_RBUTTONDOWN:
    //     case WM_RBUTTONUP:
    //         wmMessage = wParam;
    //         wmFlags = (wParam == WM_RBUTTONDOWN) ? MK_RBUTTON : 0;
    //         break;

    //     case WM_MBUTTONDOWN:
    //     case WM_MBUTTONUP:
    //         wmMessage = wParam;
    //         wmFlags = (wParam == WM_MBUTTONDOWN) ? MK_MBUTTON : 0;
    //         break;

    //     case WM_XBUTTONDOWN:
    //     case WM_XBUTTONUP:
    //     {
    //         wmMessage = wParam;
    //         WORD xbutton = HIWORD(mouse->mouseData);
    //         wmFlags = (xbutton == XBUTTON1) ? MK_XBUTTON1 : MK_XBUTTON2;
    //         break;
    //     }

    //     case WM_MOUSEWHEEL:
    //         wmMessage = WM_MOUSEWHEEL;
    //         wmFlags = mouse->mouseData;
    //         break;

    //     case WM_MOUSEHWHEEL:
    //         wmMessage = WM_MOUSEHWHEEL;
    //         wmFlags = mouse->mouseData;
    //         break;
    //     }
    //     if (wmMessage)
    //     {
    //         PostMessage(hwnd, wmMessage, wmFlags, wmLParam);
    //     }
    // }
    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardEventProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    // if (nCode == HC_ACTION)
    // {
    //     auto *kbd = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

    //     UINT msg = 0;
    //     switch (wParam)
    //     {
    //     case WM_KEYDOWN:
    //     case WM_SYSKEYDOWN:
    //         msg = WM_KEYDOWN;
    //         break;
    //     case WM_KEYUP:
    //     case WM_SYSKEYUP:
    //         msg = WM_KEYUP;
    //         break;
    //     }

    //     if (msg != 0)
    //     {
    //         for (auto &mw : ms)
    //         {
    //             if (IsWindow(mw.hwnd))
    //             {
    //                 PostMessage(mw.hwnd, msg, kbd->vkCode, kbd->scanCode << 16);
    //             }
    //         }
    //     }
    // }
    return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}

void InstallEventHooks()
{
    if (!g_mouseHook)
        g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseEventProc, nullptr, 0);
    if (!g_keyboardHook)
        g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardEventProc, nullptr, 0);
}

void UninstallEventHooks()
{
    if (g_mouseHook)
    {
        UnhookWindowsHookEx(g_mouseHook);
        g_mouseHook = nullptr;
    }
    if (g_keyboardHook)
    {
        UnhookWindowsHookEx(g_keyboardHook);
        g_keyboardHook = nullptr;
    }
}