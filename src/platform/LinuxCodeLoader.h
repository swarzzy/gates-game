#pragma once

#include "../Platform.h"

struct PlatformState;
struct Application;

typedef void (__cdecl GameUpdateAndRenderFn)(PlatformState*, GameInvoke, void** data);

struct LibraryData
{
    inline static const char* LibName = "./pong.so";
    inline static const char* TempLibName = "./TEMP_pong.so";
    inline static constexpr u32 MaxPathLen = 256;
    GameUpdateAndRenderFn* GameUpdateAndRender;
    time_t lastChangeTime;
    void* handle;
};

b32 UpdateGameCode(LibraryData* lib);
void UnloadGameCode(LibraryData* lib);
