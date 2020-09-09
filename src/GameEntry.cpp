#define __CLANG_FLOAT_H
#include "Common.h"
#include "Platform.h"
#include "Math.h"
#include "platform/OpenGL.h"

// NOTE: Logger and assert handler implementation
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

// Setup logger and assert handler
LoggerFn* GlobalLogger = Logger;
void* GlobalLoggerData = nullptr;
AssertHandlerFn* GlobalAssertHandler = AssertHandler;
void* GlobalAssertHandlerData = nullptr;

// Global variables for the game. They should be set every time after game code reloading
static PlatformState* _GlobalPlatformState;

// Shortcuts for OpenGL functions
#define gl_function(func) _GlobalPlatformState->gl->functions.fn. func

#define glClear gl_function(glClear)
#define glClearColor gl_function(glClearColor)
#define glViewport gl_function(glViewport)
#define glGenVertexArrays gl_function(glGenVertexArrays)
#define glBindVertexArray gl_function(glBindVertexArray)
#define glEnable gl_function(glEnable)
#define glDepthFunc gl_function(glDepthFunc)
#define glCullFace gl_function(glCullFace)
#define glFrontFace gl_function(glFrontFace)
#define glGenBuffers gl_function(glGenBuffers)
#define glBindBuffer gl_function(glBindBuffer)
#define glBufferData gl_function(glBufferData)
#define glMapBuffer gl_function(glMapBuffer)
#define glUnmapBuffer gl_function(glUnmapBuffer)
#define glEnableVertexAttribArray gl_function(glEnableVertexAttribArray)
#define glVertexAttribPointer gl_function(glVertexAttribPointer)
#define glUseProgram gl_function(glUseProgram)
#define glDrawElements gl_function(glDrawElements)
#define glCreateShader gl_function(glCreateShader)
#define glShaderSource gl_function(glShaderSource)
#define glCompileShader gl_function(glCompileShader)
#define glGetShaderiv gl_function(glGetShaderiv)
#define glAttachShader gl_function(glAttachShader)
#define glLinkProgram gl_function(glLinkProgram)
#define glDeleteShader gl_function(glDeleteShader)
#define glGetProgramInfoLog gl_function(glGetProgramInfoLog)
#define glCreateProgram gl_function(glCreateProgram)
#define glGetProgramiv gl_function(glGetProgramiv)
#define glGetShaderInfoLog gl_function(glGetShaderInfoLog)
#define glDisable gl_function(glDisable)
#define glGetUniformLocation gl_function(glGetUniformLocation)
#define glUniformMatrix4fv gl_function(glUniformMatrix4fv)
#define glClearDepth gl_function(glClearDepth)
#define glDrawArrays gl_function(glDrawArrays)
#define glLineWidth gl_function(glLineWidth)
// Shortcuts for platform functions
// For declarations see Platform.h
#define platform_call(func) _GlobalPlatformState->functions. func

#define PlatformDebugGetFileSize platform_call(DebugGetFileSize)
#define PlatformDebugReadFile platform_call(DebugReadFile)
#define PlatformDebugReadTextFile platform_call(DebugReadTextFile)
#define PlatformDebugWriteFile platform_call(DebugWriteFile)
#define PlatformDebugOpenFile platform_call(DebugOpenFile)
#define PlatformDebugCloseFile platform_call(DebugCloseFile)
#define PlatformDebugCopyFile platform_call(DebugCopyFile)
#define PlatformDebugWriteToOpenedFile platform_call(DebugWriteToOpenedFile)

// Allocator declaraions are in Common.h
#define PlatformAllocate platform_call(Allocate)
#define PlatformDeallocate platform_call(Deallocate)
#define PlatformReallocate platform_call(Reallocate)

#include "Game.h"

// Game context also should be set manually afted dll reloading
static GameContext* _GlobalGameContext;

const PlatformState* GetPlatform() { return _GlobalPlatformState; }
GameContext* GetContext() {return _GlobalGameContext; }
const InputState* GetInput() { return &_GlobalPlatformState->input; }


// NOTE: Game DLL entry point. Will be called by the platform layer.
extern "C" GAME_CODE_ENTRY void __cdecl GameUpdateAndRender(PlatformState* platform, GameInvoke invoke, void** data) {
    switch (invoke) {
    case GameInvoke::Init: {
        auto context = (GameContext*)platform->functions.Allocate(sizeof(GameContext), alignof(GameContext), nullptr);
        *data = context;
        _GlobalGameContext = context;
        _GlobalPlatformState = platform;
        GameInit();
    } break;
    case GameInvoke::Reload: {
        _GlobalGameContext = (GameContext*)*data;
        _GlobalPlatformState = platform;
        GameReload();
    } break;
    case GameInvoke::Update: {
        GameUpdate();
    } break;
    case GameInvoke::Render: {
        GameRender();
    } break;
    invalid_default();
    }
}

// NOTE(swarzzy): All game .cpp files should be included here
#include "Game.cpp"
#include "RenderQueue.cpp"
#include "Render.cpp"
