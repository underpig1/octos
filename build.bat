@echo off
setlocal enabledelayedexpansion

rem Kill previous instance
taskkill /f /im app.exe >nul 2>nul

rem Check for Windows SDK environment
if "%WindowsSdkDir%"=="" (
    echo ERROR: Windows SDK environment not set. Please run this from a Developer Command Prompt.
    pause
    exit /b 1
)

rem Check for NuGet
where nuget >nul 2>nul
if errorlevel 1 (
    echo ERROR: NuGet is not installed or not in PATH.
    pause
    exit /b 1
)

rem Restore packages if needed
if exist packages.config (
    echo Restoring NuGet packages...
    nuget restore packages.config -PackagesDirectory packages
)

rem Setup versions and paths
set WEBVIEW2_VERSION=1.0.3240.44
set WIL_VERSION=1.0.250325.1

set WEBVIEW2_PATH=packages\Microsoft.Web.WebView2.%WEBVIEW2_VERSION%\build\native
set WIL_PATH=packages\Microsoft.Windows.ImplementationLibrary.%WIL_VERSION%\include

set APP_EXE=app.exe
set APP_EXE_PATH=build\%APP_EXE%
set OBJ_DIR=build\obj

rem Ensure output directories exist
if not exist build mkdir build
if not exist %OBJ_DIR% mkdir %OBJ_DIR%

rem Compile all changed .cpp files to .obj
echo Compiling source files...

set FILES=
for /r src %%f in (*.cpp) do (
    set SRC=%%f
    set NAME=%%~nf
    set OBJ=%OBJ_DIR%\%%~nxf.obj

    rem Recompile if .obj is missing or older than .cpp
    if not exist "!OBJ!" (
        echo Compiling !SRC! ...
        cl /nologo /c /EHsc /std:c++17 /DWEBVIEW2_NO_IDL /DUNICODE /D_UNICODE ^
            /I"%WEBVIEW2_PATH%\include" ^
            /I"%WIL_PATH%" ^
            "!SRC!" /Fo"!OBJ!"
    ) else (
        for %%X in (!OBJ!) do set OBJTIME=%%~tX
        for %%Y in (!SRC!) do set SRCTIME=%%~tY
        if "!SRCTIME!" GTR "!OBJTIME!" (
            echo Recompiling updated !SRC! ...
            cl /nologo /c /EHsc /std:c++17 /DWEBVIEW2_NO_IDL /DUNICODE /D_UNICODE ^
                /I"%WEBVIEW2_PATH%\include" ^
                /I"%WIL_PATH%" ^
                "!SRC!" /Fo"!OBJ!"
        )
    )

    set FILES=!FILES! "!OBJ!"
)

rem Link all .obj files
echo Linking...
link /nologo ^
    %FILES% ^
    /OUT:%APP_EXE_PATH% ^
    /SUBSYSTEM:WINDOWS ^
    /LIBPATH:"%WEBVIEW2_PATH%\x64" ^
    WebView2LoaderStatic.lib ^
    ole32.lib uuid.lib user32.lib shell32.lib comctl32.lib shlwapi.lib advapi32.lib ^
    Shcore.lib dwmapi.lib dcomp.lib d3d11.lib windowsapp.lib

rem Copy web files
if exist build\app rmdir /s /q build\app
xcopy /e /i /y src\app build\app

if exist build\assets rmdir /s /q build\assets
xcopy /e /i /y assets build\assets

rem Run the app if build succeeded
if exist %APP_EXE_PATH% (
    echo Running %APP_EXE_PATH% ...
    start "" %APP_EXE_PATH%
) else (
    echo ERROR: Build failed.
)

pause
