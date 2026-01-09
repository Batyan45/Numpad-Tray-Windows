#include "numpad.h"
#include "hotkeys.h"
#include <iostream>

HHOOK g_hKeyboardHook = nullptr;

// Synthesize a key press
void SendKey(WORD vk) {
    INPUT inputs[2] = {};
    
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = vk;
    
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = vk;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    
    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* pKbd = (KBDLLHOOKSTRUCT*)lParam;
        
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            bool handled = false;
            WORD targetVk = 0;
            
            switch (pKbd->vkCode) {
                case VK_F1: targetVk = '1'; handled = true; break;
                case VK_F2: targetVk = '2'; handled = true; break;
                case VK_F3: targetVk = '3'; handled = true; break;
                case VK_F4: targetVk = '4'; handled = true; break;
                case VK_F5: targetVk = '5'; handled = true; break;
                case VK_F6: targetVk = '6'; handled = true; break;
                case VK_F7: targetVk = '7'; handled = true; break;
                case VK_F8: targetVk = '8'; handled = true; break;
                case VK_F9: targetVk = '9'; handled = true; break;
                case VK_F10: targetVk = '0'; handled = true; break;
            }
            
            if (handled) {
                SendKey(targetVk);
                return 1; // Suppress original key
            }
        }
    }
    
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

void StartKeyboardHook() {
    if (!g_hKeyboardHook) {
        g_hKeyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(nullptr), 0);
    }
}

void StopKeyboardHook() {
    if (g_hKeyboardHook) {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = nullptr;
    }
}
