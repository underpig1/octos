@echo off
setlocal
taskkill /f /im main.exe
if not exist ".\build\CMakeCache.txt" (
    cmake --preset=default
)
cmake --build build
start "" .\out\Debug\main.exe