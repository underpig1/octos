#include "../main.h"
#include "../Bridge/Bridge.h"
#include "../Storage/Storage.h"
#include "../Core/Core.h"
#include "../WebView/WebView.h"

void ParseCommandLineArgs(LPWSTR args)
{
    wprintf(L"\n\n################ I GOT THIS FOR ARG: %s\n\n", args);
    int argc;
    LPWSTR *argv = CommandLineToArgvW(args, &argc);
    if (argv == nullptr)
        return;
    if (argc < 3)
        return;
    const std::wstring subcommand = argv[1];
    if (subcommand == L"run")
    {
        bool autoFlag = false;
        bool devToolsFlag = false;
        std::wstring folderPath;
        for (int i = 2; i < argc; ++i)
        {
            std::wstring arg = argv[i];
            if (arg == L"--auto")
                autoFlag = true;
            else if (arg == L"--dev-tools")
                devToolsFlag = true;
            else if ((!arg.empty() && arg[0] == L'-') &&
                     (arg.size() == 1 || arg[1] != L'-'))
                return;
            else if (folderPath.empty())
                folderPath = arg;
        }
        if (folderPath.empty())
            return;
        ConfigParams params = GetFolderConfigParams(fs::directory_entry(folderPath));
        std::wstring entryPath = params.entryPath;
        if (!entryPath.empty())
        {
            json prefs = LoadPrefs();
            prefs["selected"] = json::object();
            DumpPrefs(prefs);
            WaitForWallpaperWindowsAndCallback([entryPath, devToolsFlag]()
                                               {
                                            wprintf(L"\n\n#### NAVIGATING ALL\n\n");
                                            NavigateAllWallpapers(entryPath);
                                            WaitForMainWindowAndDispatch(L"{\"type\":\"preview\"}");
                                            if (devToolsFlag)
                                            {
                                                for (auto &mw : ms)
                                                {
                                                    if (IsWindow(mw.hwnd)) {
                                                        PostMessage(mw.hwnd, WM_USER + 7, 0, 0);
                                                    }
                                                }
                                            }
                                        });
        }
    }
    LocalFree(argv);
    return;
}