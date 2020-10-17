#include "Canvas.h"

Canvas CreateCanvas(Allocator drawListAllocator) {
    Canvas canvas {};
    canvas.scale = 1.0f;
    DrawListInit(&canvas.drawList, 256, drawListAllocator);

    return canvas;
}

void BeginCanvas(Canvas* canvas) {
    auto platform = GetPlatform();

    f32 wWindow = (f32)platform->windowWidth;
    f32 hWindow = (f32)platform->windowHeight;

    canvas->pixelsPerCm = platform->pixelsPerCentimeter * canvas->scale;
    canvas->cmPerPixel = 1.0f / canvas->pixelsPerCm;

    canvas->sizePx = V2(wWindow, hWindow);
    canvas->sizeCm = canvas->sizePx * canvas->cmPerPixel;

    auto proj = OrthoGLRH(0.0f, canvas->sizeCm.x, 0.0f, canvas->sizeCm.y, -1.0f, 1.0f);
    Renderer.SetCamera(&proj);

    //canvas->pixelSize.x = canvas->size.x / wWindow;
    //canvas->pixelSize.y = canvas->size.y / hWindow;
}

void EndCanvas(Canvas* canvas) {
    Renderer.RenderDrawList(&canvas->drawList);
    DrawListClear(&canvas->drawList);
}
