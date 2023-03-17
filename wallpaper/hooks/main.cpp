#define UNICODE 1

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <gdiplus.h>
#include <iostream>

#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    HWND p = FindWindowEx(hwnd, NULL, L"SHELLDLL_DefView", NULL);
    HWND* ret = (HWND*)lParam;

    if (p) *ret = FindWindowEx(NULL, hwnd, L"WorkerW", NULL);
    return true;
}

VOID OnPaint(HDC hdc)
{
    Graphics graphics(hdc);
    
    // graphics.DrawLine(&pen, 0, 0, 200, 100);
    // Image image(L"C:\\Users\\chase\\Downloads\\test.gif");

    // FontFamily fontFamily(L"Roboto");
    // Font font(&fontFamily, 64, FontStyleRegular, UnitPixel);
    // PointF pointF(30.0f, 10.0f);
    // SolidBrush solidBrush(Color(255, 255, 255, 255));
    // graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
    // graphics.DrawString(L"Hello", -1, &font, pointF, &solidBrush);
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    HWND progman = FindWindow(L"ProgMan", NULL);
    SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
    HWND desktop = nullptr;
    EnumWindows(EnumWindowsProc, (LPARAM)&desktop);
    HDC canvas = GetDC(desktop);

    while (true)
    {
        OnPaint(canvas);
    }

    ReleaseDC(desktop, canvas);
    GdiplusShutdown(gdiplusToken);
}