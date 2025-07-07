@echo off
setlocal
taskkill /f /im main.exe
@REM call npm run build-docs
call npm run build-api
if not exist ".\build\CMakeCache.txt" (
    cmake --preset=default
)
cmake --build build
start "" .\out\Debug\main.exe