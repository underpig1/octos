@echo off
if exist AppPackage (
    del /Q AppPackage\*
    for /D %%p in (AppPackage\*) do rmdir "%%p" /S /Q
)
else (
    mkdir AppPackage
)
if not exist ".\build\CMakeCache.txt" (
    rmdir /s /q build
    mkdir build
    cmake -S . -B build --preset=default
)
cmake --build build --config Release
xcopy out\Release\* AppPackage\ /s /e /i /Y
MakeAppx pack /d AppPackage /p out\octos-installer.msix
signtool sign /fd SHA256 /f "%USERPROFILE%\underpig.pfx" /p password out\octos-installer.msix