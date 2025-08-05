#pragma once

#include <windows.h>

void InitializeTrayIcon();
void DestroyTrayIcon();
void ShowTrayMenu();
void ShowTrayNotification(const wchar_t *title, const wchar_t *message);