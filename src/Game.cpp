#include "Game.h"

void GameInit() {
    void InitConsole(Console* console, Logger* logger, PlatformHeap* heap, GameContext* gameContext);

}

void GameReload() {
}

void GameUpdate() {

}

void GameRender() {
    auto context = GetContext();

    if(KeyPressed(Key::Tilde)) {
        context->consoleEnabled = !context->consoleEnabled;
    }

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }
}
