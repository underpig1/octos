#include "../Bridge/Bridge.h"
#include "../Core/Core.h"
#include "../Storage/Storage.h"
#include "../WebView/WebView.h"
#include "../main.h"
#include <chrono>

void RunWallpaperCommand(std::wstring folderPath, bool devToolsFlag)
{
    ConfigParams params = GetFolderConfigParams(fs::directory_entry(folderPath));
    std::wstring entryPath = params.entryPath;
    if (!entryPath.empty())
    {
        json prefs = LoadPrefs();
        prefs["selected"] = json::object();
        DumpPrefs(prefs);
        WaitForWallpaperWindowsAndCallback([entryPath, devToolsFlag, params]()
                                           {
                wprintf(L"\n\n#### NAVIGATING ALL\n\n");
                NavigateAllWallpapers(entryPath);
                std::wstring paramStr = ParamsAsJsonString(params);
                if (paramStr.empty()) return;
                std::wstring sendStr = L"{\"type\":\"preview\",\"data\":" + paramStr + L"}";
                wprintf(sendStr.c_str());
                WaitForMainWindowAndDispatch(sendStr);

                // dev tools
                if (devToolsFlag) {
                    std::thread([]() {
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                        bool found_window = false;
                        for (auto &mw : ms) {
                            if (IsWindow(mw.hwnd)) {
                                PostMessage(mw.hwnd, WM_USER + 7, 0, 0);
                                found_window = true;
                            }
                        }
                        if (!found_window) {
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            for (auto &mw : ms) {
                                if (IsWindow(mw.hwnd))
                                    PostMessage(mw.hwnd, WM_USER + 7, 0, 0);
                            }
                        }
                    }).detach();
                } });
        // if (configFlag && !params.configPath.empty())
        // {
        //     OpenFile(to_string(params.configPath));
        // }
    }
}

void ParseCommandLineArgs(LPWSTR args)
{
    wprintf(L"\n\n################ I GOT THIS FOR ARG: %s\n\n", args);
    int argc;
    LPWSTR *argv = CommandLineToArgvW(args, &argc);
    if (argv == nullptr || argc == 1)
        return;
    const std::wstring subcommand = argv[1];
    wprintf(L"sub command is %ws\n", subcommand.c_str());
    if (subcommand == L"run")
    {
        bool autoFlag = false;
        bool devToolsFlag = false;
        // bool configFlag = false;
        std::wstring folderPath;
        for (int i = 2; i < argc; ++i)
        {
            std::wstring arg = argv[i];
            if (arg == L"--auto")
                autoFlag = true;
            else if (arg == L"--dev-tools")
                devToolsFlag = true;
            // else if (arg == L"--config")
            //     configFlag = true;
            else if ((!arg.empty() && arg[0] == L'-') &&
                     (arg.size() == 1 || arg[1] != L'-'))
                return;
            else if (folderPath.empty())
                folderPath = arg;
        }
        if (folderPath.empty())
            return;
        folderPath = fs::absolute(folderPath);
        if (!fs::exists(folderPath) || !fs::is_directory(folderPath))
            return;
        RunWallpaperCommand(folderPath, devToolsFlag);
    }
    else if (subcommand == L"reload")
    {
        wprintf(L"refreshing1\n");
        ReloadAllWindows();
    }
    else if (subcommand == L"new")
    {
        std::wstring folderPath;
        for (int i = 2; i < argc; ++i)
        {
            std::wstring arg = argv[i];
            if ((!arg.empty() && arg[0] == L'-') &&
                (arg.size() == 1 || arg[1] != L'-'))
                return;
            else if (folderPath.empty())
                folderPath = arg;
        }
        if (folderPath.empty())
            return;
        folderPath = fs::absolute(folderPath);
        if (!fs::exists(folderPath) || !fs::is_directory(folderPath))
            return;
        CreateNewWallpaper(folderPath);
    }
    else if (subcommand == L"dev-tools")
    {
        for (auto &mw : ms)
        {
            if (IsWindow(mw.hwnd))
                PostMessage(mw.hwnd, WM_USER + 7, 0, 0);
        }
    }
    LocalFree(argv);
    return;
}