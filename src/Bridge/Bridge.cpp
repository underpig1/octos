#include <nlohmann/json.hpp>
#include <thread>

#include "Bridge.h"
#include "../main.h"
#include "../Core/Core.h"
#include "../WebView/Webview.h"
#include "../Storage/Storage.h"
#include "../API/API.h"

using json = nlohmann::json;

void DispatchToHwnd(HWND hwnd, std::wstring message)
{
    WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (data && data->webview)
        data->webview->PostWebMessageAsJson(message.c_str());
}

void DispatchJson(std::wstring message)
{
    wprintf(L"DISPATCHING MESSAGE: %ws\n", message.c_str());
    DispatchToHwnd(app_hwnd, message);
}

void DispatchByMonitorId(std::wstring message, std::wstring monitorId)
{
    MonitorWindow *mw = FindMonitorWindowById(monitorId);
    if (mw && mw->hwnd && IsWindow(mw->hwnd)) {
        WebViewData *data = reinterpret_cast<WebViewData *>(GetWindowLongPtr(mw->hwnd, GWLP_USERDATA));
        if (data && data->webview)
        {
            wprintf(L"\n\n\nDISPATCHING WALLPAPER MESSAGE: %ws\n\n\n", message.c_str());
            data->webview->PostWebMessageAsJson(message.c_str());
        }
    }
}

void RaiseErrorBox(std::wstring title, std::wstring caption)
{
    std::wstring message =
        L"{\"type\":\"error-box\",\"title\":\"" + title +
        L"\",\"caption\":\"" + caption +
        L"\"}";
    DispatchJson(message);
}

void DispatchNavigateAllWallpapers(std::wstring id)
{
    json message =
        {
            {"type", "navigate-all"},
            {"id", to_string(id)}
        };
    DispatchJson(to_wstring(message.dump()));
}

void DispatchMonitorData()
{
    std::vector<std::wstring> monitorIds16 = GetMonitorIds();
    std::vector<std::string> monitorIds8;
    for (const auto &monitorId16 : monitorIds16)
        monitorIds8.push_back(to_string(monitorId16));
    json sendJson = {
        {"type", "monitor-ids"},
        {"data", monitorIds8}};
    DispatchJson(to_wstring(sendJson.dump()));
}

void DispatchVisibility()
{
    std::wstring visibility = IsWindowVisible(ms[0].hwnd) ? L"true" : L"false";
    std::wstring message = L"{\"type\":\"visibility\",\"value\":" + visibility + L"}";
    DispatchJson(message);
}

void DispatchSetWallpaper(std::wstring monitorId)
{
    json sendJson = {
        {"type", "wallpaper-set"},
        {"monitor-id", to_string(monitorId)}};
    DispatchJson(to_wstring(sendJson.dump()));
}

