#include "Event.h"
#include "../Core/Core.h"

static HHOOK g_mouseHook = nullptr;
HHOOK g_keyboardHook = nullptr;

LRESULT CALLBACK MouseEventProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    //     if (m_dcompDevice || m_wincompCompositor)
    //     {
    //         POINT point;
    //         POINTSTOPOINT(point, lParam);
    //         if (message == WM_MOUSEWHEEL ||
    //             message == WM_MOUSEHWHEEL || message == WM_NCRBUTTONDOWN || message == WM_NCRBUTTONUP)
    //         {
    //             // Mouse wheel messages are delivered in screen coordinates.
    //             // SendMouseInput expects client coordinates for the WebView, so convert
    //             // the point from screen to client.
    //             ::ScreenToClient(m_appWindow->GetMainWindow(), &point);
    //         }
    //         // Send the message to the WebView if the mouse location is inside the
    //         // bounds of the WebView, if the message is telling the WebView the
    //         // mouse has left the client area, or if we are currently capturing
    //         // mouse events.
    //         bool isMouseInWebView = PtInRect(&m_webViewBounds, point);
    //         if (isMouseInWebView || message == WM_MOUSELEAVE || m_isCapturingMouse)
    //         {
    //             DWORD mouseData = 0;

    //             switch (message)
    //             {
    //             case WM_MOUSEWHEEL:
    //             case WM_MOUSEHWHEEL:
    //                 mouseData = GET_WHEEL_DELTA_WPARAM(wParam);
    //                 break;
    //             case WM_XBUTTONDBLCLK:
    //             case WM_XBUTTONDOWN:
    //             case WM_XBUTTONUP:
    //                 mouseData = GET_XBUTTON_WPARAM(wParam);
    //                 break;
    //             case WM_MOUSEMOVE:
    //                 if (!m_isTrackingMouse)
    //                 {
    //                     // WebView needs to know when the mouse leaves the client area
    //                     // so that it can dismiss hover popups. TrackMouseEvent will
    //                     // provide a notification when the mouse leaves the client area.
    //                     TrackMouseEvents(TME_LEAVE);
    //                     m_isTrackingMouse = true;
    //                 }
    //                 break;
    //             case WM_MOUSELEAVE:
    //                 m_isTrackingMouse = false;
    //                 break;
    //             }
    //             if (message == WM_LBUTTONDOWN || message == WM_MBUTTONDOWN ||
    //                 message == WM_RBUTTONDOWN || message == WM_XBUTTONDOWN)
    //             {
    //                 if (isMouseInWebView && ::GetCapture() != hwnd->GetMainWindow())
    //                 {
    //                     m_isCapturingMouse = true;
    //                     ::SetCapture(hwnd->GetMainWindow());
    //                 }
    //             }
    //             else if (message == WM_LBUTTONUP || message == WM_MBUTTONUP ||
    //                      message == WM_RBUTTONUP || message == WM_XBUTTONUP)
    //             {
    //                 if (::GetCapture() == hwnd->GetMainWindow())
    //                 {
    //                     m_isCapturingMouse = false;
    //                     ::ReleaseCapture();
    //                 }
    //             }
    //             if (message != WM_MOUSELEAVE)
    //             {
    //                 point.x -= m_webViewBounds.left;
    //                 point.y -= m_webViewBounds.top;
    //             }

    //             CHECK_FAILURE(m_compositionController->SendMouseInput(
    //                 static_cast<COREWEBVIEW2_MOUSE_EVENT_KIND>(message),
    //                 static_cast<COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS>(GET_KEYSTATE_WPARAM(wParam)),
    //                 mouseData, point));
    //             return true;
    //         }
    //         else if (message == WM_MOUSEMOVE && m_isTrackingMouse)
    //         {
    //             // When the mouse moves outside of the WebView, but still inside the app
    //             // turn off mouse tracking and send the WebView a leave event.
    //             m_isTrackingMouse = false;
    //             TrackMouseEvents(TME_LEAVE | TME_CANCEL);
    //             OnMouseMessage(WM_MOUSELEAVE, 0, 0);
    //         }
    //     }
    //     return false;
    // }
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