#include "Canvas.h"

Canvas CreateCanvas(Allocator* drawListAllocator) {
    Canvas canvas {};
    canvas.scale = 1.0f;
    DrawListInit(&canvas.drawList, 256, drawListAllocator);

    return canvas;
}

void UpdateCanvas(Canvas* canvas) {
    auto platform = GetPlatform();

    f32 wWindow = (f32)platform->windowWidth;
    f32 hWindow = (f32)platform->windowHeight;

    canvas->pixelsPerCm = platform->pixelsPerCentimeter * canvas->scale;
    canvas->cmPerPixel = 1.0f / canvas->pixelsPerCm;

    canvas->sizePx = V2(wWindow, hWindow);
    canvas->sizeCm = canvas->sizePx * canvas->cmPerPixel;
}

v2 CanvasPositionFromNormalized(Canvas* canvas, v2 normalized) {
    v2 result = Hadamard(canvas->sizeCm, normalized);
    return result;
}

void BeginCanvas(Canvas* canvas) {
    auto proj = OrthoGLRH(0.0f, canvas->sizeCm.x, 0.0f, canvas->sizeCm.y, -1.0f, 1.0f);
    Renderer.SetCamera(&proj);
}

v2 CanvasProjectScreenPos(Canvas* canvas, v2 normalizedScreenPos) {
    v2 result = Hadamard(normalizedScreenPos, canvas->sizeCm);
    return result;
}

void EndCanvas(Canvas* canvas) {
    Renderer.SubmitDrawList(&canvas->drawList);
    DrawListClear(&canvas->drawList);
}
