#include "Game.h"
#undef UNICODE
#undef _UNICODE

CodepointRange ranges[2];
f32 DefaultFontHeight = 24.0f;

void GameInit() {
    auto context = GetContext();
    DrawListInit(&context->drawList, 256, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));

    auto image = ResourceLoaderLoadImage("../res/alpha_test.png", true, 4, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));
    TextureID texture = Renderer.UploadTexture(0, image->width, image->height, TextureFormat::RGBA8, TextureFilter::Bilinear, TextureWrapMode::Repeat, image->bits);
    assert(texture);
    context->testTexture = texture;

    ranges[0].begin = 32;
    ranges[0].end = 126;
    ranges[1].begin = 0x0410;
    ranges[1].end = 0x044f;

    auto allocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap);

    auto f1 = ResourceLoader.BakeFont(&context->font, "../res/fonts/Merriweather-Regular.ttf", &allocator, 512, DefaultFontHeight, ranges, array_count(ranges));
    auto f2 = ResourceLoader.LoadFontBM(&context->sdfFont, "../res/fonts/merriweather_sdf.fnt", &allocator);
    assert(f1);
    assert(f2);

    context->sdfFont.sdf = true;
    context->sdfFont.sdfParams = V2(1.0f, 0.0f);

    TextureID fontAtlas = Renderer.UploadTexture(0, context->font.bitmapSize, context->font.bitmapSize, TextureFormat::R8, TextureFilter::Bilinear, TextureWrapMode::Repeat, context->font.bitmap);
    context->font.atlas = fontAtlas;

    TextureID sdfFontAtlas = Renderer.UploadTexture(0, context->sdfFont.bitmapSize, context->sdfFont.bitmapSize, TextureFormat::R8, TextureFilter::Bilinear, TextureWrapMode::Repeat, context->sdfFont.bitmap);
    context->sdfFont.atlas = sdfFontAtlas;
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


    const char16* string = u"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.";

    f32 width = 300.0f;
    static f32 scale = 1.0f;
    static f32 scaleSDF = 1.0f;

    auto textDim = CalcTextSizeUnscaled(&context->font, string, width / scale);
    textDim.x *= pixelSize.x * scale;
    textDim.y *= pixelSize.y * scale;

    //DEBUG_OVERLAY_SLIDER(context->sdfFont.sdfParams.x, 0.0f, 3.0f);
    //DEBUG_OVERLAY_SLIDER(context->sdfFont.sdfParams.y, 0.0f, 3.0f);
    auto prevScale = scale;
    DEBUG_OVERLAY_SLIDER(scale, 0.3f, 3.0f);
    DEBUG_OVERLAY_SLIDER(scaleSDF, 0.3f, 3.0f);


    if (scale != prevScale) {
        f32 newHeight = Round(DefaultFontHeight * scale);
        auto allocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap);
        FontFreeResources(&context->font, allocator);
        memset(context->font.glyphIndexTable, 0, sizeof(u16) * U16::Max);
        auto f1 = ResourceLoader.BakeFont(&context->font, "../res/fonts/Merriweather-Regular.ttf", &allocator, 8192, newHeight, ranges, array_count(ranges));
        assert(f1);
        TextureID fontAtlas = Renderer.UploadTexture(context->font.atlas, context->font.bitmapSize, context->font.bitmapSize, TextureFormat::R8, TextureFilter::Bilinear, TextureWrapMode::Repeat, context->font.bitmap);
        context->font.atlas = fontAtlas;
    }


    DrawText(list, &context->font, string, V3(200.0f, 350.0f, 0.0f), V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.5f), width, TextAlign::Center, 1.0f);
    DrawText(list, &context->sdfFont, string, V3(700.0f, 350.0f, 0.0f), V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.5f), width, TextAlign::Center, scaleSDF);

    //string[strOffset] = 0;

    //DrawListPushQuad(list, V2(100.0f, 200.0f), V2(100.0f + textDim.x, 200.0f), V2(100.0f + textDim.x, 200.0f + textDim.y), V2(100.0f, 200.0f + textDim.y), 0.0f, V4(1.0f, 0.0f, 0.0f, 0.3f));
    //DrawText(list, &context->font, string, V2(100.0f, 200.0f), 0.0f, V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f));
    //DrawTextLine(list, &context->font, string, V2(100.0f, 200.0f), 0.0f, V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f), 220.0f, textDim);
    //DrawText(list, &context->font, string, V3(100.0f, 200.0f, 0.0f), V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f), width, TextAlign::Left, scale);
    //DrawListPushQuad(list, V2(100.0f, 200.0f), V2(105.0f, 200.0f), V2(104.0f, 205.0f), V2(100.0f, 205.0f), 0.0f, V4(0.0f, 0.0f, 1.0f, 1.0f));

    Renderer.RenderDrawList(list);
    DrawListClear(list);

    if(KeyPressed(Key::Tilde)) {
        context->consoleEnabled = !context->consoleEnabled;
    }

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }
}
