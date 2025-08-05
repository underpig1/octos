@echo off
setlocal
taskkill /f /im octos.exe
timeout /t 1 >nul
if exist .\out\Debug\octos.exe (
    del /s /q .\out\Debug\main.exe
)
if exist .\out\Debug\octos.pdb (
    del /s /q .\out\Debug\octos.pdb
)
@REM del /s /q .\build\*.obj
@REM call npm run build-docs
@REM call npm run build-api
if not exist ".\build\CMakeCache.txt" (
    cmake --preset=default
)
cmake --build build
start "" .\out\Debug\octos.exe