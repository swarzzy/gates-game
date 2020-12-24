#include "SDLWin32Platform.h"

#include "ImGui.h"

#include "shellscalingapi.h"

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

PlatformHeap* ResourceLoaderScratchHeap;

void* HeapAllocAPI(uptr size, b32 clear, uptr alignment, void* data) { return HeapAlloc((PlatformHeap*)data, (usize)size, clear); }
void HeapFreeAPI(void* ptr, void* data) { Free(ptr); }

#define GL (((const Win32Context* )&GlobalContext)->sdl.gl.functions.fn)

f64 Win32GetTimeStamp() {
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

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int showCmd)
{
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#if defined(ENABLE_CONSOLE)
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
#endif

    MiMallocInit();

    auto context = &GlobalContext;

    // NOTE(swarzzy): Setting granularity of windows scheduler
    // so Sleep will work with more granular value
    UINT sleepGranularityMs = 1;
    auto granularityWasSet = (timeBeginPeriod(sleepGranularityMs) == TIMERR_NOERROR);

    QueryPerformanceFrequency(&context->performanceFrequency);

    context->state.windowWidth = DefaultWindowWidth;
    context->state.windowHeight = DefaultWindowHeight;

    f32 defaultWindowsDpi = 96.0f;
    f32 cmInInch = 2.54f;
    f32 cmPerPixel = cmInInch / defaultWindowsDpi;
    context->state.pixelsPerCentimeter = 1.0f / cmPerPixel;

    SDLInit(&context->sdl, &context->state, OPENGL_MAJOR_VERSION, OPENGL_MINOR_VERSION);

    // Loading OpenGL
    OpenGLLoadResult glResult = SDLLoadOpenGL(&context->sdl);
    if (!glResult.success) {
        panic("Failed to load OpenGL functions");
    }

    // Initializing ImGui context
    context->imguiHeap = CreateHeap();
    if (!context->imguiHeap) {
        panic("Failed to create heap for Dear ImGui");
    }

    context->platformHeap = CreateHeap();
    if (!context->platformHeap) {
        panic("Failed to create platform heap");
    }

    ResourceLoaderScratchHeap = context->platformHeap;

    context->state.imguiContext = InitImGuiForGL3(context->imguiHeap, context->sdl.window, &context->sdl.glContext);
    if (!context->state.imguiContext) {
        panic("Failed to load Dear ImGui");
    } else {
        context->state.ImGuiAlloc = ImguiAllocWrapper;
        context->state.ImGuiFree = ImguiFreeWrapper;
        context->state.imguiAllocatorData = context->imguiHeap;
    }

    // Setting function pointers to platform routines a for game
    context->state.platformAPI.DebugGetFileSize = DebugGetFileSize;
    context->state.platformAPI.DebugReadFile = DebugReadFileToBuffer;
    context->state.platformAPI.DebugReadTextFile = DebugReadTextFileToBuffer;
    context->state.platformAPI.DebugWriteFile = DebugWriteFile;
    context->state.platformAPI.DebugOpenFile = DebugOpenFile;
    context->state.platformAPI.DebugCloseFile = DebugCloseFile;
    context->state.platformAPI.DebugCopyFile = DebugCopyFile;
    context->state.platformAPI.DebugWriteToOpenedFile = DebugWriteToOpenedFile;

    context->state.platformAPI.CreateHeap = CreateHeap;
    context->state.platformAPI.DestroyHeap = DestroyHeap;
    context->state.platformAPI.HeapAlloc = HeapAlloc;
    context->state.platformAPI.HeapRealloc = HeapRealloc;
    context->state.platformAPI.Free = Free;

    context->state.rendererAPI.RenderDrawList = RenderDrawList;
    context->state.rendererAPI.UploadTexture = RendererUploadTexture;
    context->state.rendererAPI.SetCamera = RenderSetCamera;

    RendererInit(&context->renderer);

    if (!UpdateGameCode(&context->gameLib)) {
        panic("[Platform] Failed to load game library");
    }

    context->state.targetSimStepsPerSecond = 100;

    context->state.vsync = VSyncMode::Disabled;
    VSyncMode vsyncMode = VSyncMode::Disabled;
    SDLSetVsync(VSyncMode::Disabled);

    // Init the game
    context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Init, &GlobalGameData);

    f64 currentTime = Win32GetTimeStamp();
    f64 accumulator = 0.0;
    f64 secondTimer = 0.0;
    u32 simStepsForSec = 0;

    while (context->sdl.running) {
        f64 newTime = Win32GetTimeStamp();
        f64 beginFrameTime = Win32GetTimeStamp();
        f64 timePerSimStep = 1.0 / (context->state.targetSimStepsPerSecond == 0 ? 1 : context->state.targetSimStepsPerSecond);
        f64 frameTime = newTime - currentTime;
        accumulator += frameTime;
        currentTime = newTime;

        secondTimer += frameTime;
        if (secondTimer > 1.0) {
            context->state.simStepsPerSecond = simStepsForSec;
            simStepsForSec = 0;
            secondTimer = 0.0;
        }

        context->state.tickCount++;

        if (context->state.vsync != vsyncMode) {
            vsyncMode = context->state.vsync;
            SDLSetVsync(vsyncMode);
        }

        SDLPollEvents(&context->sdl, &context->state);
        ImGuiNewFrameForGL3(context->sdl.window, context->state.windowWidth, context->state.windowHeight);

        context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Update, &GlobalGameData);


        f64 stepTime = accumulator / timePerSimStep;
        u32 simSteps = (u32)(stepTime);
        accumulator -= simSteps * timePerSimStep;

        f64 simBeginTime = Win32GetTimeStamp();

        for (u32 i = 0; i < simSteps; i++) {
            context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Sim, &GlobalGameData);
            simStepsForSec++;
            context->state.simStepCount++;

            // Abort sim if it takes longer than 100ms
            // TODO: Maybe it is a bad idea to do zilliard of syscalls per frame to get performance counter
            // maybe _rdtsc() can do the trick?
            f64 simCurrTime = Win32GetTimeStamp();
            if ((simCurrTime - simBeginTime) > 0.1f) {
                break;
            }
        }

        context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Render, &GlobalGameData);

        ImGuiEndFrameForGL3();

        // Reload game lib if it was updated
        bool codeReloaded = UpdateGameCode(&context->gameLib);
        if (codeReloaded) {
            log_print("[Platform] Game was hot-reloaded\n");
            context->gameLib.GameUpdateAndRender(&context->state, GameInvoke::Reload, &GlobalGameData);
        }

        //bool show_demo_window = true;
        //ImGui::ShowDemoWindow(&show_demo_window);

        for (u32 keyIndex = 0; keyIndex < array_count(context->state.input.keys); keyIndex ++) {
            context->state.input.keys[keyIndex].wasPressed = context->state.input.keys[keyIndex].pressedNow;
        }

        for (u32 mbIndex = 0; mbIndex < array_count(context->state.input.mouseButtons); mbIndex++) {
            context->state.input.mouseButtons[mbIndex].wasPressed = context->state.input.mouseButtons[mbIndex].pressedNow;
        }

        context->state.input.scrollFrameOffset = 0;
        context->state.input.mouseFrameOffsetX = 0;
        context->state.input.mouseFrameOffsetY = 0;

        SDLSwapBuffers(&context->sdl);


        if (granularityWasSet && vsyncMode == VSyncMode::Disabled && (context->state.targetFramerate > 0)) {
            f64 deltaTime = Win32GetTimeStamp() - beginFrameTime;
            f64 tragetDeltaTime = 1.0 / context->state.targetFramerate;
            while (deltaTime < tragetDeltaTime) {
                DWORD timeToWait = (DWORD)((tragetDeltaTime - deltaTime) * 1000);
                if (timeToWait > 0) {
                    Sleep(timeToWait);
                }
                deltaTime = Win32GetTimeStamp() - beginFrameTime;
            }
        }

        // If framerate lower than 15 fps just clamping delta time
        context->state.deltaTime = (f32)Clamp(frameTime, 0.0, 0.066);
        context->state.framesPerSecond = (i32)(1.0f / frameTime);
        context->state.updatesPerSecond = context->state.framesPerSecond;
    }

    // TODO(swarzzy): Is that necessary?
    SDL_Quit();
    return 0;
}

#include "RendererGL.cpp"
#include "Win32CodeLoader.cpp"

#include "SDL.cpp"
#include "Allocation.cpp"
#include "ImGui.cpp"

#include "../Array.cpp"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#define STBTT_malloc(x,u)   (HeapAlloc(GlobalContext.platformHeap, (usize)(x), false))
#define STBTT_free(x,u)     (Free(x))
#define STBTT_assert(x)     assert(x)
#include "../../ext/stb_truetype-1.24/stb_truetype.h"
