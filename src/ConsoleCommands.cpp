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

void SaveAsConsoleCommand(Console* console, GameContext* gameContext, ConsoleCommandArgs* args) {
    auto logger = console->logger;
    if (args->args) {
        auto fileData = SerializeDeskToJson(gameContext->desk, &gameContext->desk->serializer);
        auto succeed = Platform.DebugWriteFile(args->args, fileData.Data(), fileData.Count() - 1);
        if (succeed) {
            LogMessage(logger, "%s saved successfully\n", args->args);
        } else {
            LogMessage(logger, "Failed to write file %s\n", args->args);
        }
    } else {
        LogMessage(logger, "save_as: No file name specified\n");
    }
}

void LoadConsoleCommand(Console* console, GameContext* gameContext, ConsoleCommandArgs* args) {
    auto logger = console->logger;
    if (args->args) {
        if (gameContext->desk) {
            // It's safe to do this since commands are executed at the end of Render
            // TODO: Should they be executed in Update?
            DestroyDesk();
        }

        CreateDesk();

        auto desk = GetDesk();
        auto deserializer = &desk->deserializer;

        bool loaded = LoadDeskFromFile(deserializer, desk, args->args);

        if (!loaded) {
            DestroyDesk();
            gameContext->gameState = GameState::Menu;
            LogMessage(logger, "Failed to load desk %s. Exiting to main menu\n", args->args);
        } else {
            gameContext->gameState = GameState::Desk;
            LogMessage(logger, "Desk %s loaded successfully\n", args->args);
        }
    } else {
        LogMessage(logger, "load: No file name specified\n");
    }
}
