#include "numpad.h"

void EnableModernStyle(HWND hwnd) {
    BOOL useDarkMode = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));
    
    int cornerPreference = 2;
    DwmSetWindowAttribute(hwnd, 33, &cornerPreference, sizeof(cornerPreference));
    
    int backdropType = 2;
    DwmSetWindowAttribute(hwnd, 38, &backdropType, sizeof(backdropType));
}

void PaintButton(HWND hwnd, HDC hdc, wchar_t ch, bool isSymbol, bool isHovered, bool isPressed) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    
    COLORREF bgColor = isPressed ? BUTTON_PRESS : (isHovered ? BUTTON_HOVER : BUTTON_COLOR);
    
    HBRUSH hBrush = CreateSolidBrush(bgColor);
    HPEN hPen = CreatePen(PS_SOLID, 1, bgColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 8, 8);
    
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hBrush);
    DeleteObject(hPen);
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, isSymbol ? SYMBOL_COLOR : TEXT_COLOR);
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, isSymbol ? g_hSymbolFont : g_hFont);
    wchar_t text[2] = {ch, L'\0'};
    DrawTextW(hdc, text, 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(hdc, hOldFont);
}

void SendKey(WORD vk, bool shift);

LRESULT CALLBACK ButtonWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    int buttonId = (int)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    bool isSymbol = (buttonId >= ID_BUTTON_BASE + NUM_DIGITS);
    int index = buttonId - ID_BUTTON_BASE;
    if (isSymbol) index -= NUM_DIGITS;
    
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rc;
            GetClientRect(hwnd, &rc);
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
            
            HBRUSH hBgBrush = CreateSolidBrush(BG_COLOR);
            FillRect(hdcMem, &rc, hBgBrush);
            DeleteObject(hBgBrush);
            
            wchar_t ch;
            if (isSymbol) {
                ch = SYMBOLS[index].display;
            } else {
                ch = DIGITS[index].display;
            }
            
            bool isHovered = (g_hoveredButton == buttonId);
            bool isPressed = (g_pressedButton == buttonId);
            PaintButton(hwnd, hdcMem, ch, isSymbol, isHovered, isPressed);
            
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
            
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_MOUSEMOVE: {
            if (g_hoveredButton != buttonId) {
                if (g_hoveredButton >= 0) {
                    for (int i = 0; i < 32; i++) {
                        if (g_hButtons[i] && (int)GetWindowLongPtr(g_hButtons[i], GWLP_USERDATA) == g_hoveredButton) {
                            InvalidateRect(g_hButtons[i], nullptr, FALSE);
                            break;
                        }
                    }
                }
                g_hoveredButton = buttonId;
                InvalidateRect(hwnd, nullptr, FALSE);
                
                TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, hwnd, 0};
                TrackMouseEvent(&tme);
            }
            return 0;
        }
        
        case WM_MOUSELEAVE: {
            if (g_hoveredButton == buttonId) {
                g_hoveredButton = -1;
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        }
        
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;

        case WM_LBUTTONDOWN: {
            g_pressedButton = buttonId;
            InvalidateRect(hwnd, nullptr, FALSE);
            SetCapture(hwnd);
            return 0;
        }
        
        case WM_LBUTTONUP: {
            if (g_pressedButton == buttonId) {
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(hwnd, &pt);
                
                RECT rc;
                GetClientRect(hwnd, &rc);
                
                if (PtInRect(&rc, pt)) {
                    if (isSymbol) {
                        SendKey(SYMBOLS[index].vk, SYMBOLS[index].shift);
                    } else {
                        SendKey(DIGITS[index].vk, DIGITS[index].shift);
                    }
                }
            }
            g_pressedButton = -1;
            ReleaseCapture();
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        
        case WM_ERASEBKGND:
            return 1;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void SendKey(WORD vk, bool shift) {
    INPUT input[4] = {0};
    int count = 0;
    
    if (shift) {
        input[count].type = INPUT_KEYBOARD;
        input[count].ki.wVk = VK_SHIFT;
        count++;
    }
    
    input[count].type = INPUT_KEYBOARD;
    input[count].ki.wVk = vk;
    count++;
    
    input[count].type = INPUT_KEYBOARD;
    input[count].ki.wVk = vk;
    input[count].ki.dwFlags = KEYEVENTF_KEYUP;
    count++;
    
    if (shift) {
        input[count].type = INPUT_KEYBOARD;
        input[count].ki.wVk = VK_SHIFT;
        input[count].ki.dwFlags = KEYEVENTF_KEYUP;
        count++;
    }
    
    SendInput(count, input, sizeof(INPUT));
}
