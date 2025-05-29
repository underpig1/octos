@echo off
taskkill /f /im app.exe

if "%WindowsSdkDir%"=="" (
    echo ERROR: Windows SDK environment not set. Please run this script from a "Developer Command Prompt for Visual Studio".
    pause
    exit /b 1
)

where nuget >nul 2>nul
if errorlevel 1 (
    echo ERROR: NuGet is not installed or not in PATH. Please install NuGet and try again.
    pause
    exit /b 1
)

if exist packages.config (
    echo Restoring NuGet packages...
    nuget restore packages.config -PackagesDirectory packages
)

set WEBVIEW2_VERSION=1.0.3240.44
set WIL_VERSION=1.0.250325.1

set WEBVIEW2_PATH=packages\Microsoft.Web.WebView2.%WEBVIEW2_VERSION%\build\native
set WIL_PATH=packages\Microsoft.Windows.ImplementationLibrary.%WIL_VERSION%\include

set APP_EXE=app.exe
set APP_EXE_PATH=build\%APP_EXE%

if not exist build mkdir build

if exist %APP_EXE_PATH% del /f /q %APP_EXE_PATH%

cl /EHsc /std:c++17 /DWEBVIEW2_NO_IDL /DUNICODE /D_UNICODE ^
   /I"%WEBVIEW2_PATH%\include" ^
   /I"%WIL_PATH%" ^
   src\main.cpp src\Core\Core.cpp src\Watchdog\Watchdog.cpp src\WebView\WebView.cpp src\TrayIcon\TrayIcon.cpp ^
   /link ^
   /LIBPATH:"%WEBVIEW2_PATH%\x64" ^
   WebView2LoaderStatic.lib ^
   ole32.lib uuid.lib user32.lib shell32.lib comctl32.lib shlwapi.lib advapi32.lib Shcore.lib dwmapi.lib ^
   /SUBSYSTEM:WINDOWS /OUT:build/%APP_EXE%

REM Copy assets folder to build directory
if exist build\assets rmdir /s /q build\assets
xcopy /e /i /y assets build\assets

if exist %APP_EXE_PATH% (
    echo Running %APP_EXE_PATH% ...
    start "" %APP_EXE_PATH%
) else (
    echo ERROR: %APP_EXE_PATH% not found. Build may have failed.
)

pause