#pragma once

// NOTE: This file is shared by both platform layer and game library
// It defines plaform API. When platform calls game code it passes PlatformState
// structure to the game. This structure contains pointers to all platform functions
// (including OpenGL calls) and other necessary stuff such as current input state.

#include "Common.h"
#include "Intrinsics.h"
#include "Math.h"

struct OpenGL;

#if defined(PLATFORM_WINDOWS)
#define GAME_CODE_ENTRY __declspec(dllexport)
#elif defined(PLATFORM_LINUX)
#define GAME_CODE_ENTRY
#else
#error Unsupported OS
#endif

enum struct GameInvoke : u32
{
    Init, Reload, Update, Render
};

#if defined(PLATFORM_WINDOWS)
typedef uptr FileHandle;
const FileHandle InvalidFileHandle = Uptr::Max;
#else
typedef int FileHandle;
const FileHandle InvalidFileHandle = -1;
#endif


// For accessing in-game resources just char versoins will be enough
typedef u32(DebugGetFileSizeFn)(const char* filename);

// Read file contents to buffer of size bufferSize. If buffer is too small, it will
// read only that number of bits which will fit in the buffer.
// Returns number of bytes written to the buffer
typedef u32(DebugReadFileFn)(void* buffer, u32 bufferSize, const char* filename);

// Read whole file to buffer and null terminate it
typedef u32(DebugReadTextFileFn)(void* buffer, u32 bufferSize, const char* filename);
typedef b32(DebugWriteFileFn)(const char* filename, void* data, u32 dataSize);
typedef b32(DebugCopyFileFn)(const char* source, const char* dest, b32 overwrite);

typedef FileHandle(DebugOpenFileFn)(const char* filename);
typedef b32(DebugCloseFileFn)(FileHandle handle);
typedef u32(DebugWriteToOpenedFileFn)(FileHandle handle, void* data, u32 size);

// NOTE: Functions that platform passes to the game
struct PlatformCalls
{
    DebugGetFileSizeFn* DebugGetFileSize;
    DebugReadFileFn* DebugReadFile;
    DebugReadTextFileFn* DebugReadTextFile;
    DebugWriteFileFn* DebugWriteFile;
    DebugOpenFileFn* DebugOpenFile;
    DebugCloseFileFn* DebugCloseFile;
    DebugCopyFileFn* DebugCopyFile;
    DebugWriteToOpenedFileFn* DebugWriteToOpenedFile;

    // Default allocator
    AllocateFn* Allocate;
    DeallocateFn* Deallocate;
    ReallocateFn* Reallocate;
};

struct KeyState
{
    // TODO(swarzzy): u8
    // TODO(swarzzy): Repeat count
    b32 pressedNow;
    b32 wasPressed;
};

struct MouseButtonState
{
    // TODO: u8
    b32 pressedNow;
    b32 wasPressed;
};

enum struct MouseButton : u8
{
    Left = 0, Right, Middle, XButton1, XButton2
};

enum struct Key : u8
{
    Invalid = 0x00,
    LeftCtrl,
    RightCtrl,
    LeftShift,
    RightShift,
    LeftAlt,
    RightAlt,
    Space,
    Apostrophe,
    Comma,
    Minus,
    Period,
    Slash,
    _0 = 0x30,
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,
    _9,
    Semicolon,
    Equal,
    A = 0x41,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    LeftBracket,
    BackSlash,
    RightBracket,
    Tilde,
    Escape,
    Enter,
    Tab,
    Backspace,
    Insert,
    Delete,
    Right,
    Left,
    Down,
    Up,
    PageUp,
    PageDown,
    Home,
    End,
    CapsLock,
    ScrollLock,
    NumLock,
    PrintScreen,
    Pause,
    F1 = 114,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    Num0,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    NumDecimal,
    NumDivide,
    NumMultiply,
    NumSubtract,
    NumAdd,
    LeftSuper,
    Menu,
    RightSuper,
    Clear,
    NumEnter,
};

// NOTE: Input state that also gets passed to the game by platform
struct InputState
{
    static const u32 KeyCount = 256;
    static const u32 MouseButtonCount = 5;
    KeyState keys[KeyCount];
    MouseButtonState mouseButtons[MouseButtonCount];
    b32 mouseInWindow;
    b32 activeApp;
    // NOTE: All mouse position values are normalized
    f32 mouseX;
    f32 mouseY;
    f32 mouseFrameOffsetX;
    f32 mouseFrameOffsetY;
    // NOTE: Not normalized
    i32 scrollOffset;
    i32 scrollFrameOffset;
};

struct PlatformState
{
    PlatformCalls functions;
    OpenGL* gl;
    InputState input;
    u64 tickCount;
    i32 fps;
    i32 ups;
    f32 deltaTime;
    u32 windowWidth;
    u32 windowHeight;
};

