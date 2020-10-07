#include "Game.h"

void GameInit() {
    auto context = GetContext();
    DrawListInit(&context->drawList, 256, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));

    auto image = ResourceLoaderLoadImage("../res/alpha_test.png", true, 4, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));
    TextureID texture = Renderer.UploadTexture(0, image->width, image->height, TextureFormat::RGBA8, TextureFilter::Bilinear, TextureWrapMode::Repeat, image->bits);
    assert(texture);
    context->testTexture = texture;

    auto allocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap);

    CodepointRange ranges[2];
    ranges[0].begin = 32;
    ranges[0].end = 94;
    ranges[1].begin = 0x0410;
    ranges[1].end = 0x044f;

    context->font.bitmap = (u8*)Platform.HeapAlloc(context->mainHeap, sizeof(u8) * 512 * 512, false);
    context->font.bitmapSize = 512;

    uptr glyphTableLength = CalcGlyphTableLength(ranges, array_count(ranges));

    context->font.glyphs = (GlyphInfo*)Platform.HeapAlloc(context->mainHeap, (u32)sizeof(GlyphInfo) * (u32)glyphTableLength, false);
    context->font.glyphCount = (u32)glyphTableLength;

    ResourceLoader.BakeFont(&context->font, "../res/fonts/Merriweather-Regular.ttf", &allocator, 64, ranges, array_count(ranges));
    TextureID atlas = Renderer.UploadTexture(0, 512, 512, TextureFormat::R8, TextureFilter::Bilinear, TextureWrapMode::Repeat, context->font.bitmap);
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

    auto glyph = context->font.glyphs[context->font.glyphIndexTable[0x0428]];
    DrawListPushGlyph(list, V2(6.0f), V2(7.0f), V2(glyph.x0, glyph.y1), V2(glyph.x1, glyph.y0), 0.0f, V4(1.0f), context->fontAtlas);

    Renderer.RenderDrawList(list);
    DrawListClear(list);

    if(KeyPressed(Key::Tilde)) {
        context->consoleEnabled = !context->consoleEnabled;
    }

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }
}
