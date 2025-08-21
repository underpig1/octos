#include <chrono>

#include "../Bridge/Bridge.h"
#include "../Core/Core.h"
#include "../Storage/Storage.h"
#include "../WebView/WebView.h"
#include "../main.h"

// void RunWallpaperCommand(std::wstring folderPath, bool devToolsFlag)
// {
//     ConfigParams params = GetFolderConfigParams(fs::directory_entry(folderPath), false);
//     std::wstring entryPath = params.entryPath;
//     if (!entryPath.empty())
//     {
//         json prefs = LoadPrefs();
//         prefs["selected"] = json::object();
//         DumpPrefs(prefs);
//         WaitForWallpaperWindowsAndCallback([entryPath, devToolsFlag, params]()
//                                            {
//             wprintf(L"\n\n#### NAVIGATING ALL\n\n");
//             NavigateAllWallpapers(entryPath);
//             std::wstring paramStr = ParamsAsJsonString(params);
//             if (paramStr.empty())
//             {
//                 MessageBox(app_hwnd, (L"Could not run mod at " + params.folderPath + L". This may be a configuration error with octos.json.").c_str(), L"[Octos CLI] Run failed", MB_OK | MB_ICONERROR | MB_TOPMOST);
//                 return;
//             }
//             std::wstring sendStr = L"{\"type\":\"preview\",\"data\":" + paramStr + L"}";
//             wprintf(sendStr.c_str());
//             ReloadAllWindows();

//             // dev tools
//             std::thread([devToolsFlag, sendStr]()
//                         {
//                     std::this_thread::sleep_for(std::chrono::seconds(1));
//                     WaitForMainWindowAndDispatch(sendStr);
                    
//                     if (devToolsFlag)
//                     {
//                         bool found_window = false;
//                         for (auto &mw : ms)
//                         {
//                             if (IsWindow(mw.hwnd))
//                             {
//                                 PostMessage(mw.hwnd, WM_USER + 7, 0, 0);
//                                 found_window = true;
//                             }
//                         }
//                         if (!found_window)
//                         {
//                             std::this_thread::sleep_for(std::chrono::seconds(1));
//                             for (auto &mw : ms)
//                             {
//                                 if (IsWindow(mw.hwnd))
//                                 {
//                                     PostMessage(mw.hwnd, WM_USER + 7, 0, 0);
//                                     found_window = true;
//                                 }
//                             }
//                         }
//                         if (!found_window)
//                         {
//                             MessageBox(app_hwnd, L"Failed to open DevTools", L"[Octos CLI] DevTools failed", MB_OK | MB_ICONERROR | MB_TOPMOST);
//                         }
//                     } })
//             .detach(); });
//         // if (configFlag && !params.configPath.empty())
//         // {
//         //     OpenFile(to_string(params.configPath));
//         // }
//     }
//     else
//     {
//         MessageBox(app_hwnd, (L"Could not find a valid HTML file in folder " + folderPath).c_str(), L"[Octos CLI] Run failed", MB_OK | MB_ICONERROR | MB_TOPMOST);
//     }
// }

