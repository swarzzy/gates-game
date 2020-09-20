#include "Game.h"

void GameInit() {
    auto context = GetContext();
    DrawListInit(&context->drawList, 256, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));

    auto image = ResourceLoaderLoadImage("../res/alpha_test.png", true, 4, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));
    TextureID texture = Renderer.UploadTexture(0, image->width, image->height, TextureFormat::RGBA8, TextureFilter::Bilinear, TextureWrapMode::Repeat, image->bits);
    assert(texture);
    context->testTexture = texture;

    auto allocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap);

    auto font = ResourceLoader.BakeFont("../res/fonts/Merriweather-Regular.ttf", &allocator, 64, 512);
    TextureID atlas = Renderer.UploadTexture(0, 512, 512, TextureFormat::R8, TextureFilter::Bilinear, TextureWrapMode::Repeat, font.bitmap);
    assert(atlas);
    context->fontAtlas = atlas;
}

void GameReload() {
}

void GameUpdate() {

}

void GameRender() {
    auto context = GetContext();
    auto list = &context->drawList;

    DrawListPushQuad(list, V2(1.0f, 1.0f), V2(2.0f, 1.0f), V2(2.0f, 2.0f), V2(1.0f, 2.0f), 0.0f, V4(1.0f, 0.0f, 0.0f, 1.0f));
    DrawListPushQuadAlphaMask(list, V2(3.0f, 3.0f), V2(5.0f, 3.0f), V2(5.0f, 5.0f), V2(3.0f, 5.0f), 0.0f, context->fontAtlas, V4(1.0f));

    Renderer.RenderDrawList(list);
    DrawListClear(list);

    if(KeyPressed(Key::Tilde)) {
        context->consoleEnabled = !context->consoleEnabled;
    }

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }
}
