#include "Game.h"
#undef UNICODE
#undef _UNICODE

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
    ranges[0].end = 126;
    ranges[1].begin = 0x0410;
    ranges[1].end = 0x044f;

    context->font.bitmapSize = 1024;
    //context->font.bitmap = (u8*)Platform.HeapAlloc(context->mainHeap, sizeof(u8) * context->font.bitmapSize * context->font.bitmapSize, false);

    //uptr glyphTableLength = CalcGlyphTableLength(ranges, array_count(ranges));

    //context->font.glyphs = (GlyphInfo*)Platform.HeapAlloc(context->mainHeap, (u32)sizeof(GlyphInfo) * (u32)glyphTableLength, false);
    //context->font.glyphCount = (u32)glyphTableLength;

    //ResourceLoader.BakeFont(&context->font, "../res/fonts/Merriweather-Regular.ttf", &allocator, 512, 18, ranges, array_count(ranges));
    ResourceLoader.LoadFontBM(&context->font, "../res/fonts/merriweather_sdf.fnt", &allocator);

    TextureID atlas = Renderer.UploadTexture(0, 512, 512, TextureFormat::R8, TextureFilter::Bilinear, TextureWrapMode::Repeat, context->font.bitmap);
    context->font.atlas = atlas;
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

    auto platform = GetPlatform();

    u32 wWindow = platform->windowWidth;
    u32 hWindow = platform->windowHeight;
    u32 wCanvas = 1280;
    u32 hCanvas = 720;
    f32 wPixel = (f32)wCanvas / wWindow;
    f32 hPixel = (f32)hCanvas / hWindow;
    v2 pixelSize = V2(wPixel, hPixel);

    auto proj = OrthoGLRH(0.0f, (f32)wCanvas, 0.0f, (f32)hCanvas, -1.0f, 1.0f);
    Renderer.SetCamera(&proj);

    //DrawListPushQuad(list, V2(1.0f, 1.0f), V2(2.0f, 1.0f), V2(2.0f, 2.0f), V2(1.0f, 2.0f), 0.0f, V4(1.0f, 0.0f, 0.0f, 1.0f));
    // DrawListPushQuadAlphaMask(list, V2(3.0f, 3.0f), V2(500.0f, 3.0f), V2(500.0f, 500.0f), V2(3.0f, 500.0f), 0.0f, context->fontAtlas, V4(0.0f, 0.0f, 0.0f, 0.0f));

    //   auto glyph = context->font.glyphs[context->font.glyphIndexTable[0x0428]];
    //   DrawListPushGlyph(list, V2(6.0f), V2(7.0f), V2(glyph.x0, glyph.y1), V2(glyph.x1, glyph.y0), 0.0f, V4(1.0f), context->fontAtlas);

    const char16* str = u"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.";
    auto size = sizeof(u"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.");
    char16* string = (char16*)Platform.HeapAlloc(context->mainHeap, (usize)size, false);
    memcpy(string, str, size);
    const char16* string1 = u"Here if more text\nWith multiple lines\nOh! Another line";
#if 1
    //auto[lineDim, strOffset] = CalcSingleLineBondingBoxUnscaled(&context->font, string, 220.0f);
    //lineDim.x *= pixelSize.x;
    //lineDim.y *= pixelSize.y;

    f32 width = 300.0f;
    f32 scale = 1.0f;

    auto textDim = CalcTextSizeUnscaled(&context->font, string, width / scale);
    textDim.x *= pixelSize.x * scale;
    textDim.y *= pixelSize.y * scale;

    //string[strOffset] = 0;

    //DrawListPushQuad(list, V2(100.0f, 200.0f), V2(100.0f + textDim.x, 200.0f), V2(100.0f + textDim.x, 200.0f + textDim.y), V2(100.0f, 200.0f + textDim.y), 0.0f, V4(1.0f, 0.0f, 0.0f, 0.3f));
    //DrawText(list, &context->font, string, V2(100.0f, 200.0f), 0.0f, V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f));
    //DrawTextLine(list, &context->font, string, V2(100.0f, 200.0f), 0.0f, V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f), 220.0f, textDim);
    DrawText(list, &context->font, string, V3(100.0f, 200.0f, 0.0f), V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f), width, TextAlign::Left, scale);
    DrawListPushQuad(list, V2(100.0f, 200.0f), V2(105.0f, 200.0f), V2(104.0f, 205.0f), V2(100.0f, 205.0f), 0.0f, V4(0.0f, 0.0f, 1.0f, 1.0f));




#else
    auto size = CalcTextSize(&context->font, string, pixelSize, 300.0f );
    f32 h = size.y;
    Rectangle2 bbox {V2(0.0f), size};
    bbox.min += V2(400.0f, 200.0f);// - V2(0.0f, h);
    bbox.max += V2(400.0f, 200.0f);// - V2(0.0f, h);
    DrawListPushQuad(list, V2(bbox.min.x, bbox.min.y), V2(bbox.max.x, bbox.min.y), V2(bbox.max.x, bbox.max.y), V2(bbox.min.x, bbox.max.y), 0.0f, V4(1.0f, 0.0f, 0.0f, 0.3f));

    DrawText(list, &context->font, string, V2(400.0f, 200.0f), 0.0f, V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f), 300.0f);
    DrawText(list, &context->font, string1, V2(200.0f, 500.0f), 0.0f, V4(1.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f));
    DrawListPushQuad(list, V2(400.0f, 200.0f), V2(405.0f, 200.0f), V2(405.0f, 205.0f), V2(400.0f, 205.0f), 0.0f, V4(0.0f, 0.0f, 1.0f, 1.0f));
#endif
    Renderer.RenderDrawList(list);
    DrawListClear(list);

    if(KeyPressed(Key::Tilde)) {
        context->consoleEnabled = !context->consoleEnabled;
    }

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }
}
