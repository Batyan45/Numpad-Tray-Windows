#include "numpad.h"
#include <shlobj.h>

// Global definitions
Mode g_currentMode = MODE_CLASSIC;
HWND g_hPopup = nullptr;
HWND g_hHidden = nullptr;
HWND g_hButtons[32] = {nullptr};
NOTIFYICONDATA g_nid = {0};
HFONT g_hFont = nullptr;
HFONT g_hSymbolFont = nullptr;
bool g_popupVisible = false;
int g_hoveredButton = -1;
int g_pressedButton = -1;
bool g_customPosition = false;
int g_savedX = 0;
int g_savedY = 0;
HWND g_lastActiveWindow = nullptr;

std::wstring GetConfigPath() {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        std::wstring configDir = std::wstring(path) + L"\\NumpadTray";
        CreateDirectoryW(configDir.c_str(), nullptr);
        return configDir + L"\\config.ini";
    }
    return L"numpad_config.ini";
}

void LoadConfig() {
    std::wstring path = GetConfigPath();
    
    wchar_t buffer[64];
    GetPrivateProfileStringW(L"Settings", L"Mode", L"classic", buffer, 64, path.c_str());
    g_currentMode = (wcscmp(buffer, L"advanced") == 0) ? MODE_ADVANCED : MODE_CLASSIC;
    
    g_customPosition = GetPrivateProfileIntW(L"Position", L"Custom", 0, path.c_str()) != 0;
    g_savedX = GetPrivateProfileIntW(L"Position", L"X", 0, path.c_str());
    g_savedY = GetPrivateProfileIntW(L"Position", L"Y", 0, path.c_str());
}

void SaveConfig() {
    std::wstring path = GetConfigPath();
    
    WritePrivateProfileStringW(L"Settings", L"Mode", 
        g_currentMode == MODE_ADVANCED ? L"advanced" : L"classic", path.c_str());
    
    WritePrivateProfileStringW(L"Position", L"Custom", 
        g_customPosition ? L"1" : L"0", path.c_str());
    
    wchar_t buffer[32];
    swprintf_s(buffer, L"%d", g_savedX);
    WritePrivateProfileStringW(L"Position", L"X", buffer, path.c_str());
    swprintf_s(buffer, L"%d", g_savedY);
    WritePrivateProfileStringW(L"Position", L"Y", buffer, path.c_str());
}

void ResetPosition() {
    g_customPosition = false;
    g_savedX = 0;
    g_savedY = 0;
    SaveConfig();
}

int GetPopupWidth() {
    return 5 * BUTTON_SIZE + 4 * BUTTON_MARGIN + 2 * POPUP_PADDING;
}

int GetPopupHeight() {
    if (g_currentMode == MODE_CLASSIC) {
        // 2 rows
        return 2 * BUTTON_SIZE + 1 * BUTTON_MARGIN + 2 * POPUP_PADDING;
    } else {
        // 4 rows for advanced (2 symbol rows + 2 digit rows)
        return 4 * BUTTON_SIZE + 3 * BUTTON_MARGIN + 2 * POPUP_PADDING;
    }
}
