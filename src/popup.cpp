#include "numpad.h"

// Dragging state
static bool g_dragging = false;
static POINT g_dragStart = {0};

void CreateButtons();
void RecreatePopup();
void ShowPopupWindow();
void HidePopupWindow();

LRESULT CALLBACK PopupWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            BOOL useDarkMode = TRUE;
            DwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));
            int cornerPreference = 2;
            DwmSetWindowAttribute(hwnd, 33, &cornerPreference, sizeof(cornerPreference));
            int backdropType = 2;
            DwmSetWindowAttribute(hwnd, 38, &backdropType, sizeof(backdropType));
            return 0;
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rc;
            GetClientRect(hwnd, &rc);
            
            HBRUSH hBrush = CreateSolidBrush(BG_COLOR);
            FillRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_ERASEBKGND:
            return 1;
        
        case WM_LBUTTONDOWN: {
            g_dragging = true;
            GetCursorPos(&g_dragStart);
            SetCapture(hwnd);
            return 0;
        }
        
        case WM_MOUSEMOVE: {
            if (g_dragging) {
                POINT pt;
                GetCursorPos(&pt);
                
                RECT rc;
                GetWindowRect(hwnd, &rc);
                
                int newX = rc.left + (pt.x - g_dragStart.x);
                int newY = rc.top + (pt.y - g_dragStart.y);
                
                SetWindowPos(hwnd, nullptr, newX, newY, 0, 0, 
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                
                g_dragStart = pt;
                
                g_customPosition = true;
                g_savedX = newX;
                g_savedY = newY;
            }
            return 0;
        }
        
        case WM_LBUTTONUP: {
            if (g_dragging) {
                g_dragging = false;
                ReleaseCapture();
                SaveConfig();
            }
            return 0;
        }
        
        case WM_ACTIVATE:
            return 0;
        
        case WM_MOUSEACTIVATE:
            // Don't activate window on click - this prevents stealing focus!
            return MA_NOACTIVATE;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CreateButtons() {
    // Clear old buttons
    for (int i = 0; i < 32; i++) {
        if (g_hButtons[i]) {
            DestroyWindow(g_hButtons[i]);
            g_hButtons[i] = nullptr;
        }
    }
    
    if (g_currentMode == MODE_CLASSIC) {
        // Classic mode: 2 rows of 5 digits
        // Row 1: 1 2 3 4 5
        // Row 2: 6 7 8 9 0
        int digitOrder[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
        
        for (int i = 0; i < 10; i++) {
            int row = i / 5;
            int col = i % 5;
            int digitIndex = digitOrder[i];
            
            int x = POPUP_PADDING + col * (BUTTON_SIZE + BUTTON_MARGIN);
            int y = POPUP_PADDING + row * (BUTTON_SIZE + BUTTON_MARGIN);
            
            HWND hBtn = CreateWindowW(
                L"NumpadButton", L"",
                WS_CHILD | WS_VISIBLE,
                x, y, BUTTON_SIZE, BUTTON_SIZE,
                g_hPopup, (HMENU)(INT_PTR)(ID_BUTTON_BASE + digitIndex),
                GetModuleHandle(nullptr), nullptr
            );
            
            SetWindowLongPtr(hBtn, GWLP_USERDATA, ID_BUTTON_BASE + digitIndex);
            g_hButtons[digitIndex] = hBtn;
        }
    } else {
        // Advanced mode: 5 columns x 4 rows
        // Row 1 (Sym 0-4): ! @ # $ &
        // Row 2 (Dig 0-4): 1 2 3 4 5
        // Row 3 (Sym 5-9): * ( ) % ^
        // Row 4 (Dig 5-9): 6 7 8 9 0
        
        // Symbols row 1
        for (int i = 0; i < 5; i++) {
            int symbolIndex = i; // 0-4
            int x = POPUP_PADDING + i * (BUTTON_SIZE + BUTTON_MARGIN);
            int y = POPUP_PADDING + 0 * (BUTTON_SIZE + BUTTON_MARGIN);
            
            HWND hBtn = CreateWindowW(
                L"NumpadButton", L"", WS_CHILD | WS_VISIBLE,
                x, y, BUTTON_SIZE, BUTTON_SIZE,
                g_hPopup, (HMENU)(INT_PTR)(ID_BUTTON_BASE + NUM_DIGITS + symbolIndex),
                GetModuleHandle(nullptr), nullptr
            );
            SetWindowLongPtr(hBtn, GWLP_USERDATA, ID_BUTTON_BASE + NUM_DIGITS + symbolIndex);
            g_hButtons[NUM_DIGITS + symbolIndex] = hBtn;
        }

        // Digits row 1
        int digitRow1[] = {1, 2, 3, 4, 5};
        for (int i = 0; i < 5; i++) {
            int digitIndex = digitRow1[i];
            int x = POPUP_PADDING + i * (BUTTON_SIZE + BUTTON_MARGIN);
            int y = POPUP_PADDING + 2 * (BUTTON_SIZE + BUTTON_MARGIN);
            
            HWND hBtn = CreateWindowW(
                L"NumpadButton", L"", WS_CHILD | WS_VISIBLE,
                x, y, BUTTON_SIZE, BUTTON_SIZE,
                g_hPopup, (HMENU)(INT_PTR)(ID_BUTTON_BASE + digitIndex),
                GetModuleHandle(nullptr), nullptr
            );
            SetWindowLongPtr(hBtn, GWLP_USERDATA, ID_BUTTON_BASE + digitIndex);
            g_hButtons[digitIndex] = hBtn;
        }

        // Symbols row 2
        for (int i = 0; i < 5; i++) {
            int symbolIndex = 5 + i; // 5-9
            int x = POPUP_PADDING + i * (BUTTON_SIZE + BUTTON_MARGIN);
            int y = POPUP_PADDING + 1 * (BUTTON_SIZE + BUTTON_MARGIN);
            
            HWND hBtn = CreateWindowW(
                L"NumpadButton", L"", WS_CHILD | WS_VISIBLE,
                x, y, BUTTON_SIZE, BUTTON_SIZE,
                g_hPopup, (HMENU)(INT_PTR)(ID_BUTTON_BASE + NUM_DIGITS + symbolIndex),
                GetModuleHandle(nullptr), nullptr
            );
            SetWindowLongPtr(hBtn, GWLP_USERDATA, ID_BUTTON_BASE + NUM_DIGITS + symbolIndex);
            g_hButtons[NUM_DIGITS + symbolIndex] = hBtn;
        }

        // Digits row 2
        int digitRow2[] = {6, 7, 8, 9, 0};
        for (int i = 0; i < 5; i++) {
            int digitIndex = digitRow2[i];
            int x = POPUP_PADDING + i * (BUTTON_SIZE + BUTTON_MARGIN);
            int y = POPUP_PADDING + 3 * (BUTTON_SIZE + BUTTON_MARGIN);
            
            HWND hBtn = CreateWindowW(
                L"NumpadButton", L"", WS_CHILD | WS_VISIBLE,
                x, y, BUTTON_SIZE, BUTTON_SIZE,
                g_hPopup, (HMENU)(INT_PTR)(ID_BUTTON_BASE + digitIndex),
                GetModuleHandle(nullptr), nullptr
            );
            SetWindowLongPtr(hBtn, GWLP_USERDATA, ID_BUTTON_BASE + digitIndex);
            g_hButtons[digitIndex] = hBtn;
        }
    }
}

void RecreatePopup() {
    bool wasVisible = g_popupVisible;
    
    if (wasVisible) {
        HidePopupWindow();
    }
    
    SetWindowPos(g_hPopup, nullptr, 0, 0, GetPopupWidth(), GetPopupHeight(), 
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    
    CreateButtons();
    SaveConfig();
    
    if (wasVisible) {
        ShowPopupWindow();
    }
}

void ShowPopupWindow() {
    if (g_popupVisible) return;
    
    // Remember the currently active window BEFORE showing popup
    g_lastActiveWindow = GetForegroundWindow();
    
    int popupW = GetPopupWidth();
    int popupH = GetPopupHeight();
    int x, y;
    
    if (g_customPosition) {
        x = g_savedX;
        y = g_savedY;
    } else {
        APPBARDATA abd = {sizeof(abd)};
        SHAppBarMessage(ABM_GETTASKBARPOS, &abd);
        
        POINT pt;
        GetCursorPos(&pt);
        
        switch (abd.uEdge) {
            case ABE_BOTTOM:
                x = pt.x - popupW / 2;
                y = abd.rc.top - popupH;
                break;
            case ABE_TOP:
                x = pt.x - popupW / 2;
                y = abd.rc.bottom;
                break;
            case ABE_LEFT:
                x = abd.rc.right;
                y = pt.y - popupH / 2;
                break;
            case ABE_RIGHT:
                x = abd.rc.left - popupW;
                y = pt.y - popupH / 2;
                break;
            default:
                RECT workArea;
                SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
                x = pt.x - popupW / 2;
                y = workArea.bottom - popupH;
                break;
        }
    }
    
    RECT screenRect;
    GetWindowRect(GetDesktopWindow(), &screenRect);
    if (x < 0) x = 0;
    if (x + popupW > screenRect.right) x = screenRect.right - popupW;
    if (y < 0) y = 0;
    if (y + popupH > screenRect.bottom) y = screenRect.bottom - popupH;
    
    // Use ShowWindow with SW_SHOWNOACTIVATE to not steal focus
    SetWindowPos(g_hPopup, HWND_TOPMOST, x, y, popupW, popupH, SWP_NOACTIVATE);
    ShowWindow(g_hPopup, SW_SHOWNOACTIVATE);
    
    g_popupVisible = true;
}

void HidePopupWindow() {
    if (!g_popupVisible) return;
    
    ShowWindow(g_hPopup, SW_HIDE);
    g_popupVisible = false;
}