void HandleWebMessage(std::wstring msg, HWND hwnd)
{
    printf("RECIEVED MESSAGE: %ws\n", msg.c_str());
    try
    {
        json j = json::parse(to_string(msg));
        std::string type = j.value("type", "");
        // printf("RECIEVED MESSAGE: %s\n", j.dump().c_str());

        if (hwnd == app_hwnd) {
            if (type == "drag")
            {
                ReleaseCapture();
                SendMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
            }
            else if (type == "minimize")
                ShowWindow(app_hwnd, SW_MINIMIZE);
            else if (type == "maximize")
            {
                if (IsZoomed(hwnd))
                    ShowWindow(hwnd, SW_RESTORE);
                else
                    ShowWindow(hwnd, SW_MAXIMIZE);
            }
            else if (type == "close")
                ReleaseMainWindow();
            else if (type == "open-external-link")
            {
                std::string url = j.value("url", "");
                if (!url.empty())
                    ShellExecute(NULL, L"open", to_wstring(url).c_str(), NULL, NULL, SW_SHOWNORMAL);
            }
            else if (type == "refresh")
            {
                RecreateWallpapers();
                std::wstring message = IterateWallpapersAsJsonString();
                DispatchJson(message);
            }
            else if (type == "request-wallpaper-data")
            {
                std::wstring message = IterateWallpapersAsJsonString();
                DispatchJson(message);
            }
            else if (type == "install-wallpaper")
            {
                std::thread([]()
                            {
                                bool result = SelectAndInstallWallpaper();
                                if (result)
                                {
                                    std::wstring message = IterateWallpapersAsJsonString();
                                    PostMessage(app_hwnd, WM_USER + 5, 0, (LPARAM) new std::wstring(message));
                                    message = L"{\"type\":\"installed-wallpaper\"}";
                                    PostMessage(app_hwnd, WM_USER + 5, 0, (LPARAM) new std::wstring(message));
                                    // message = L"{\"type\":\"error-box\",\"title\":\"Successfully installed\",\"caption\":\"Your new wallpaper is ready.\"}";
                                    // PostMessage(app_hwnd, WM_USER + 5, 0, (LPARAM) new std::wstring(message));
                                }
                                else if (result != NULL)
                {
                    std::wstring message = L"{\"type\":\"error-box\",\"title\":\"Installation failed\",\"caption\":\"Please try again.\"}";
                    PostMessage(app_hwnd, WM_USER + 5, 0, (LPARAM) new std::wstring(message));
                } })
                    .detach();
            }
            else if (type == "request-visibility")
            {
                DispatchVisibility();
            }
            else if (type == "set-visibility")
            {
                if (j.contains("value") && j["value"].is_boolean())
                {
                    bool visibility = j["value"];
                    SetWallpaperVisibility(visibility);
                }
            }
            else if (type == "open-folder")
            {
                std::string path = j.value("path", "");
                if (!path.empty())
                {
                    std::wstring p;
                    if (j.value("relative", false))
                        p = ResolvePath(to_wstring(path));
                    else
                        p = to_wstring(path);
                    ShellExecuteW(NULL, L"open", p.c_str(), NULL, NULL, SW_SHOWDEFAULT);
                }
            }
            else if (type == "request-prefs")
                DispatchJson(LoadPrefsAsJsonString());
            else if (type == "dump-prefs")
            {
                if (j.contains("value") && j["value"].is_object())
                    DumpPrefs(j["value"]);
            }
            else if (type == "remove-wallpaper")
            {
                if (j.contains("folderPath") && j["folderPath"].is_string())
                {
                    std::string folderPath = j["folderPath"];
                    RemoveWallpaper(to_wstring(folderPath));
                    std::wstring message = IterateWallpapersAsJsonString();
                    DispatchJson(message);
                }
            }
            else if (type == "request-monitor-ids")
                DispatchMonitorData();
            else if (type == "set-wallpaper")
            {
                if (j.contains("monitor-id") && j.contains("url") &&
                    j["monitor-id"].is_string() && j["url"].is_string())
                {
                    std::string monitorId = j["monitor-id"];
                    std::string url = j["url"];
                    NavigateWallpaperByMonitorId(to_wstring(monitorId), to_wstring(url));
                }
            }
            else if (type == "download-wallpaper")
            {
                if (j.contains("id") && j.contains("url") &&
                    j["id"].is_string() && j["url"].is_string())
                {
                    const std::string url = j["url"];
                    const std::string id = j["id"];
                    std::thread([url, id]()
                                {
                        if (DownloadWallpaper(to_wstring(url)))
                        {
                            wprintf(L"SUCCESS????\n");
                            json sendJson = {
                                {"type", "downloaded-wallpaper"},
                                {"url", url},
                                {"id", id}};
                            std::wstring message = to_wstring(sendJson.dump());
                            PostMessage(app_hwnd, WM_USER + 5, 0, (LPARAM) new std::wstring(message));
                            message = IterateWallpapersAsJsonString();
                            PostMessage(app_hwnd, WM_USER + 5, 0, (LPARAM) new std::wstring(message));
                        }
                        else
                            RaiseErrorBox(L"Download failed", L"Please try again."); })
                        .detach();
                }
            }
            else if (type == "send-to-wallpaper")
            {
                if (j.contains("data") && j.contains("monitor-id") &&
                    j["data"].is_object() && j["monitor-id"].is_string())
                {
                    std::string monitorId = j["monitor-id"];
                    DispatchByMonitorId(to_wstring(j["data"].dump()), to_wstring(monitorId));
                }
            }
        }
        else { // API
            if (type == "request")
                HandleRequest(j, hwnd);
            else if (type == "subscribe" || type == "unsubscribe")
                HandleSubscription(j, hwnd, type == "subscribe");
            else if (type == "command")
                HandleCommand(j, hwnd);
        }
    }
    catch (const std::exception &e)
    {
        wprintf(L"EXCEPTION: %S\n", e.what());
    }
    catch (...)
    {
        wprintf(L"UNKNOWN EXCEPTION\n");
    }
}