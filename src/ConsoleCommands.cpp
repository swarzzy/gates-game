#include "ConsoleCommands.h"

void ConsoleDrawModeCommand(Console* console, GameContext* gameContext, ConsoleCommandArgs* args) {
    auto logger = console->logger;
    if (args->args) {
        if (StringsAreEqual(args->args, "normal")) {
            gameContext->drawMode = DrawMode::Normal;
        } else if (StringsAreEqual(args->args, "desk_debug")) {
            gameContext->drawMode = DrawMode::DeskDebug;
        } else {
            LogMessage(logger, "draw: Wrong argument %s\n", args->args);
        }
    } else {
        LogMessage(logger, "draw: No arguments passed. should be normal or desk_debug\n");
    }
}
