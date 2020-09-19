#include "Game.h"

void GameInit() {
    auto context = GetContext();
    DrawListInit(&context->drawList, 256, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));

    auto image = ResourceLoaderLoadImage("../res/tile_stone.png", true, 4, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));
    int i = 0;
}

void GameReload() {
}

void GameUpdate() {

}

void GameRender() {
    auto context = GetContext();
    auto list = &context->drawList;

    DrawListPushQuad(list, V2(1.0f, 1.0f), V2(2.0f, 1.0f), V2(2.0f, 2.0f), V2(1.0f, 2.0f), 0.0f, V4(1.0f, 0.0f, 0.0f, 1.0f));
    DrawListPushQuad(list, V2(3.0f, 3.0f), V2(5.0f, 3.0f), V2(5.0f, 5.0f), V2(3.0f, 5.0f), 0.0f, V4(0.0f, 1.0f, 0.0f, 1.0f));

    Renderer.RenderDrawList(list);
    DrawListClear(list);

    if(KeyPressed(Key::Tilde)) {
        context->consoleEnabled = !context->consoleEnabled;
    }

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }
}
