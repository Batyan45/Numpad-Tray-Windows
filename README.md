# Numpad Tray

![Numpad Icon](icon/numpad.ico)

Windows system tray numpad for when your number keys don't work.

**Origin:** Born from a common Lenovo laptop issue where the keyboard behaves erratically, often leaving only the '5' and '6' keys working. This tool provides a reliable backup.

## Features

- **Tray icon** - left-click to show/hide numpad
- **No focus stealing** - popup doesn't take focus from your editor
- **Draggable** - move anywhere, position is saved
- **Two modes** (right-click menu):
  - **Classic**: 0-9
  - **Advanced**: 0-9 + 10 symbols

## Build

```bash
cmake -B build
cmake --build build --config Release
```

## Usage

| Action | Result |
|--------|--------|
| Left-click tray | Toggle popup |
| Click button | Send keystroke |
| Drag popup | Move (saved) |
| Right-click tray | Menu |

## Layout

**Classic:**
```
[1][2][3][4][5]
[6][7][8][9][0]
```

**Advanced:**
```
[!][@][#][$][&]  <-- Symbols
[*][(][)][%][^]  <-- Symbols
[1][2][3][4][5]  <-- Digits
[6][7][8][9][0]  <-- Digits
```

## Structure

```
numpad/
├── include/numpad.h
├── src/
│   ├── main.cpp
│   ├── config.cpp
│   ├── button.cpp
│   └── popup.cpp
├── icon/
│   └── numpad.ico
├── CMakeLists.txt
└── resources.rc
```
