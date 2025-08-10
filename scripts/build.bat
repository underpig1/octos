@echo off
setlocal
taskkill /f /im Octos.exe
timeout /t 1 >nul
if exist .\build\Debug\Octos.exe (
    del /s /q .\build\Debug\main.exe
)
if exist .\out\Debug\octos.pdb (
    del /s /q .\out\Debug\octos.pdb
)
@REM del /s /q .\build\*.obj
@REM call npm run build-docs
@REM call npm run build-api
msbuild
start "" .\out\Debug\Octos.exe