#pragma once

#include "Common.h"
#include "Platform.h"
#include "Console.h"
#include "Draw.h"
#include "Canvas.h"
#include "Desk.h"

enum struct CellValue {
    Wire, Element, Input, Output
};

enum struct ElementType {
    And, Or, Not
};

enum struct PinType {
    Input, Output
};

struct ElementPin {
    PinType type;
    iv2 relativeP;
};

ElementPin CreateElementPin(iv2 relP, PinType type) {
    ElementPin pin {};
    pin.type = type;
    pin.relativeP = relP;
    return pin;
}


struct Element {
    DeskPosition p;
    iv2 dim;
    v4 color;
    ElementType type;
    u32 pinCount;
    ElementPin pins[8];
};

Element CreateElement(iv2 p, ElementType type) {
    Element element {};
    element.p = MakeDeskPosition(p);
    element.type = type;

    switch (element.type) {
    case ElementType::And: {
        element.color = V4(0.4f, 0.6f, 0.0f, 1.0f);
        element.dim = IV2(2, 3);
        element.pins[0] = CreateElementPin(IV2(0, 1), PinType::Input);
        element.pins[1] = CreateElementPin(IV2(0, 2), PinType::Input);
        element.pins[2] = CreateElementPin(IV2(2, 2), PinType::Output);
        element.pinCount = 3;
    } break;
    case ElementType::Or: {
        element.color = V4(0.0f, 0.0f, 0.6f, 1.0f);
        element.dim = IV2(2, 3);
        element.pins[0] = CreateElementPin(IV2(0, 1), PinType::Input);
        element.pins[1] = CreateElementPin(IV2(0, 2), PinType::Input);
        element.pins[2] = CreateElementPin(IV2(2, 2), PinType::Output);
        element.pinCount = 3;
    } break;
    case ElementType::Not: {
        element.color = V4(0.6f, 0.0f, 0.0f, 1.0f);
        element.dim = IV2(2, 2);
        element.pins[0] = CreateElementPin(IV2(0, 1), PinType::Input);
        element.pins[1] = CreateElementPin(IV2(2, 1), PinType::Output);
        element.pinCount = 2;

    } break;
        invalid_default();
    }
    return element;
}

struct Cell {
};

struct GameContext {
    PlatformHeap* mainHeap;
    Logger logger;
    Console console;
    bool consoleEnabled;
    //DrawList drawList;
    Canvas deskCanvas;
    TextureID testTexture;
    TextureID fontAtlas;
    Font font;
    Font sdfFont;
    //Cell desk[32][32];
    Element elements[32];
    u32 elementCount;
    v2 deskPosition;
    Desk desk;
    Element ghostElement;
    b32 ghostElementEnabled;
};

void GameInit();
void GameReload();
void GameUpdate();
void GameRender();

// Getters for global variables
// Implemented in GameEntry.cpp
const PlatformState* GetPlatform();
GameContext* GetContext();
const InputState* GetInput();
bool ImGuiAvailable();

LoadImageResult* ResourceLoaderLoadImage(const char* filename, b32 flipY, u32 forceBPP, Allocator allocator);

void* HeapAllocAPI(uptr size, b32 clear, uptr alignment, void* data);
void HeapFreeAPI(void* ptr, void* data);

// Helpers for input handling
inline bool KeyDown(Key key) { return GetInput()->keys[(u32)key].pressedNow; }
inline bool KeyPressed(Key key) { return (GetInput()->keys[(u32)key].pressedNow) && !(GetInput()->keys[(u32)key].wasPressed); }
inline bool MouseButtonDown(MouseButton button) { return GetInput()->mouseButtons[(u32)button].pressedNow; }
inline bool MouseButtonPressed(MouseButton button) { return GetInput()->mouseButtons[(u32)button].pressedNow && !GetInput()->mouseButtons[(u32)button].wasPressed; }
