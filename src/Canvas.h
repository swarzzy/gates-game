#pragma once

#include "Draw.h"

// Physical centimeter is default measurement

struct Canvas {
    v2 sizeCm;
    v2 sizePx;
    f32 pixelsPerCm;
    f32 cmPerPixel;
    f32 scale;
    DrawList drawList;
};

Canvas CreateCanvas(Allocator* drawListAllocator);
void UpdateCanvas(Canvas* canvas);
void BeginCanvas(Canvas* canvas);
void EndCanvas(Canvas* canvas);
v2 CanvasProjectScreenPos(Canvas* canvas, v2 normalizedScreenPos);

v2 CanvasPositionFromNormalized(Canvas* canvas, v2 normalized);

f32 CmToRender(Canvas* canvas, f32 cm) {
    f32 result = canvas->pixelsPerCm * cm;
    return result;
}

v2 CmToRender(Canvas* canvas, v2 cm) {
    v2 result = cm * canvas->pixelsPerCm;
    return result;
}
