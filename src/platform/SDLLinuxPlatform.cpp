#include "SDLLinuxPlatform.h"

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>

// TODO(swarzzy): DISCRETE_GRAPHICS_DEFAULT
/*
#if defined (DISCRETE_GRAPHICS_DEFAULT)
extern "C" { __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }
extern "C" { __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x01; }
#endif
*/

// TODO: Logger
void Logger(void* data, const char* fmt, va_list* args) {
    vprintf(fmt, *args);
}

inline void AssertHandler(void* data, const char* file, const char* func, u32 line, const char* assertStr, const char* fmt, va_list* args) {
    log_print("[Assertion failed] Expression (%s) result is false\nFile: %s, function: %s, line: %d.\n", assertStr, file, func, (int)line);
    if (args) {
        GlobalLogger(GlobalLoggerData, fmt, args);
    }
    debug_break();
}

LoggerFn* GlobalLogger = Logger;
void* GlobalLoggerData = nullptr;
AssertHandlerFn* GlobalAssertHandler = AssertHandler;
void* GlobalAssertHandlerData = nullptr;

static Win32Context GlobalContext;
static void* GlobalGameData;

// TODO: Check is clock_gettime precise enough and is there more preciese alternatives
f64 GetTimeStamp() {
    f64 time = 0.0;
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &tp) == 0) {
        time = (f64)tp.tv_sec + (tp.tv_nsec / 1000000000.0);
    }
    return time;
}

void* Allocate(uptr size, uptr alignment, void* data) {
    if (alignment == 0) {
        alignment = 16;
    }
    auto memory = memalign(alignment, size);
    assert(memory);
    return memory;
}

void Deallocate(void* ptr, void* data) {
    free(ptr);
}

void* Reallocate(void* ptr, uptr newSize, void* allocatorData) {
    return realloc(ptr, newSize);
}

u32 DebugGetFileSize(const char* filename) {
    u32 size = 0;
    struct stat fileAttribs;
    if (stat(LibraryData::LibName, &fileAttribs) == 0) {
        size = fileAttribs.st_size;
    }
    return size;
}

u32 DebugReadFileToBuffer(void* buffer, u32 bufferSize, const char* filename) {
    u32 written = 0;
    int fileHandle = open(filename, O_RDONLY);
    if (fileHandle) {
        off_t fileEnd = lseek(fileHandle, 0, SEEK_END);
        if (fileEnd) {
            lseek(fileHandle, 0, SEEK_SET);
            u32 readSize = bufferSize;
            if (fileEnd < bufferSize) {
                readSize = fileEnd;
            }
            if (buffer) {
                ssize_t result = read(fileHandle, buffer, readSize);
                if (result == readSize) {
                    written = readSize;
                }
            }
        }
    }
    return written;
}

u32 DebugReadTextFileToBuffer(void* buffer, u32 bufferSize, const char* filename) {
    u32 written = 0;
    int fileHandle = open(filename, O_RDONLY);
    if (fileHandle) {
        off_t fileEnd = lseek(fileHandle, 0, SEEK_END);
        if (fileEnd) {
            lseek(fileHandle, 0, SEEK_SET);
            u32 readSize = bufferSize - 1;
            if ((fileEnd + 1) < bufferSize) {
                readSize = fileEnd;
            }
            if (buffer) {
                ssize_t result = read(fileHandle, buffer, readSize);
                if (result == readSize) {
                    written = (u32)result + 1;
                    ((char*)buffer)[readSize] = '\0';
                }
            }
        }
    }
    return written;
}

b32 DebugWriteFile(const char* filename, void* data, u32 dataSize) {
    b32 result = false;
    int fileHandle = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IRWXU | S_IRGRP);
    if (fileHandle) {
        ssize_t written = write(fileHandle, data, dataSize);
        if (written == dataSize) {
            result = true;
        }
        close(fileHandle);
    }
    return result;
}

FileHandle DebugOpenFile(const char* filename) {
    FileHandle handle = open(filename, O_RDWR | O_CREAT, S_IROTH | S_IRWXU | S_IRUSR | S_IWUSR | S_IRGRP);
    return handle;
}

