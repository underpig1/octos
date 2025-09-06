#include <windows.h>
#include <iostream>
#include <shlobj.h>

#include "API.h"
#include "../Bridge/Bridge.h"
#include "../Storage/Storage.h"
#include "../Core/Core.h"
#include "../WebView/WebView.h"
#include "Media.h"
#include "Audio.h"

HWND GetSysListViewHwnd()
{
    HWND progman = FindWindow(L"Progman", nullptr);
    HWND listView = nullptr;
    HWND shellViewWin = FindWindowEx(progman, nullptr, L"SHELLDLL_DefView", nullptr);
    if (!shellViewWin)
    {
        HWND desktopHWND = nullptr;
        HWND workerW = nullptr;
        while ((workerW = FindWindowEx(nullptr, workerW, L"WorkerW", nullptr)) != nullptr)
        {
            shellViewWin = FindWindowEx(workerW, nullptr, L"SHELLDLL_DefView", nullptr);
            if (shellViewWin)
                break;
        }
    }
    if (shellViewWin)
        listView = FindWindowEx(shellViewWin, nullptr, L"SysListView32", L"FolderView");
    return listView;
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
    std::wstring monitorId = FindMonitorIdByHwnd(targetHwnd);
    msg["monitor-id"] = to_string(monitorId);
    DispatchJson(to_wstring(msg.dump()));
}

wil::com_ptr<ICoreWebView2> GetWebViewInstance(HWND hwnd)
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (data && data->webview)
        return data->webview;
    return {};
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
    wprintf(L"\n\n--- OK WE ARE SENDING ---\n\n");
    if (mainThread)
        PostMessage(hwnd, WM_USER + 5, 0, (LPARAM) new std::wstring(to_wstring(msg.dump())));
    else
        DispatchToHwnd(hwnd, to_wstring(msg.dump()));
}

void HandleRequest(json msg, HWND hwnd)
{
    if (msg.contains("requestId") && msg.contains("requestType") &&
        msg["requestId"].is_string() && msg["requestType"].is_string())
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
            else if (type == "is-theme-light")
            {
                bool light = IsLightTheme();
                RespondToHwnd(hwnd, msg, light);
            }
            else if (type == "visibility")
            {
                WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
                if (data && data->controller)
                {
                    BOOL visibility;
                    HRESULT hr = data->controller->get_IsVisible(&visibility);
                    if (SUCCEEDED(hr))
                    {
                        if (visibility)
                            RespondToHwnd(hwnd, msg, true);
                        else
                            RespondToHwnd(hwnd, msg, false);
                    }
                }
            }
            else if (type == "siblings")
            {
                std::vector<std::string> siblingIds8;
                std::vector<std::wstring> siblingIds16 = GetSiblingMonitorIds(hwnd);
                for (auto &sibling16 : siblingIds16)
                    siblingIds8.push_back(to_string(sibling16));
                RespondToHwnd(hwnd, msg, siblingIds8);
            }
            else if (type == "monitor-id")
            {
                std::wstring monitorId = FindMonitorIdByHwnd(hwnd);
                if (!monitorId.empty())
                    RespondToHwnd(hwnd, msg, to_string(monitorId));
            }
            else if (type == "desktop-icons-visibility")
            {
                HWND listView = GetSysListViewHwnd();
                if (listView != nullptr && IsWindow(listView))
                    RespondToHwnd(hwnd, msg, IsWindowVisible(listView) ? true : false);
            }
        }
    }
    else 
    {
        std::string type = msg["requestType"];
        wprintf(L"Unknown request type: %hs\n", type.c_str());
    }
}

void HandleCommand(json msg, HWND hwnd)
{
    if (msg.contains("commandType") && msg["commandType"].is_string())
    {
        std::string type = msg["commandType"];
        if (type == "set-option" || type == "send-to-monitor-hwnd")
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
                    else
                        SendMediaCommand(cmd);
                }
            }
        }
        else if (type == "set-visible")
            SetWebViewVisibility(hwnd, true);
        else if (type == "set-hidden")
            SetWebViewVisibility(hwnd, false);
        else if (type == "set-desktop-icons-visible")
        {
            HWND listView = GetSysListViewHwnd();
            if (listView != nullptr && IsWindow(listView))
                ShowWindow(listView, SW_SHOW);
        }
        else if (type == "set-desktop-icons-hidden")
        {
            HWND listView = GetSysListViewHwnd();
            if (listView != nullptr && IsWindow(listView))
                ShowWindow(listView, SW_HIDE);
        }
        else if (type == "redirect")
        {
            if (msg.contains("data") && msg["data"].is_object())
            {
                json data = msg["data"];
                if (data.contains("receiver-id") && data["receiver-id"].is_string() &&
                    data.contains("message"))
                {
                    std::string receiverId = data["receiver-id"];
                    MonitorWindow *receiverMw = FindMonitorWindowById(to_wstring(receiverId));
                    std::string senderId = to_string(FindMonitorIdByHwnd(hwnd));
                    if (receiverMw && receiverMw->hwnd && IsWindow(receiverMw->hwnd) && !senderId.empty())
                    {
                        data["sender-id"] = senderId;
                        msg["eventType"] = "redirect";
                        SendEventToHwnd(receiverMw->hwnd, msg, data);
                    }
                }
            }
        }
    }
}

void HandleSubscription(json msg, HWND hwnd, bool sub)
{
    if (msg.contains("eventType") && msg["eventType"].is_string())
    {
        std::string type = msg["eventType"];
        if (type == "mediaChange")
        {
            if (sub)
                AddMediaSubscription(hwnd, msg);
            else
                RemoveMediaSubscription(hwnd);
        }
        else if (type == "playbackChange")
        {
            if (sub)
                AddPlaybackSubscription(hwnd, msg);
            else
                RemovePlaybackSubscription(hwnd);
        }
        else if (type == "timelineChange")
        {
            if (sub)
                AddTimelineSubscription(hwnd, msg);
            else
                RemoveTimelineSubscription(hwnd);
        }
        else if (type == "audioStream")
        {
            if (sub)
                AddAudioSubscription(hwnd, msg);
            else
                RemoveAudioSubscription(hwnd);
        }
    }
}

void HandleJsExec(json j, HWND hwnd)
{
    wprintf(L"\n\nWE ARE PREEXECUTING\n\n");
    if (j.contains("data") && j["data"].is_string())
    {
        auto webview = GetWebViewInstance(hwnd);
        if (webview)
        {
            std::string data = j["data"];
            wprintf(L"\n\nWE ARE EXECUTING %ws\n\n", to_wstring(data).c_str());
            webview->ExecuteScript(to_wstring(data).c_str(), nullptr);
        }
    }
}