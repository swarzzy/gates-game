#pragma once

#include "SDL.h"

#include "RendererGL.h"

#include <Windows.h>

// For timeBeginPeriod
#include <mmsystem.h>

#include "../../ext/mimalloc-1.6.4/include/mimalloc.h"

#define ENABLE_CONSOLE

#include "Win32CodeLoader.h"

#define DISCRETE_GRAPHICS_DEFAULT
#define ENABLE_CONSOLE
#define DEBUG_OPENGL

#define USE_IMGUI

#define OPENGL_MAJOR_VERSION 3
#define OPENGL_MINOR_VERSION 0

const u32 DefaultWindowWidth = 1280;
const u32 DefaultWindowHeight = 720;

struct Win32Context {
    PlatformState state;

    SDLContext sdl;

    LARGE_INTEGER performanceFrequency;

    LibraryData gameLib;

    mi_heap_t* imguiHeap;
    mi_heap_t* stbHeap;

    Renderer renderer;
};

OpenGLLoadResult LoadOpenGL();
u32 DebugGetFileSize(const char* filename);
u32 DebugReadFileToBuffer(void* buffer, u32 bufferSize, const char* filename);
u32 DebugReadTextFileToBuffer(void* buffer, u32 bufferSize, const char* filename);
b32 DebugWriteFile(const char* filename, void* data, u32 dataSize);
FileHandle DebugOpenFile(const char* filename);
b32 DebugCloseFile(FileHandle handle);
u32 DebugWriteToOpenedFile(FileHandle handle, void* data, u32 size);
b32 DebugCopyFile(const char* source, const char* dest, b32 overwrite);
