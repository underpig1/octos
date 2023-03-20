#include <windows.h>

#pragma once

void Attach(unsigned char*);
void Detach(unsigned char*);
POINT MousePosition(void);
BOOL LMousePressed(void);
BOOL MMousePressed(void);
BOOL RMousePressed(void);
BOOL InForeground(void);
void SetTaskbar(BOOL state);
bool* KeyboardState(void);
void SendMediaEvent(int state);
char* ForegroundWindow(void);
std::string TrackTitle(void);