#include <windows.h>
#include <shellapi.h>

void ParseCommandLineArgs()
{
    int argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == nullptr)
        return;
}