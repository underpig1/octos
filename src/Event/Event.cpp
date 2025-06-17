#include "Event.h"
#include "../Core/Core.h"
#include "../WebView/WebView.h"
#include "../Watchdog/Watchdog.h"
#include <chrono>

static HHOOK g_mouseHook = nullptr;
HHOOK g_keyboardHook = nullptr;
bool just_released = false;

LRESULT CALLBACK MouseEventProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    static auto lastEventTime = std::chrono::steady_clock::now();
    constexpr auto debounceInterval = std::chrono::milliseconds(10);
    if (nCode != HC_ACTION)
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);

    auto now = std::chrono::steady_clock::now();
    if (now - lastEventTime < debounceInterval)
    {
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    }
    lastEventTime = now;

    const auto *mouse = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
    POINT screenMouse = mouse->pt;

    HWND hoverHwnd = WindowFromPoint(screenMouse);
    wchar_t className[256] = {};
    GetClassNameW(hoverHwnd, className, 256);
    bool not_over_wallpaper = wcscmp(className, L"SysListView32") != 0;
    // if (not_over_wallpaper && just_released)
    //     return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    // else if (just_released)
    //     just_released = false;

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

    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!data || !data->compController)
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);

    // Convert screen to client
    POINT clientMouse = screenMouse;
    ScreenToClient(hwnd, &clientMouse);

    if (not_over_wallpaper)
    {
        data->compController->SendMouseInput(
            COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_UP,
            COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_LEFT_BUTTON,
            0,
            clientMouse);
        data->compController->SendMouseInput(
            COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_UP,
            COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_RIGHT_BUTTON,
            0,
            clientMouse);
        data->compController->SendMouseInput(
            COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_UP,
            COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_MIDDLE_BUTTON,
            0,
            clientMouse);
        // data->compController->SendMouseInput(
        //     COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE,
        //     COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE,
        //     0,
        //     POINT{10000, 10000});
        just_released = true;
        // return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    }

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

    if (kind != COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE && valid && not_over_wallpaper)
        valid = false;

    if (!valid)
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);

    COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS vk = COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE;
    if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
        vk |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_LEFT_BUTTON;
    if (GetAsyncKeyState(VK_RBUTTON) & 0x8000)
        vk |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_RIGHT_BUTTON;
    if (GetAsyncKeyState(VK_MBUTTON) & 0x8000)
        vk |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_MIDDLE_BUTTON;
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
        vk |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_CONTROL;
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
        vk |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_SHIFT;

    if (wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP)
    {
        WORD xbtn = HIWORD(mouse->mouseData);
        if (xbtn == XBUTTON1)
            vk |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_X_BUTTON1;
        else if (xbtn == XBUTTON2)
            vk |= COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_X_BUTTON2;
    }

    UINT32 mouseData = 0;
    if (wParam == WM_MOUSEWHEEL || wParam == WM_MOUSEHWHEEL)
        mouseData = GET_WHEEL_DELTA_WPARAM(mouse->mouseData);
    else if (wParam == WM_XBUTTONDOWN || wParam == WM_XBUTTONUP)
        mouseData = GET_XBUTTON_WPARAM(mouse->mouseData);

    static DWORD lastMoveTime = 0;
    static POINT lastMovePt = {-1, -1};
    if (kind == COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE)
    {
        DWORD now = GetTickCount();
        if ((now - lastMoveTime < 8) && clientMouse.x == lastMovePt.x && clientMouse.y == lastMovePt.y)
            return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
        lastMoveTime = now;
        lastMovePt = clientMouse;
    }

    // send input only to our own windows
    data->compController->SendMouseInput(kind, vk, mouseData, clientMouse);
    if (kind != COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE)
        FixWallpaperOrder(hwnd);

    // Always pass the event along to avoid interfering with other apps
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
