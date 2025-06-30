#pragma once

#include <windows.h>
#include <string>

void HandleWebMessage(std::wstring msg, HWND hwnd);
void RaiseErrorBox(std::wstring title, std::wstring caption);
void DispatchJson(std::wstring message);
void DispatchMonitorData();