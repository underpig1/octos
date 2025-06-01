#include "Event.h"
#include "../Core/Core.h"
#include "../WebView/WebView.h"

static HHOOK g_mouseHook = nullptr;
HHOOK g_keyboardHook = nullptr;

LRESULT CALLBACK MouseEventProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        auto *mouse = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
        POINT screenMouse = mouse->pt;

        // find window under mouse
        HWND hwnd = nullptr;
        for (auto &mw : ms)
        {
            RECT rc;
            if (IsWindow(mw.hwnd) && GetWindowRect(mw.hwnd, &rc) && PtInRect(&rc, screenMouse))
            {
                hwnd = mw.hwnd;
                break;
            }
        }
        if (!hwnd)
            return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);

        // extract data
        WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (!data || !data->compController)
            return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);

        // Convert screen to client coordinates
        POINT clientMouse = screenMouse;
        ScreenToClient(hwnd, &clientMouse);

        // determine type of input
        COREWEBVIEW2_MOUSE_EVENT_KIND kind;
        bool valid = true;
        switch (wParam)
        {
        case WM_MOUSEMOVE:
            kind = COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE;
            break;
        case WM_LBUTTONDOWN:
            kind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_DOWN;
            break;
        case WM_LBUTTONUP:
            kind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_UP;
            break;
        case WM_RBUTTONDOWN:
            kind = COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_DOWN;
            break;
        case WM_RBUTTONUP:
            kind = COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_UP;
            break;
        case WM_MBUTTONDOWN:
            kind = COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_DOWN;
            break;
        case WM_MBUTTONUP:
            kind = COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_UP;
            break;
        case WM_MOUSEWHEEL:
            kind = COREWEBVIEW2_MOUSE_EVENT_KIND_WHEEL;
            break;
        case WM_MOUSEHWHEEL:
            kind = COREWEBVIEW2_MOUSE_EVENT_KIND_HORIZONTAL_WHEEL;
            break;
        default:
            valid = false;
            break;
        }
        if (!valid)
            return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);

        // build virtual keys
        COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS vkFlags = COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE;
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
            vkFlags |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_LEFT_BUTTON;
        if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
            vkFlags |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_RIGHT_BUTTON;
        if (GetAsyncKeyState(VK_MBUTTON) & 0x8000)
            vkFlags |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_MIDDLE_BUTTON;
        if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            vkFlags |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_CONTROL;
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
            vkFlags |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_SHIFT;
        if ((wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP))
        {
            WORD xbutton = HIWORD(mouse->mouseData);
            if (xbutton == XBUTTON1)
                vkFlags |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_X_BUTTON1;
            else if (xbutton == XBUTTON2)
                vkFlags |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_X_BUTTON2;
        }

        // handle wheel or xbutton
        UINT32 mouseData = 0;
        if (wParam == WM_MOUSEWHEEL || wParam == WM_MOUSEHWHEEL)
            mouseData = GET_WHEEL_DELTA_WPARAM(mouse->mouseData);
        else if (wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP)
            mouseData = GET_XBUTTON_WPARAM(mouse->mouseData);

        // throttle move events
        static DWORD lastMoveTime = 0;
        static POINT lastMovePoint = {-1, -1};
        if (kind == COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE)
        {
            DWORD now = GetTickCount();
            if ((now - lastMoveTime < 8) && clientMouse.x == lastMovePoint.x && clientMouse.y == lastMovePoint.y)
            {
                return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
            }
            lastMoveTime = now;
            lastMovePoint = clientMouse;
        }

        // send mouse data
        data->compController->SendMouseInput(kind, vkFlags, mouseData, clientMouse);
    }
    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardEventProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);
}

void InstallEventHooks()
{
    if (!g_mouseHook)
        g_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseEventProc, nullptr, 0);
    // if (!g_keyboardHook)
    //     g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardEventProc, nullptr, 0);
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
