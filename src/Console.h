#pragma once
#include "Memory.h"
#include "ConsoleCommands.h"

struct LoggerBufferBlock {
    static const u32 Size = 4096;
    static const u32 Capacity = Size - 1;
    LoggerBufferBlock* next;
    LoggerBufferBlock* prev;
    u32 at;
    char data[Size];
};

struct Logger {
    static const u32 BlockBudget = 32;
    PlatformHeap* heap;
    LoggerBufferBlock* firstBlock;
    LoggerBufferBlock* lastBlock;
    LoggerBufferBlock* freeBlock;
    u32 usedBlockCount;
    u32 allocatedBlockCount;
    char formatBuffer[4096];
};

void InitLogger(Logger* logger, PlatformHeap* heap);
void ClearLogger(Logger* logger);

void LoggerPushString(Logger* logger, const char* string);
void LogMessage(Logger* logger, const char* fmt, ...);
void LogMessageAPI(void* loggerData, const char* fmt, va_list* args);


struct ConsoleCommandArgs {
    char* args;
};

const char* PullCommandArg(ConsoleCommandArgs* args);

struct Console;
struct GameContext;

typedef void(ConsoleCommandFn)(Console* console, GameContext* gameContext, ConsoleCommandArgs* args);

struct ConsoleCommand {
    const char* name;
    ConsoleCommandFn* command;
    const char* description;
};

// TODO: Match for full word
static const ConsoleCommand GlobalConsoleCommands[] = {
    { "dummy",              nullptr },
#if 0
    { "clear",              ConsoleClearCommand },
    { "help",               ConsoleHelpCommand },
    { "history",            ConsoleHistoryCommand },
    { "echo",               ConsoleEchoCommand },
    { "set",                ConsoleSetCommand },

    { "reset_player_p",     ResetPlayerPositionCommand },
    { "recompile_shaders",  RecompileShadersCommand },
    { "camera",             CameraCommand , "Available modes: free, follow, game" },
    { "add_entity",         AddEntityCommand },
    { "print_entities",     PrintEntitiesCommand },
    { "toggle_dbg_overlay", ToggleDebugOverlayCommand },
    { "pos",                SetEntityPosCommand },
    { "inventory",          PrintPlayerInventoryCommand },
    { "meta_info",          PrintGameMetaInfoCommand },
    { "creative_mode",      ToggleCreativeModeCommand }
#endif
};

struct ConsoleCommandRecord {
    ConsoleCommandRecord* next;
    ConsoleCommandRecord* prev;
    char* string;
};

struct Console {
    GameContext* gameContext;
    Logger* logger;
    PlatformHeap* heap;
    bool autoScrollEnabled;
    bool justOpened;
    u32 commandHistoryCount;
    ConsoleCommandRecord* commandHistoryCursor;
    ConsoleCommandRecord* firstCommandRecord;
    ConsoleCommandRecord* lastCommandRecord;
    char inputBuffer[256];
};

void InitConsole(Console* console, Logger* logger, PlatformHeap* heap, GameContext* gameContext);
void DrawConsole(Console* console);
