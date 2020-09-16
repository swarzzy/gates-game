#include "Game.h"

void GameInit() {
    auto context = GetContext();
    DrawListInit(&context->drawList, 256, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));
}

void GameReload() {
}

void GameUpdate() {

}

void GameRender() {
    auto context = GetContext();
    auto list = &context->drawList;

    DrawRect(list, V2(1.0f, 1.0f), V2(2.0f, 2.0f), 0.0f);
    DrawRect(list, V2(3.0f, 3.0f), V2(5.0f, 6.0f), 1.0f);

    if(KeyPressed(Key::Tilde)) {
        context->consoleEnabled = !context->consoleEnabled;
    }

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }
}
