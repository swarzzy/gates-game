#define __CLANG_FLOAT_H
#define __STDC_UTF_16__

#include "Common.h"
#include "Globals.h"
#include "Platform.h"
#include "Math.h"
#include "Console.h"
#include "DebugOverlay.h"

#include "../ext/imgui-1.78/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../ext/imgui-1.78/imgui_internal.h"

#include "../ext/json/json.h"

inline void AssertHandler(void* data, const char* file, const char* func, u32 line, const char* assertStr, const char* fmt, va_list* args) {
    log_print("[Assertion failed] Expression (%s) result is false\nFile: %s, function: %s, line: %d.\n", assertStr, file, func, (int)line);
    if (args) {
        GlobalLogger(GlobalLoggerData, fmt, args);
    }
    BreakDebug();
}

LoggerFn* GlobalLogger = LogMessageAPI;
void* GlobalLoggerData = nullptr;
AssertHandlerFn* GlobalAssertHandler = AssertHandler;
void* GlobalAssertHandlerData = nullptr;

static PlatformState* _GlobalPlatformState;

bool _GlobalImGuiAvailable;
bool ImGuiAvailable() { return _GlobalImGuiAvailable; }

#define Platform (*((const PlatformAPI*)(&_GlobalPlatformState->platformAPI)))
#define Renderer (*((const RendererAPI*)(&_GlobalPlatformState->rendererAPI)))

#include "Game.h"

static GameContext* _GlobalGameContext;

const PlatformState* GetPlatform() { return _GlobalPlatformState; }
PlatformState* GetPlatformMutable() { return _GlobalPlatformState; }
GameContext* GetContext() {return _GlobalGameContext; }
const InputState* GetInput() { return &_GlobalPlatformState->input; }

void* HeapAllocAPI(uptr size, b32 clear, uptr alignment, void* data) { return Platform.HeapAlloc((PlatformHeap*)data, (usize)size, clear); }
void HeapFreeAPI(void* ptr, void* data) { Platform.Free(ptr); }

// NOTE: Game DLL entry point. Will be called by the platform layer.
extern "C" GAME_CODE_ENTRY void __cdecl GameUpdateAndRender(PlatformState* platform, GameInvoke invoke) {
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
        _GlobalGameContext = context;

        InitLogger(&context->logger, mainHeap);
        GlobalLoggerData = &context->logger;
        InitConsole(&context->console, &context->logger, mainHeap, context);

        GameInit();
    } break;
    case GameInvoke::Update: {
        GameUpdate();
    } break;
    case GameInvoke::Sim: {
        GameSim();
    } break;
    case GameInvoke::Render: {
        if (KeyPressed(Key::F1)) {
            Global_ShowDebugOverlay = !Global_ShowDebugOverlay;
        }

        BeginDebugOverlay();

        GameRender();
    } break;
    invalid_default();
    }
}

#include "Game.cpp"
#include "Console.cpp"
#include "Draw.cpp"
#include "DebugOverlay.cpp"
#include "Canvas.cpp"
#include "Desk.cpp"
#include "HashMap.cpp"
#include "ConsoleCommands.cpp"
#include "PartInfo.cpp"
#include "Part.cpp"
#include "Tools.cpp"
#include "BucketArray.cpp"
#include "List.cpp"
#include "Array.cpp"
#include "String.cpp"
#include "Position.cpp"
#include "Language.cpp"
#include "Assets.cpp"
#include "StringBuilder.cpp"

#include "Intrinsics.cpp"

#include "../ext/imgui-1.78/imconfig.h"
#include "../ext/imgui-1.78/imgui.cpp"
#include "../ext/imgui-1.78/imgui_draw.cpp"
#include "../ext/imgui-1.78/imgui_widgets.cpp"
#include "../ext/imgui-1.78/imgui_demo.cpp"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#define STBTT_malloc(x,u)   (Platform.HeapAlloc(GetContext()->mainHeap, (usize)(x), false))
#define STBTT_free(x,u)     (Platform.Free(x))
#define STBTT_assert(x)     assert(x)
#include "../ext/stb_truetype-1.24/stb_truetype.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x)            assert(x)
#define STBI_MALLOC(sz)           (Platform.HeapAlloc(GetContext()->mainHeap, (usize)(sz), false))
#define STBI_REALLOC(p,newsz)     (Platform.HeapRealloc(GetContext()->mainHeap, (p), (usize)(newsz), false))
#define STBI_FREE(p)              (Platform.Free(p))

#include "../ext/stb_image-2.26/stb_image.h"
