#pragma once

#include "Common.h"
#include "RenderAPI.h"
#include "Array.h"

struct Font;


enum struct TextAlign {
    Left, Center
};

void DrawListInit(DrawList* list, u32 capacity, Allocator* allocator);

void DrawListClear(DrawList* list);

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, v4 color);
void DrawListPushRect(DrawList* list, v2 min, v2 max, f32 z, v4 color);
void DrawListPushGlyph(DrawList* list, v2 min, v2 max, v2 uv0, v2 uv1, f32 z, v4 color, TextureID atlas);
void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture);
void DrawListPushQuadAlphaMask(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture, v4 color);

DrawCommand* DrawListBeginBatch(DrawList* list, TextureMode mode, TextureID texture = 0);
forceinline void DrawListPushRectBatch(DrawList* list, v2 min, v2 max, f32 z, v2 uv0, v2 uv1, v4 color, f32 blend);
forceinline void DrawSimpleLineBatch(DrawList* list, v2 begin, v2 end, f32 z, f32 thickness, v4 color);
forceinline void DrawBoxBatch(DrawList* list, Box2D box, f32 z, f32 thickness, v4 color);
void DrawListEndBatch(DrawList* list);

Tuple<v2, uptr> CalcSingleLineBondingBoxUnscaled(Font* font, const char16* string, f32 maxWidth);
void DrawTextLine(DrawList* list, Font* font, const char16* string, u32 count, v3 p, v4 color, v2 pixelSize, v2 anchor, f32 maxWidth, f32 fontScale);
v2 CalcTextSizeUnscaled(Font* font, const char16* string, f32 maxWidth);
// Optionally returns text bounding box relative to p
Box2D DrawText(DrawList* list, Font* font, const char16* string, v3 p, v4 color, v2 pixelSize, v2 anchor, f32 maxWidth, TextAlign align, f32 fontScale = 1.0f);
