#pragma once



#include <windows.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <string>

#pragma comment(lib, "dwmapi.lib")

// Window messages
#define WM_TRAYICON (WM_USER + 1)
#define IDI_TRAYICON 1

// Menu IDs
#define ID_EXIT 1000
#define ID_MODE_CLASSIC 1001
#define ID_MODE_ADVANCED 1002
#define ID_RESET_POSITION 1003
#define ID_BUTTON_BASE 2000

// Modes
enum Mode { MODE_CLASSIC, MODE_ADVANCED };

// Button dimensions
const int BUTTON_SIZE = 40;
const int BUTTON_MARGIN = 4;
const int POPUP_PADDING = 12;

// Colors (Windows 11 dark theme)
const COLORREF BG_COLOR = RGB(32, 32, 32);
const COLORREF BUTTON_COLOR = RGB(55, 55, 55);
const COLORREF BUTTON_HOVER = RGB(70, 70, 70);
const COLORREF BUTTON_PRESS = RGB(45, 45, 45);
const COLORREF TEXT_COLOR = RGB(255, 255, 255);
const COLORREF SYMBOL_COLOR = RGB(160, 160, 255);

// Key definitions for all buttons
// Classic mode: 0-9 (10 buttons, 2 rows of 5)
// Advanced mode: 0-9 + 12 symbols (22 buttons total)

struct KeyDef {
    wchar_t display;    // Character to display
    WORD vk;            // Virtual key code
    bool shift;         // Needs shift?
};

// Digits 0-9
const KeyDef DIGITS[] = {
    {L'0', '0', false},
    {L'1', '1', false},
    {L'2', '2', false},
    {L'3', '3', false},
    {L'4', '4', false},
    {L'5', '5', false},
    {L'6', '6', false},
    {L'7', '7', false},
    {L'8', '8', false},
    {L'9', '9', false},
};

// Symbols (Shift+number and others)
const KeyDef SYMBOLS[] = {
    {L'!', '1', true},
    {L'@', '2', true},
    {L'#', '3', true},
    {L'$', '4', true},
    {L'&', '7', true},
    {L'*', '8', true},
    {L'(', '9', true},
    {L')', '0', true},
    {L'%', '5', true},
    {L'^', '6', true},
};

const int NUM_DIGITS = 10;
const int NUM_SYMBOLS = 10;

// Global state
extern Mode g_currentMode;
extern HWND g_hPopup;
extern HWND g_hHidden;
extern HWND g_hButtons[];
extern NOTIFYICONDATA g_nid;
extern HFONT g_hFont;
extern HFONT g_hSymbolFont;
extern bool g_popupVisible;
extern int g_hoveredButton;
extern int g_pressedButton;
extern bool g_customPosition;
extern int g_savedX;
extern int g_savedY;
extern HWND g_lastActiveWindow;

// Config functions
std::wstring GetConfigPath();
void LoadConfig();
void SaveConfig();
void ResetPosition();

// Popup dimensions
int GetPopupWidth();
int GetPopupHeight();

// Hotkeys
#define ID_TOGGLE_HOTKEYS 3000
extern bool g_hotkeysEnabled;
