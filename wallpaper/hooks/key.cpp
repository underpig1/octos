#include <windows.h>

int main() {
    INPUT input[2];
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = VK_MEDIA_PLAY_PAUSE;
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = VK_MEDIA_PLAY_PAUSE;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, input, sizeof(INPUT));
}