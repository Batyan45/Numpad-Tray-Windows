#include "numpad.h"

// External functions
void ShowPopupWindow();
void HidePopupWindow();
void RecreatePopup();
void CreateButtons();
LRESULT CALLBACK PopupWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ButtonWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void CreateTrayIcon(HWND hwnd) {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hwnd;
    g_nid.uID = IDI_TRAYICON;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_TRAYICON));
    wcscpy_s(g_nid.szTip, L"Numpad - click for digits");
    Shell_NotifyIconW(NIM_ADD, &g_nid);
}

void RemoveTrayIcon() {
    Shell_NotifyIconW(NIM_DELETE, &g_nid);
}

LRESULT CALLBACK HiddenWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TRAYICON:
            switch (LOWORD(lParam)) {
                case WM_LBUTTONUP:
                    if (g_popupVisible) {
                        HidePopupWindow();
                    } else {
                        ShowPopupWindow();
                    }
                    break;
                    
                case WM_RBUTTONUP: {
                    POINT pt;
                    GetCursorPos(&pt);
                    
                    HMENU hMenu = CreatePopupMenu();
                    AppendMenuW(hMenu, MF_STRING | (g_currentMode == MODE_CLASSIC ? MF_CHECKED : 0), 
                               ID_MODE_CLASSIC, L"Classic (0-9)");
                    AppendMenuW(hMenu, MF_STRING | (g_currentMode == MODE_ADVANCED ? MF_CHECKED : 0), 
                               ID_MODE_ADVANCED, L"Advanced (symbols)");
                    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
                    AppendMenuW(hMenu, MF_STRING, ID_RESET_POSITION, L"Reset Position");
                    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
                    AppendMenuW(hMenu, MF_STRING, ID_EXIT, L"Exit");
                    
                    SetForegroundWindow(hwnd);
                    TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN,
                                   pt.x, pt.y, 0, hwnd, nullptr);
                    DestroyMenu(hMenu);
                    break;
                }
            }
            return 0;
        
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_EXIT:
                    RemoveTrayIcon();
                    PostQuitMessage(0);
                    break;
                case ID_MODE_CLASSIC:
                    if (g_currentMode != MODE_CLASSIC) {
                        int oldHeight = GetPopupHeight();
                        g_currentMode = MODE_CLASSIC;
                        int newHeight = GetPopupHeight();
                        
                        if (g_customPosition) {
                            // Shrinking: Move down to keep bottom anchor
                            g_savedY += (oldHeight - newHeight);
                        }
                        RecreatePopup();
                    }
                    break;
                case ID_MODE_ADVANCED:
                    if (g_currentMode != MODE_ADVANCED) {
                        int oldHeight = GetPopupHeight();
                        g_currentMode = MODE_ADVANCED;
                        int newHeight = GetPopupHeight();
                        
                        if (g_customPosition) {
                            // Expanding: Move up to keep bottom anchor
                            g_savedY -= (newHeight - oldHeight);
                        }
                        RecreatePopup();
                    }
                    break;
                case ID_RESET_POSITION:
                    ResetPosition();
                    if (g_popupVisible) {
                        HidePopupWindow();
                        ShowPopupWindow();
                    }
                    break;
            }
            return 0;
        
        case WM_DESTROY:
            RemoveTrayIcon();
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    LoadConfig();

    // Load Icon
    HICON hAppIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAYICON));
    
    // Create fonts - use Consolas for better symbol rendering
    g_hFont = CreateFontW(
        20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );
    
    g_hSymbolFont = CreateFontW(
        18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Consolas"  // Consolas renders & and other symbols better
    );
    
    // Register window classes
    WNDCLASSW hiddenClass = {0};
    hiddenClass.lpfnWndProc = HiddenWndProc;
    hiddenClass.hInstance = hInstance;
    hiddenClass.lpszClassName = L"NumpadTrayHidden";
    RegisterClassW(&hiddenClass);
    
    WNDCLASSW popupClass = {0};
    popupClass.lpfnWndProc = PopupWndProc;
    popupClass.hInstance = hInstance;
    popupClass.lpszClassName = L"NumpadPopup";
    popupClass.hCursor = LoadCursor(nullptr, IDC_SIZEALL);
    popupClass.hIcon = hAppIcon;
    RegisterClassW(&popupClass);
    
    WNDCLASSW buttonClass = {0};
    buttonClass.lpfnWndProc = ButtonWndProc;
    buttonClass.hInstance = hInstance;
    buttonClass.lpszClassName = L"NumpadButton";
    buttonClass.hCursor = LoadCursor(nullptr, IDC_HAND);
    RegisterClassW(&buttonClass);
    
    g_hHidden = CreateWindowW(
        L"NumpadTrayHidden", L"",
        0, 0, 0, 0, 0,
        HWND_MESSAGE, nullptr, hInstance, nullptr
    );
    
    // Create popup with WS_EX_NOACTIVATE to prevent stealing focus
    g_hPopup = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        L"NumpadPopup", L"",
        WS_POPUP,
        0, 0, GetPopupWidth(), GetPopupHeight(),
        nullptr, nullptr, hInstance, nullptr
    );
    
    CreateButtons();
    CreateTrayIcon(g_hHidden);
    
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    if (g_hFont) DeleteObject(g_hFont);
    if (g_hSymbolFont) DeleteObject(g_hSymbolFont);
    
    return (int)msg.wParam;
}