b32 DebugCloseFile(FileHandle handle) {
    bool result = false;
    if (close(handle) == 0) {
        result = true;
    }
    return result;
}

u32 DebugWriteToOpenedFile(FileHandle handle, void* data, u32 size) {
    auto written = write(handle, data, size);
    if ((written == -1) || written != size) {
        written = 0;
    }
    return written;
}

b32 DebugCopyFile(const char* source, const char* dest, b32 overwrite) {
    b32 result = false;
    auto fileSize = DebugGetFileSize(source);
    if (fileSize) {
        void* data = Allocate(fileSize, 0, nullptr);
        if (data) {
            auto bytesRead = DebugReadFileToBuffer(data, fileSize, source);
            if (bytesRead == fileSize) {
                if (DebugWriteFile(dest, data, fileSize)) {
                    result = true;
                }
            }
            Deallocate(data, nullptr);
        }
    }
    return result;
}

int main() {
    auto context = &GlobalContext;

    context->state.windowWidth = DefaultWindowWidth;
    context->state.windowHeight = DefaultWindowHeight;

    SDLInit(&context->sdl, &context->state, OPENGL_MAJOR_VERSION, OPENGL_MINOR_VERSION);

    // Loading OpenGL
    OpenGLLoadResult glResult = SDLLoadOpenGL();
    if (!glResult.success) {
        panic("Failed to load OpenGL functions");
    }
    context->state.gl = glResult.context;

    // Setting function pointers to platform routines a for game
    context->state.functions.DebugGetFileSize = DebugGetFileSize;
    context->state.functions.DebugReadFile = DebugReadFileToBuffer;
    context->state.functions.DebugReadTextFile = DebugReadTextFileToBuffer;
    context->state.functions.DebugWriteFile = DebugWriteFile;
    context->state.functions.DebugOpenFile = DebugOpenFile;
    context->state.functions.DebugCloseFile = DebugCloseFile;
    context->state.functions.DebugCopyFile = DebugCopyFile;
    context->state.functions.DebugWriteToOpenedFile = DebugWriteToOpenedFile;

    context->state.functions.Allocate = Allocate;
    context->state.functions.Deallocate = Deallocate;
    context->state.functions.Reallocate = Reallocate;

    // Init the game
    if (!UpdateGameCode(&context->gameLib)) {
        panic("[Platform] Failed to load game library");
    }

    // Init the game
    context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Init, &GlobalGameData);

    while (context->sdl.running) {
        auto frameStartTime = GetTimeStamp();
        context->state.tickCount++;

        // Reload game lib if it was updated
        bool codeReloaded = UpdateGameCode(&context->gameLib);
        if (codeReloaded) {
            log_print("[Platform] Game was hot-reloaded\n");
            context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Reload, &GlobalGameData);
        }

        // TODO(swarzzy): For now we do ONE update per frame with variable delta time.
        // This is not a good solution. We probably need to do updates with fixed timestep
        // with higher frequency (for example 120Hz)

        SDLPollEvents(&context->sdl, &context->state);

        context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Update, &GlobalGameData);

        for (u32 keyIndex = 0; keyIndex < InputState::KeyCount; keyIndex ++) {
            context->state.input.keys[keyIndex].wasPressed = context->state.input.keys[keyIndex].pressedNow;
        }

        for (u32 mbIndex = 0; mbIndex < InputState::MouseButtonCount; mbIndex++) {
            context->state.input.mouseButtons[mbIndex].wasPressed = context->state.input.mouseButtons[mbIndex].pressedNow;
        }

        context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Render, &GlobalGameData);

        SDLSwapBuffers(&context->sdl);

        auto frameEndTime = GetTimeStamp();
        auto frameTime = frameEndTime - frameStartTime;

        // If framerate lower than 15 fps just clamping delta time
        context->state.deltaTime = (f32)Clamp(1.0 / frameTime, 0.0, 0.066);
        context->state.fps = (i32)(1.0f / context->state.deltaTime);
        context->state.ups = context->state.fps;
    }

    // TODO(swarzzy): Is that necessary?
    SDL_Quit();
    return 0;
}

#include "SDL.cpp"
#include "LinuxCodeLoader.cpp"