// THIS METHOD USES VIRTUAL FOLDER MAPPING
void RunWallpaperCommand(std::wstring folderPath, bool devToolsFlag)
{
    ConfigParams params = GetFolderConfigParams(fs::directory_entry(folderPath));
    std::wstring entryPath = params.entryPath;
    if (!entryPath.empty())
    {
        std::thread([folderPath, devToolsFlag]
                    {
            fs::path wallpapers = GetWallpapersDir();
            fs::path newDir = wallpapers / fs::path(folderPath).filename();
            fs::copy(folderPath, newDir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            ConfigParams params = GetFolderConfigParams(fs::directory_entry(newDir));
            std::wstring entryPath = params.entryPath;
            // json prefs = LoadPrefs();
            // prefs["selected"] = json::object();
            // DumpPrefs(prefs);
            WaitForWallpaperWindowsAndCallback([entryPath, devToolsFlag, params]()
                                               {
                                                   std::this_thread::sleep_for(std::chrono::seconds(1));
                                                   wprintf(L"\n\n#### NAVIGATING ALL\n\n");
                                                   NavigateAllWallpapers(entryPath);
                                                   std::wstring paramStr = ParamsAsJsonString(params);
                                                   std::wstring sendStr = L"{\"type\":\"select-all\",\"data\":" + paramStr + L"}";
                                                   WaitForMainWindowAndDispatch(sendStr);
                                                   std::wstring message = IterateWallpapersAsJsonString();
                                                   WaitForMainWindowAndDispatch(message);
                                                   ReloadAllWindows();
                                                   std::this_thread::sleep_for(std::chrono::seconds(1));

                                                   if (devToolsFlag)
                                                   {
                                                       bool found_window = false;
                                                       for (auto &mw : ms)
                                                       {
                                                           if (IsWindow(mw.hwnd))
                                                           {
                                                               PostMessage(mw.hwnd, WM_USER + 7, 0, 0);
                                                               found_window = true;
                                                           }
                                                       }
                                                       if (!found_window)
                                                       {
                                                           std::this_thread::sleep_for(std::chrono::seconds(1));
                                                           for (auto &mw : ms)
                                                           {
                                                               if (IsWindow(mw.hwnd))
                                                               {
                                                                   PostMessage(mw.hwnd, WM_USER + 7, 0, 0);
                                                                   found_window = true;
                                                               }
                                                           }
                                                       }
                                                       if (!found_window)
                                                       {
                                                           MessageBox(app_hwnd, L"Failed to open DevTools", L"[Octos CLI] DevTools failed", MB_OK | MB_ICONERROR | MB_TOPMOST);
                                                       }
                                                   } }); })
            .detach();
        // if (configFlag && !params.configPath.empty())
        // {
        //     OpenFile(to_string(params.configPath));
        // }
    }
    else
    {
        MessageBox(app_hwnd, (L"Could not find a valid HTML file in folder " + folderPath).c_str(), L"[Octos CLI] Run failed", MB_OK | MB_ICONERROR | MB_TOPMOST);
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
            // else if ((!arg.empty() && arg[0] == L'-') &&
            //          (arg.size() == 1 || arg[1] != L'-'))
            //     return;
            else if (!arg.empty() && arg[0] != L'-' && folderPath.empty())
                folderPath = arg;
        }
        if (folderPath.empty())
            folderPath = fs::current_path();
        // {
        //     MessageBox(app_hwnd, L"No folderPath provided. Try specifying the path to a valid mod folder. Ex. octos run path/to/mod", L"[Octos CLI] Run failed", MB_OK | MB_ICONERROR | MB_TOPMOST);
        //     return;
        // }
        folderPath = fs::absolute(folderPath);
        if (!fs::exists(folderPath))
        {
            MessageBox(app_hwnd, (L"Provided folderPath " + folderPath + L" does not exist! Try specifying the path to a valid mod folder. Ex. octos run path/to/mod").c_str(), L"[Octos CLI] Run failed", MB_OK | MB_ICONERROR | MB_TOPMOST);
            return;
        }
        else if (!fs::is_directory(folderPath))
            folderPath = fs::path(folderPath).parent_path();
        RunWallpaperCommand(folderPath, devToolsFlag);
    }
    else if (subcommand == L"reload")
    {
        wprintf(L"refreshing1\n");
        ReloadAllWindows();
    }
    else if (subcommand == L"new")
    {
        std::wstring folderName;
        for (int i = 2; i < argc; ++i)
        {
            std::wstring arg = argv[i];
            if (!arg.empty() && arg[0] != L'-' && folderName.empty())
            {
                folderName = arg;
                break;
            }
        }
        if (folderName.empty())
            folderName = L"new-mod";
        std::wstring folderPath = fs::absolute(folderName);
        if (!fs::exists(folderPath) || !fs::is_directory(folderPath))
        {
            if (fs::create_directory(folderPath))
                CreateNewWallpaper(folderPath);
            else
            {
                MessageBox(app_hwnd, (L"Failed to create new mod at " + folderPath).c_str(), L"[Octos CLI] New failed", MB_OK | MB_ICONERROR | MB_TOPMOST);
                return;
            }
        }
        else
            CreateNewWallpaper(folderPath);
        MessageBox(app_hwnd, (L"Successfully created new mod at " + folderPath).c_str(), L"[Octos CLI] New succeeded", MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
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