#define __CLANG_FLOAT_H
#include "Common.h"
#include "Platform.h"
#include "Math.h"
#include "Console.h"

#include "../ext/imgui-1.78/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../ext/imgui-1.78/imgui_internal.h"

inline void AssertHandler(void* data, const char* file, const char* func, u32 line, const char* assertStr, const char* fmt, va_list* args) {
    log_print("[Assertion failed] Expression (%s) result is false\nFile: %s, function: %s, line: %d.\n", assertStr, file, func, (int)line);
    if (args) {
        GlobalLogger(GlobalLoggerData, fmt, args);
    }
    debug_break();
}

// Setup logger and assert handler
LoggerFn* GlobalLogger = LogMessageAPI;
void* GlobalLoggerData = nullptr;
AssertHandlerFn* GlobalAssertHandler = AssertHandler;
void* GlobalAssertHandlerData = nullptr;

// Global variables for the game. They should be set every time after game code reloading
static PlatformState* _GlobalPlatformState;

bool _GlobalImGuiAvailable;
bool ImGuiAvailable() { return _GlobalImGuiAvailable; }

#define Platform (*((const PlatformCalls*)(&_GlobalPlatformState->functions)))

#include "Game.h"

// Game context also should be set manually after dll reloading
static GameContext* _GlobalGameContext;

const PlatformState* GetPlatform() { return _GlobalPlatformState; }
GameContext* GetContext() {return _GlobalGameContext; }
const InputState* GetInput() { return &_GlobalPlatformState->input; }

void* HeapAllocAPI(uptr size, b32 clear, uptr alignment, void* data) { return Platform.HeapAlloc((PlatformHeap*)data, (usize)size, clear); }
void HeapFreeAPI(void* ptr, void* data) { Platform.Free(ptr); }

// NOTE: Game DLL entry point. Will be called by the platform layer.
extern "C" GAME_CODE_ENTRY void __cdecl GameUpdateAndRender(PlatformState* platform, GameInvoke invoke, void** data) {
    switch (invoke) {
    case GameInvoke::Init: {
        _GlobalImGuiAvailable = platform->imguiContext ? true : false;

        if (ImGuiAvailable()) {
            IMGUI_CHECKVERSION();
            ImGui::SetAllocatorFunctions(platform->ImGuiAlloc, platform->ImGuiFree, platform->imguiAllocatorData);
            ImGui::SetCurrentContext(platform->imguiContext);
        }

        _GlobalPlatformState = platform;

        auto mainHeap = Platform.CreateHeap();
        assert(mainHeap);

        auto context = (GameContext*)Platform.HeapAlloc(mainHeap, sizeof(GameContext), true);
        assert(context);
        context->mainHeap = mainHeap;
        *data = context;
        _GlobalGameContext = context;

        InitLogger(&context->logger, mainHeap);
        GlobalLoggerData = &context->logger;
        InitConsole(&context->console, &context->logger, mainHeap, context);

        GameInit();
    } break;
    case GameInvoke::Reload: {
        _GlobalImGuiAvailable = platform->imguiContext ? true : false;

        if (ImGuiAvailable()) {
            IMGUI_CHECKVERSION();
            ImGui::SetAllocatorFunctions(platform->ImGuiAlloc, platform->ImGuiFree, platform->imguiAllocatorData);
            ImGui::SetCurrentContext(platform->imguiContext);
        }

        _GlobalGameContext = (GameContext*)*data;
        _GlobalPlatformState = platform;

        GlobalLoggerData = &_GlobalGameContext->logger;

        GameReload();
    } break;
    case GameInvoke::Update: {
        GameUpdate();
    } break;
    case GameInvoke::Render: {

        bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);

        GameRender();
    } break;
    invalid_default();
    }
}

// NOTE(swarzzy): All game .cpp files should be included here
#include "Game.cpp"
#include "Console.cpp"
#include "Draw.cpp"

#include "../ext/imgui-1.78/imconfig.h"
#include "../ext/imgui-1.78/imgui.cpp"
#include "../ext/imgui-1.78/imgui_draw.cpp"
#include "../ext/imgui-1.78/imgui_widgets.cpp"
#include "../ext/imgui-1.78/imgui_demo.cpp"
