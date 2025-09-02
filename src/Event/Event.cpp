#include <chrono>

#include "Event.h"
#include "../main.h"
#include "../Core/Core.h"
#include "../WebView/WebView.h"
#include "../Watchdog/Watchdog.h"
#include "../TrayIcon/TrayIcon.h"

static HHOOK g_mouseHook = nullptr;
HHOOK g_keyboardHook = nullptr;
bool waiting_for_first_click = true;
bool left_mouse_down = false;
bool right_mouse_down = false;
bool middle_mouse_down = false;

void FirstClickMessage()
{
    ShowTrayNotification(L"Tip", L"To drag on your desktop without the select box showing, double click and drag instead. (Or triple tap and drag for trackpads.) Some mods hide it by default.");
}

HWND working_hwnd;
bool working_clicked = false;

LRESULT CALLBACK MouseEventProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode != HC_ACTION || GetPref(Pref::DisableMouseInput))
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);

    // static auto lastEventTime = std::chrono::steady_clock::now();
    // constexpr auto debounceInterval = std::chrono::milliseconds(10);
    // auto now = std::chrono::steady_clock::now();
    // if (now - lastEventTime < debounceInterval)
    // {
    //     return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    // }
    // lastEventTime = now;

    const auto *mouse = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
    POINT screenMouse = mouse->pt;

    HWND hoverHwnd = WindowFromPoint(screenMouse);
    wchar_t className[256] = {};
    GetClassNameW(hoverHwnd, className, 256);
    bool not_over_wallpaper = wcscmp(className, L"SysListView32") != 0 && wcscmp(className, L"SHELLDLL_DefView") != 0 && wcscmp(className, L"Progman") != 0;
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
            if (working_hwnd != hwnd)
            {
                working_hwnd = hwnd;
                working_clicked = false;
            }
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
        if (working_clicked)
        {
            if (left_mouse_down)
                data->compController->SendMouseInput(
                    COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_UP,
                    COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_LEFT_BUTTON,
                    0,
                    clientMouse);
            else if (right_mouse_down)
                data->compController->SendMouseInput(
                    COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_UP,
                    COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_RIGHT_BUTTON,
                    0,
                    clientMouse);
            else if (middle_mouse_down)
                data->compController->SendMouseInput(
                    COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_UP,
                    COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_MIDDLE_BUTTON,
                    0,
                    clientMouse);
        }
        // data->compController->SendMouseInput(
        //     COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE,
        //     COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE,
        //     0,
        //     POINT{10000, 10000});

        // COMMENT LINE BELOW TO ALLOW HOVER FROM ANYWHERE
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
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
        left_mouse_down = true;
        if (waiting_for_first_click && !not_over_wallpaper)
        {
            FirstClickMessage();
            waiting_for_first_click = false;
        }
        break;
    case WM_LBUTTONUP:
        kind = COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_UP;
        left_mouse_down = false;
        break;
    case WM_RBUTTONDOWN:
        kind = COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_DOWN;
        right_mouse_down = true;
        break;
    case WM_RBUTTONUP:
        kind = COREWEBVIEW2_MOUSE_EVENT_KIND_RIGHT_BUTTON_UP;
        right_mouse_down = false;
        break;
    case WM_MBUTTONDOWN:
        kind = COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_DOWN;
        middle_mouse_down = true;
        break;
    case WM_MBUTTONUP:
        kind = COREWEBVIEW2_MOUSE_EVENT_KIND_MIDDLE_BUTTON_UP;
        middle_mouse_down = false;
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
        valid = false; // logic for allowing hover events from anywhere

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
    if (data->registerInput)
    {
        data->compController->SendMouseInput(kind, vk, mouseData, clientMouse);
        if (kind != COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE)
        {
            working_clicked = true;
            FixWallpaperOrder(hwnd);
        }
    }

    return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK KeyboardEventProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    static auto lastEventTime = std::chrono::steady_clock::now();
    constexpr auto debounceInterval = std::chrono::milliseconds(10);
    if (nCode != HC_ACTION)
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    wprintf(L"EVENT INIT\n");

    auto now = std::chrono::steady_clock::now();
    if (now - lastEventTime < debounceInterval)
    {
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
    }
    lastEventTime = now;

    const auto *mouse = reinterpret_cast<MSLLHOOKSTRUCT *>(lParam);
    POINT screenMouse = mouse->pt;

    HWND focusHwnd = GetForegroundWindow();
    wchar_t className[256] = {};
    GetClassNameW(focusHwnd, className, 256);
    if (wcscmp(className, L"Progman") != 0)
        return CallNextHookEx(g_mouseHook, nCode, wParam, lParam);
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
        return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);

    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!data || !data->webview)
        return CallNextHookEx(g_keyboardHook, nCode, wParam, lParam);

    KBDLLHOOKSTRUCT *kb = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
    UINT vkCode = kb->vkCode;

    std::wstring eventType;
    switch (wParam)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        eventType = L"keydown";
        break;
    case WM_KEYUP:
    case WM_SYSKEYUP:
        eventType = L"keyup";
        break;
    default:
        eventType = L"";
        break;
    }

    if (!eventType.empty())
    {
        wprintf(L"SENDING EVENT vk=%u\n", vkCode);

        SetFocus(hwnd);
        data->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);

        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = vkCode;
        if (eventType == L"keyup")
            input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));

        BYTE keyboardState[256];
        GetKeyboardState(keyboardState);
        WCHAR buff[5] = {};
        int rc = ToUnicode(vkCode, kb->scanCode, keyboardState, buff, 4, 0);

        if (rc > 0 && wParam == WM_KEYDOWN)
        {
            std::wstring text(buff, rc);
            UINT vk = kb->vkCode;

            std::wstring js = LR"(
(function(key, vk, eventType) {
  const el = document.activeElement;

  function insertText(txt) {
    if (el && (el.tagName === 'INPUT' || el.tagName === 'TEXTAREA')) {
      const start = el.selectionStart;
      const end = el.selectionEnd;
      const oldValue = el.value;
      el.value = oldValue.slice(0, start) + txt + oldValue.slice(end);
      el.selectionStart = el.selectionEnd = start + txt.length;
      el.dispatchEvent(new Event('input', { bubbles: true }));
    } else if (el && el.isContentEditable) {
      const sel = window.getSelection();
      if (!sel.rangeCount) return;
      const range = sel.getRangeAt(0);
      range.deleteContents();
      range.insertNode(document.createTextNode(txt));
      range.collapse(false);
      el.dispatchEvent(new Event('input', { bubbles: true }));
    }
  }

  switch (vk) {
    case 8: // Backspace
      if (el && el.value !== undefined) {
        const start = el.selectionStart;
        const end = el.selectionEnd;
        if (start === end && start > 0) {
          el.value = el.value.slice(0, start - 1) + el.value.slice(end);
          el.selectionStart = el.selectionEnd = start - 1;
        } else {
          el.value = el.value.slice(0, start) + el.value.slice(end);
          el.selectionStart = el.selectionEnd = start;
        }
        el.dispatchEvent(new Event('input', { bubbles: true }));
      } else if (el && el.isContentEditable) {
        const sel = window.getSelection();
        if (!sel.rangeCount) break;
        const range = sel.getRangeAt(0);
        if (range.collapsed && range.startOffset > 0) {
          range.setStart(range.startContainer, range.startOffset - 1);
        }
        range.deleteContents();
      }
      break;
    case 13: insertText("\n"); break;
    case 46: // Delete
      if (el && el.value !== undefined) {
        const start = el.selectionStart;
        const end = el.selectionEnd;
        if (start === end && start < el.value.length) {
          el.value = el.value.slice(0, start) + el.value.slice(end + 1);
          el.selectionStart = el.selectionEnd = start;
        } else {
          el.value = el.value.slice(0, start) + el.value.slice(end);
          el.selectionStart = el.selectionEnd = start;
        }
        el.dispatchEvent(new Event('input', { bubbles: true }));
      } else if (el && el.isContentEditable) {
        const sel = window.getSelection();
        if (!sel.rangeCount) break;
        const range = sel.getRangeAt(0);
        if (range.collapsed && range.startOffset < range.startContainer.length) {
          range.setEnd(range.endContainer, range.endOffset + 1);
        }
        range.deleteContents();
      }
      break;
    default:
      if (key.length > 0) insertText(key);
      break;
  }

  const ev = new KeyboardEvent(eventType, {
    key: key || '',
    keyCode: vk,
    which: vk,
    code: 'Key' + String.fromCharCode(vk),
    bubbles: true,
    cancelable: true,
    view: window
  });
  console.log('dispatching', ev);

  if (el) el.dispatchEvent(ev);
//   document.dispatchEvent(ev);
  window.dispatchEvent(ev);

})(')";

            js += text;
            js += L"', " + std::to_wstring(vk);
            js += L", '" + eventType + L"')";

            data->webview->ExecuteScript(js.c_str(), nullptr);
        }
    }
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
