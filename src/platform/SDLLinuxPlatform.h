#pragma once

#include "SDL.h"

#include "LinuxCodeLoader.h"

#define DISCRETE_GRAPHICS_DEFAULT

#define OPENGL_MAJOR_VERSION 3
#define OPENGL_MINOR_VERSION 3

const u32 DefaultWindowWidth = 1280;
const u32 DefaultWindowHeight = 720;

struct Win32Context {
    PlatformState state;

    SDLContext sdl;

    LibraryData gameLib;
};

f64 GetTimeStamp();
u32 DebugGetFileSize(const wchar_t* filename);
u32 DebugReadFileToBuffer(void* buffer, u32 bufferSize, const wchar_t* filename);
u32 DebugReadTextFileToBuffer(void* buffer, u32 bufferSize, const wchar_t* filename);
b32 DebugWriteFile(const wchar_t* filename, void* data, u32 dataSize);
FileHandle DebugOpenFile(const wchar_t* filename);
b32 DebugCloseFile(FileHandle handle);
u32 DebugWriteToOpenedFile(FileHandle handle, void* data, u32 size);
b32 DebugCopyFile(const wchar_t* source, const wchar_t* dest, bool overwrite);
