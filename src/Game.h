#pragma once

#include "Common.h"
#include "Platform.h"
#include "Console.h"
#include "Draw.h"
#include "Canvas.h"
#include "Desk.h"
#include "Tools.h"

enum struct DrawMode {
    Normal = 0, DeskDebug
};

struct GameContext {
    PlatformHeap* mainHeap;
    Allocator mainAllocator;
    Logger logger;
    Console console;
    bool consoleEnabled;
    //DrawList drawList;
    DrawMode drawMode;
    Canvas deskCanvas;
    TextureID testTexture;
    TextureID fontAtlas;
    Font font;
    Font sdfFont;
    Desk desk;
    PartInfo partInfo;
    ToolManager toolManager;

    iv2 prevMouseDeskPos;
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

Desk* GetDesk() { return &GetContext()->desk; }

LoadImageResult* ResourceLoaderLoadImage(const char* filename, b32 flipY, u32 forceBPP, Allocator allocator);

void* HeapAllocAPI(uptr size, b32 clear, uptr alignment, void* data);
void HeapFreeAPI(void* ptr, void* data);

// Helpers for input handling
inline bool KeyDown(Key key) { return GetInput()->keys[(u32)key].pressedNow; }
inline bool KeyPressed(Key key) { return (GetInput()->keys[(u32)key].pressedNow) && !(GetInput()->keys[(u32)key].wasPressed); }
inline bool KeyReleased(Key key) { return (GetInput()->keys[(u32)key].wasPressed) && !(GetInput()->keys[(u32)key].pressedNow); }
inline bool MouseButtonDown(MouseButton button) { return GetInput()->mouseButtons[(u32)button].pressedNow; }
inline bool MouseButtonPressed(MouseButton button) { return GetInput()->mouseButtons[(u32)button].pressedNow && !GetInput()->mouseButtons[(u32)button].wasPressed; }
inline bool MouseButtonReleased(MouseButton button) { return GetInput()->mouseButtons[(u32)button].wasPressed && !GetInput()->mouseButtons[(u32)button].pressedNow; }
