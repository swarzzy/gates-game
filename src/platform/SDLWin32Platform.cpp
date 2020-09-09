#include "SDLWin32Platform.h"

// Enforcing unicode
#if !defined(UNICODE)
#define UNICODE
#endif
#if !defined(_UNICODE)
#define _UNICODE
#endif

// Forcing the app to use discrete graphics adpter by default
#if defined (DISCRETE_GRAPHICS_DEFAULT)
extern "C" { __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; }
extern "C" { __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x01; }
#endif

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

f64 GetTimeStamp() {
    f64 time = 0.0;
    LARGE_INTEGER currentTime = {};
    if (QueryPerformanceCounter(&currentTime)) {
        time = (f64)(currentTime.QuadPart) / (f64)GlobalContext.performanceFrequency.QuadPart;
    }
    return time;
}

u32 DebugGetFileSize(const char* filename) {
    u32 fileSize = 0;
    HANDLE handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (handle != INVALID_HANDLE_VALUE) {
        DWORD sz = (u32)GetFileSize(handle, 0);
        if (sz != INVALID_FILE_SIZE) {
            fileSize = (u32)sz;
        }
        CloseHandle(handle);
    } else {
        log_print("[Platform] File open error %llu\n", (unsigned long)GetLastError());
    }
    return fileSize;
}

u32 DebugReadFileToBuffer(void* buffer, u32 bufferSize, const char* filename) {
    u32 written = 0;
    LARGE_INTEGER fileSize = {0};
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE) {
        if (GetFileSizeEx(fileHandle, &fileSize)) {
            DWORD readSize = (DWORD)bufferSize;
            if (fileSize.QuadPart < bufferSize) {
                readSize = (DWORD)fileSize.QuadPart;
            }
            if (buffer) {
                DWORD read;
                BOOL result = ReadFile(fileHandle, buffer, readSize, &read, 0);
                if (!result && !(read == readSize)) {
                    log_print("[Platform] Failed to open file");
                } else {
                    written = read;
                }
            }
        }
        CloseHandle(fileHandle);
    }
    return written;
}

// TODO:(swarzzy) Rewrite rhis to match Unix implementation behavior
u32 DebugReadTextFileToBuffer(void* buffer, u32 bufferSize, const char* filename) {
    u32 bytesRead = 0;
    char* string = nullptr;
    LARGE_INTEGER fileSize = {};
    HANDLE fileHandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE) {
        if (GetFileSizeEx(fileHandle, &fileSize)) {
            if (fileSize.QuadPart + 1 > bufferSize) {
                log_print("[Platform] Failed to open file");
                CloseHandle(fileHandle);
                return 0;
            } if (buffer) {
                DWORD read;
                BOOL result = ReadFile(fileHandle, buffer,
                                       (DWORD)fileSize.QuadPart, &read, 0);
                if (!result && !(read == (DWORD)fileSize.QuadPart)) {
                    log_print("[Platform] Failed to open file");
                    return 0;
                } else {
                    ((char*)buffer)[fileSize.QuadPart] = '\0';
                    bytesRead = (u32)fileSize.QuadPart + 1;
                }
            }
        }
        CloseHandle(fileHandle);
    }
    return bytesRead;
}

b32 DebugWriteFile(const char* filename, void* data, u32 dataSize) {
    HANDLE fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if (fileHandle != INVALID_HANDLE_VALUE) {
        DWORD bytesWritten;
        BOOL writeResult = WriteFile(fileHandle, data,
                                     dataSize, &bytesWritten, 0);
        if (writeResult && (dataSize == bytesWritten)) {
            CloseHandle(fileHandle);
            return true;
        }
        // TODO(swarzzy): Logging
    }
    CloseHandle(fileHandle);
    return false;
}

FileHandle DebugOpenFile(const char* filename) {
    FileHandle result = InvalidFileHandle;
    HANDLE w32Handle = CreateFileA(filename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, 0, CREATE_NEW, 0, 0);
    if (w32Handle != INVALID_HANDLE_VALUE) {
        result = (FileHandle)w32Handle;
    }
    return result;
}

b32 DebugCloseFile(FileHandle handle) {
    bool result = false;
    if (CloseHandle((HANDLE)handle)) {
        result = true;
    }
    return result;
}

u32 DebugWriteToOpenedFile(FileHandle handle, void* data, u32 size) {
    u32 result = 0;
    DWORD bytesWritten;
    BOOL writeResult = WriteFile((HANDLE)handle, data, size, &bytesWritten, 0);
    if (writeResult && (size == bytesWritten)) {
        result = size;
    }
    return result;
}

b32 DebugCopyFile(const char* source, const char* dest, b32 overwrite) {
    BOOL failIfExists = overwrite ? FALSE : TRUE;
    auto result = CopyFileA(source, dest, failIfExists);
    return (b32)result;
}

#if defined(COMPILER_MSVC)
__declspec(restrict)
#endif
void* Allocate(uptr size, uptr alignment, void* data) {
    if (alignment == 0) {
        alignment = 16;
    }
    auto memory = _aligned_malloc(size, alignment);
    assert(memory);
    //log_print("[Platform] Allocate %llu bytes at address %llu\n", size, (u64)memory);
    return memory;
}

void Deallocate(void* ptr, void* data) {
    //log_print("[Platform] Deallocating memory at address %llu\n", (u64)ptr);
    _aligned_free(ptr);
}

#if defined(COMPILER_MSVC)
__declspec(restrict)
#endif
void* Reallocate(void* ptr, uptr newSize, void* allocatorData) {
    return realloc(ptr, newSize);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
#if defined(ENABLE_CONSOLE)
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
#endif

    auto context = &GlobalContext;

    // NOTE(swarzzy): Setting granularity of windows scheduler
    // so Sleep will work with more granular value
    UINT sleepGranularityMs = 1;
    auto granularityWasSet = (timeBeginPeriod(sleepGranularityMs) == TIMERR_NOERROR);

    QueryPerformanceFrequency(&context->performanceFrequency);

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
#include "Win32CodeLoader.cpp"
