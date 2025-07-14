@echo off
setlocal
taskkill /f /im main.exe
timeout /t 1 >nul
if exist .\out\Debug\main.exe (
    del /s /q .\out\Debug\main.exe
)
if exist .\out\Debug\main.pdb (
    del /s /q .\out\Debug\main.pdb
)
@REM call npm run build-docs
call npm run build-api
if not exist ".\build\CMakeCache.txt" (
    cmake --preset=default
)
cmake --build build
start "" .\out\Debug\main.exe