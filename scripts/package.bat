@echo off
if exist AppPackage (
    del /Q AppPackage\*
    for /D %%p in (AppPackage\*) do rmdir "%%p" /S /Q
)
else (
    mkdir AppPackage
)
msbuild Octos.vcxproj /p:Configuration=Release
xcopy out\assets\* AppPackage\ /s /e /i /Y
MakeAppx pack /d AppPackage /p out\octos-installer.msix
@REM signtool sign /fd SHA256 /f "%USERPROFILE%\underpig.pfx" /p password out\octos-installer.msix