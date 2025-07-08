#include <windows.h>
#include <iostream>
#include <shlobj.h>

#include "API.h"
#include "../Bridge/Bridge.h"
#include "../Storage/Storage.h"
#include "../Core/Core.h"
#include "../WebView/WebView.h"
#include "Media.h"

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

bool IsLightTheme()
{
    HKEY hKey;
    DWORD value = 1;
    DWORD valueSize = sizeof(value);
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0,
                      KEY_READ,
                      &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueExW(hKey,
                             L"AppsUseLightTheme",
                             nullptr,
                             nullptr,
                             reinterpret_cast<LPBYTE>(&value),
                             &valueSize) != ERROR_SUCCESS)
            value = 1;

        RegCloseKey(hKey);
    }
    return value != 0;
}

void PropogateToAppHwnd(HWND targetHwnd, json msg)
{
    for (std::wstring monitorId : FindMonitorIdsByHwnd(targetHwnd))
    {
        msg["monitor-id"] = to_string(monitorId);
        DispatchJson(to_wstring(msg.dump()));
    }
}

auto GetWebViewInstance(HWND hwnd)
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (data && data->webview)
        return data->webview;
}

void RespondToHwnd(HWND hwnd, json msg, json data, bool mainThread)
{
    msg["type"] = "response";
    msg["data"] = data;
    if (mainThread)
        PostMessage(hwnd, WM_USER + 5, 0, (LPARAM) new std::wstring(to_wstring(msg.dump())));
    else
        DispatchToHwnd(hwnd, to_wstring(msg.dump()));
}

void SendEventToHwnd(HWND hwnd, json msg, json data, bool mainThread)
{
    msg["type"] = "event";
    msg["data"] = data;
    if (mainThread)
        PostMessage(hwnd, WM_USER + 5, 0, (LPARAM) new std::wstring(to_wstring(msg.dump())));
    else
        DispatchToHwnd(hwnd, to_wstring(msg.dump()));
}

void HandleRequest(json msg, HWND hwnd)
{
    if (msg.contains("requestId") && msg.contains("requestType") &&
        msg["requestId"].is_string() && msg["requestId"].is_string())
    {
        std::string type = msg["requestType"];
        if (type == "options" || false) // handled by app_hwnd
        {
            PropogateToAppHwnd(hwnd, msg);
        }
        else // handled directly
        {
            if (type == "source")
            {
                auto webview = GetWebViewInstance(hwnd);
                if (webview)
                {
                    LPWSTR uri;
                    webview->get_Source(&uri);
                    std::string uriStr = to_string(uri);
                    RespondToHwnd(hwnd, msg, uriStr);
                }
            }
            else if (type == "media-props")
            {
                HandleMediaPropertiesRequest(hwnd, msg);
            }
            else if (type == "playback-info")
            {
                HandlePlaybackInfoRequest(hwnd, msg);
            }
            else if (type == "timeline-props")
            {
                HandleTimelinePropertiesRequest(hwnd, msg);
            }
            else if (type == "thumbnail")
                HandleThumbnailRequest(hwnd, msg);
            else if (type == "is-light-theme")
            {
                bool light = IsLightTheme();
                RespondToHwnd(hwnd, msg, light);
            }
            else if (type == "visibility")
            {
                WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                if (data)
                {
                    if (data->hidden)
                        RespondToHwnd(hwnd, msg, false);
                    else
                        RespondToHwnd(hwnd, msg, true);
                }
            }
        }
    }
}

void HandleCommand(json msg, HWND hwnd)
{
    if (msg.contains("commandType") && msg["commandType"].is_string())
    {
        std::string type = msg["commandType"];
        if (type == "set-option" || false)
        {
            PropogateToAppHwnd(hwnd, msg);
        }
        else if (type == "open-dev-tools")
        {
            auto webview = GetWebViewInstance(hwnd);
            if (webview)
                webview->OpenDevToolsWindow();
        }
        else if (type == "media")
        {
            if (msg.contains("data") && msg["data"].is_object())
            {
                json data = msg["data"];
                if (data.contains("cmd") && data["cmd"].is_string())
                {
                    std::string cmd = data["cmd"];
                    if (cmd == "set-playback-position")
                    {
                        if (data.contains("position") && data["position"].is_number_integer())
                            SetPlaybackPosition(data["position"]);
                    }
                    else SendMediaCommand(cmd);
                }
            }
        }
        else if (type == "set-visible")
            SetWebViewVisibility(hwnd, true);
        else if (type == "set-hidden")
            SetWebViewVisibility(hwnd, false);
        else if (type == "set-desktop-icons-visible")
            SetDesktopIconsVisibility(true);
        else if (type == "set-desktop-icons-hidden")
            SetDesktopIconsVisibility(false);
    }
}

void HandleSubscription(json msg, HWND hwnd, bool sub)
{
    if (msg.contains("eventType") && msg["eventType"].is_string())
    {
        std::string type = msg["eventType"];
        if (type == "media-change")
        {
            if (sub)
                AddMediaSubscription(hwnd, msg);
            else
                RemoveMediaSubscription(hwnd);
        }
        else if (type == "playback-change")
        {
            if (sub)
                AddPlaybackSubscription(hwnd, msg);
            else
                RemovePlaybackSubscription(hwnd);
        }
        else if (type == "timeline-change")
        {
            if (sub)
                AddTimelineSubscription(hwnd, msg);
            else
                RemoveTimelineSubscription(hwnd);
        }
    }
}