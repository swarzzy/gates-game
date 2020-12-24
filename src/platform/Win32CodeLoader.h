#pragma once
#include "../Platform.h"
#include <windows.h>

struct PlatformState;
struct Application;

typedef void (__cdecl GameUpdateAndRenderFn)(PlatformState*, GameInvoke);

struct LibraryData
{
    inline static const wchar_t* DllName = L"gates.dll";
    inline static const wchar_t* TempDllName = L"TEMP_gates.dll";
    inline static constexpr u32 MaxPathLen = 256;
    GameUpdateAndRenderFn* GameUpdateAndRender;
    u64 lastChangeTime;
    HMODULE handle;
};

b32 UpdateGameCode(LibraryData* lib);
void UnloadGameCode(LibraryData* lib);
