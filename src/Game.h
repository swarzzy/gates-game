#pragma once

#include "Common.h"
#include "Platform.h"
#include "Console.h"
#include "Draw.h"
#include "Canvas.h"
#include "Desk.h"
#include "Tools.h"
#include "Language.h"
#include "Assets.h"

enum struct DrawMode {
    Normal = 0, DeskDebug
};

enum struct GameState {
    Menu, Desk
};

struct GameContext {
    GameState gameState;

    Canvas menuCanvas;
    b32 hitNewGame;
    b32 hitExit;
    b32 hitExitDesk;
    b32 hitLanguage;
    Language language;

    PlatformHeap* mainHeap;
    Allocator mainAllocator;

    Logger logger;
    Console console;
    bool consoleEnabled;
    //DrawList drawList;
    DrawMode drawMode;

    Desk* desk;

    TextureID testTexture;
    TextureID fontAtlas;
    Font font;
    Font sdfFont;
    PartInfo partInfo;

    iv2 prevMouseDeskPos;

    char32* strings[(u32)Strings::_Count];
};

void GameInit();
void GameReload();
void GameUpdate();
void GameSim();
void GameRender();

void GameUpdateDesk();
void GameSimDesk();
void GameRenderDesk();

void GameUpdateMenu();
void GameRenderMenu();


// Getters for global variables
// Implemented in GameEntry.cpp
const PlatformState* GetPlatform();
GameContext* GetContext();
const InputState* GetInput();
bool ImGuiAvailable();

Desk* GetDesk() { return GetContext()->desk; }

void* HeapAllocAPI(uptr size, b32 clear, uptr alignment, void* data);
void HeapFreeAPI(void* ptr, void* data);

// Helpers for input handling
inline bool KeyDown(Key key) { return GetInput()->keys[(u32)key].pressedNow; }
inline bool KeyPressed(Key key) { return (GetInput()->keys[(u32)key].pressedNow) && !(GetInput()->keys[(u32)key].wasPressed); }
inline bool KeyReleased(Key key) { return (GetInput()->keys[(u32)key].wasPressed) && !(GetInput()->keys[(u32)key].pressedNow); }
inline bool MouseButtonDown(MouseButton button) { return GetInput()->mouseButtons[(u32)button].pressedNow; }
inline bool MouseButtonPressed(MouseButton button) { return GetInput()->mouseButtons[(u32)button].pressedNow && !GetInput()->mouseButtons[(u32)button].wasPressed; }
inline bool MouseButtonReleased(MouseButton button) { return GetInput()->mouseButtons[(u32)button].wasPressed && !GetInput()->mouseButtons[(u32)button].pressedNow; }