inline const char* ToString(Key keycode) {
    switch (keycode) {
    case Key::Backspace: { return "Backspace";  }
    case Key::Tab: { return "Tab";  }
    case Key::Clear: { return "Clear";  }
    case Key::Enter: { return "Enter";  }
    case Key::LeftShift: { return "LeftShift";  }
    case Key::LeftCtrl: { return "LeftCtrl";  }
    case Key::LeftAlt: { return "LeftAlt";  }
    case Key::RightShift: { return "RightShift";  }
    case Key::RightCtrl: { return "RightCtrl";  }
    case Key::RightAlt: { return "RightAlt";  }
    case Key::Pause: { return "Pause";  }
    case Key::CapsLock: { return "CapsLock";  }
    case Key::Escape: { return "Escape";  }
    case Key::Space: { return "Space";  }
    case Key::PageUp: { return "PageUp";  }
    case Key::PageDown: { return "PageDown";  }
    case Key::End: { return "End";  }
    case Key::Home: { return "Home";  }
    case Key::Left: { return "Left";  }
    case Key::Up: { return "Up";  }
    case Key::Right: { return "Right";  }
    case Key::Down: { return "Down";  }
    case Key::PrintScreen: { return "PrintScreen";  }
    case Key::Insert: { return "Insert";  }
    case Key::Delete: { return "Delete";  }
    case Key::_0: { return "0";  }
    case Key::_1: { return "1";  }
    case Key::_2: { return "2";  }
    case Key::_3: { return "3";  }
    case Key::_4: { return "4";  }
    case Key::_5: { return "5";  }
    case Key::_6: { return "6";  }
    case Key::_7: { return "7";  }
    case Key::_8: { return "8";  }
    case Key::_9: { return "9";  }
    case Key::A: { return "A";  }
    case Key::B: { return "B";  }
    case Key::C: { return "C";  }
    case Key::D: { return "D";  }
    case Key::E: { return "E";  }
    case Key::F: { return "F";  }
    case Key::G: { return "G";  }
    case Key::H: { return "H";  }
    case Key::I: { return "I";  }
    case Key::J: { return "J";  }
    case Key::K: { return "K";  }
    case Key::L: { return "L";  }
    case Key::M: { return "M";  }
    case Key::N: { return "N";  }
    case Key::O: { return "O";  }
    case Key::P: { return "P";  }
    case Key::Q: { return "Q";  }
    case Key::R: { return "R";  }
    case Key::S: { return "S";  }
    case Key::T: { return "T";  }
    case Key::U: { return "U";  }
    case Key::V: { return "V";  }
    case Key::W: { return "W";  }
    case Key::X: { return "X";  }
    case Key::Y: { return "Y";  }
    case Key::Z: { return "Z";  }
    case Key::LeftSuper: { return "LeftSuper";  }
    case Key::RightSuper: { return "RightSuper";  }
    case Key::Num0: { return "Num0";  }
    case Key::Num1: { return "Num1";  }
    case Key::Num2: { return "Num2";  }
    case Key::Num3: { return "Num3";  }
    case Key::Num4: { return "Num4";  }
    case Key::Num5: { return "Num5";  }
    case Key::Num6: { return "Num6";  }
    case Key::Num7: { return "Num7";  }
    case Key::Num8: { return "Num8";  }
    case Key::Num9: { return "Num9";  }
    case Key::NumMultiply: { return "NumMultiply";  }
    case Key::NumAdd: { return "NumAdd";  }
    case Key::NumSubtract: { return "NumSubtract";  }
    case Key::NumDecimal: { return "NumDecimal";  }
    case Key::NumDivide: { return "NumDivide";  }
    case Key::F1: { return "F1";  }
    case Key::F2: { return "F2";  }
    case Key::F3: { return "F3";  }
    case Key::F4: { return "F4";  }
    case Key::F5: { return "F5";  }
    case Key::F6: { return "F6";  }
    case Key::F7: { return "F7";  }
    case Key::F8: { return "F8";  }
    case Key::F9: { return "F9";  }
    case Key::F10: { return "F10";  }
    case Key::F11: { return "F11";  }
    case Key::F12: { return "F12";  }
    case Key::F13: { return "F13";  }
    case Key::F14: { return "F14";  }
    case Key::F15: { return "F15";  }
    case Key::F16: { return "F16";  }
    case Key::F17: { return "F17";  }
    case Key::F18: { return "F18";  }
    case Key::F19: { return "F19";  }
    case Key::F20: { return "F20";  }
    case Key::F21: { return "F21";  }
    case Key::F22: { return "F22";  }
    case Key::F23: { return "F23";  }
    case Key::F24: { return "F24";  }
    case Key::NumLock: { return "NumLock";  }
    case Key::ScrollLock: { return "ScrollLock";  }
    case Key::Menu: { return "Menu";  }
    case Key::Semicolon: { return "Semicolon";  }
    case Key::Equal: { return "Equal";  }
    case Key::Comma: { return "Comma";  }
    case Key::Minus: { return "Minus";  }
    case Key::Period: { return "Period";  }
    case Key::Slash: { return "Slash";  }
    case Key::Tilde: { return "Tilde";  }
    case Key::LeftBracket: { return "LeftBracket";  }
    case Key::BackSlash: { return "BackSlash";  }
    case Key::RightBracket: { return "RightBracket";  }
    case Key::Apostrophe: { return "Apostrophe";  }
    case Key::NumEnter: { return "NumEnter";  }
    default: { return "Invalid"; }
    }
}

inline const char* ToString(MouseButton button) {
    switch (button) {
    case MouseButton::Left:     return "Left";
    case MouseButton::Right:    return "Right";
    case MouseButton::Middle:   return "Middle";
    case MouseButton::XButton1: return "XButton1";
    case MouseButton::XButton2: return "XButton2";
        invalid_default();
    }
    // Dummy return for the compiler
    return "Unknown";
}
