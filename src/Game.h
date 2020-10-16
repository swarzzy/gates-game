#pragma once

#include "Common.h"
#include "Platform.h"
#include "Console.h"
#include "Draw.h"
#include "HashMap.h"
#include "String.h"

struct ProcedureName {
    const char* name;
};

// [https://cseweb.ucsd.edu/~kube/cls/100/Lectures/lec16/lec16-16.html]
u32 ProcedureHash(void* value) {
    auto key = (ProcedureName*)value;
    const char* str = key->name;

    i32 hash = 0;
    while (*str != 0) {
        hash = (hash << 4) + *(str++);
        i32 g = hash & 0xf0000000;
        if (g != 0) hash ^= g >> 24;
        hash &= ~g;
    }

    return hash;
}

bool ProcedureCompare(void* aa, void* bb) {
    auto a = (ProcedureName*)aa;
    auto b = (ProcedureName*)bb;
    return StringsAreEqual(a->name, b->name);
}

template <typename Proc>
struct Callback {
    typedef Proc ProcType;
    u32 dispatchTableIndex;
};

#define InvokeCallback(callback, ...) ((decltype(callback)::ProcType*)(GlobalProcedureDispatchTable[callback.dispatchTableIndex]))(##__VA_ARGS__)

typedef void(TestCallbackFn)();

// NOTE: All global game stuff lives here
struct GameContext {
    PlatformHeap* mainHeap;
    Logger logger;
    Console console;
    bool consoleEnabled;
    DrawList drawList;
    TextureID testTexture;
    TextureID fontAtlas;
    Font font;
    Font sdfFont;
    Callback<TestCallbackFn> testCallback;
    HashMap<ProcedureName, u32, ProcedureHash, ProcedureCompare> procedureNameTable;
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

// TODO: Compiler time validation on procedure
#define MakeCallback(procedure) _MakeCallback<decltype(procedure)>(#procedure)
template <typename Proc>
Callback<Proc> _MakeCallback(const char* name) {
    auto context = GetContext();
    ProcedureName procName = { name };
    auto bucket = HashMapGet(&context->procedureNameTable, &procName);
    assert(bucket);
    return Callback<Proc> { *bucket };
}

struct Type {
    const char* name;
};
