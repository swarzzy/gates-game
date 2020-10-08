#include "Draw.h"

void DrawListInit(DrawList* list, u32 capacity, Allocator allocator) {
    assert(!list->commandBuffer.Capacity());
    assert(!list->vertexBuffer.Capacity());
    assert(!list->indexBuffer.Capacity());

    list->commandBuffer = GrowableArray<DrawCommand>(allocator);
    list->vertexBuffer = GrowableArray<Vertex>(allocator);
    list->indexBuffer = GrowableArray<u32>(allocator);
}

void DrawListClear(DrawList* list) {
    list->commandBuffer.Clear();
    list->vertexBuffer.Clear();
    list->indexBuffer.Clear();
}

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, v2 uv0, v2 uv1, v2 uv2, v2 uv3, f32 z, v4 color, TextureID texture, f32 texBlend, TextureMode mode) {
    auto vertexOffset = list->vertexBuffer.Count();
    list->vertexBuffer.PushBack(Vertex(V3(lb, z), texBlend, color, uv0));
    list->vertexBuffer.PushBack(Vertex(V3(rb, z), texBlend, color, uv1));
    list->vertexBuffer.PushBack(Vertex(V3(rt, z), texBlend, color, uv2));
    list->vertexBuffer.PushBack(Vertex(V3(lt, z), texBlend, color, uv3));

    auto indexOffset = list->indexBuffer.Count();

    list->indexBuffer.PushBack(vertexOffset + 0);
    list->indexBuffer.PushBack(vertexOffset + 1);
    list->indexBuffer.PushBack(vertexOffset + 2);
    list->indexBuffer.PushBack(vertexOffset + 2);
    list->indexBuffer.PushBack(vertexOffset + 3);
    list->indexBuffer.PushBack(vertexOffset + 0);

    DrawCommand command {};
    command.vertexBufferOffset = vertexOffset;
    command.indexBufferOffset = indexOffset;
    command.indexCount = 6;
    command.textureMode = mode;
    command.texture = texture;

    list->commandBuffer.PushBack(command);
}

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, v4 color) {
    DrawListPushQuad(list, lb, rb, rt, lt, V2(0.0f), V2(1.0f, 0.0f), V2(1.0f), V2(0.0f, 1.0f), z, color, 0, 0.0f, TextureMode::Color);
}

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture) {
    DrawListPushQuad(list, lb, rb, rt, lt, V2(0.0f), V2(1.0f, 0.0f), V2(1.0f), V2(0.0f, 1.0f), z, {}, texture, 1.0f, TextureMode::Color);
}

void DrawListPushQuadAlphaMask(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture, v4 color) {
    DrawListPushQuad(list, lb, rb, rt, lt, V2(0.0f), V2(1.0f, 0.0f), V2(1.0f), V2(0.0f, 1.0f), z, color, texture, 1.0f, TextureMode::AlphaMask);
}

void DrawListPushGlyph(DrawList* list, v2 min, v2 max, v2 uv0, v2 uv1, f32 z, v4 color, TextureID atlas) {
    DrawListPushQuad(list, min, V2(max.x, min.y), max, V2(min.x, max.y), uv0, V2(uv1.x, uv0.y), uv1, V2(uv0.x, uv1.y), z, color, atlas, 1.0f, TextureMode::AlphaMask);
}

void DrawText(DrawList* list, Font* font, const char16* string, v2 p, f32 z, v4 color, v2 pixelSize, v2 anchor) {
    Rectangle2 bbox = CalcTextBoundingBox(font, string, pixelSize);
    v2 dim = V2(bbox.max.x - bbox.min.x, bbox.max.y - bbox.min.y);
    f32 lineHeight = (font->ascent - font->descent + font->lineGap) * pixelSize.y;

    v2 begin = p;
    // Put cursor to left bottom corner
    begin.y -= (font->ascent + font->lineGap) * pixelSize.y - dim.y;
    // Apply anchor
    begin -= Hadamard(dim, anchor);

    v2 advance = begin;

    for (const char16* at = string; *at; at++) {
        char16 ch = *at;
        if (ch == u'\n') {
            advance.x = begin.x;
            advance.y -= lineHeight;
        } else {
            assert((u32)ch < array_count(font->glyphIndexTable));
            auto glyphIndex = font->glyphIndexTable[(u16)ch];
            auto glyph = font->glyphs + glyphIndex;

            v2 minUV = V2(glyph->uv0.x, glyph->uv1.y);
            v2 maxUV = V2(glyph->uv1.x, glyph->uv0.y);

            v2 min = V2(advance.x + glyph->quadMin.x * pixelSize.x, advance.y - glyph->quadMax.y * pixelSize.y);
            v2 max = V2(advance.x + glyph->quadMax.x * pixelSize.x, advance.y - glyph->quadMin.y * pixelSize.y);
            DrawListPushGlyph(list, min, max, minUV, maxUV, z, color, font->atlas);
            advance.x += glyph->xAdvance * pixelSize.x;
        }
    }
}

Rectangle2 CalcTextBoundingBox(Font* font, const char16* string, v2 pixelSize, v2 anchor) {
    Rectangle2 result;
    v2 begin = V2(0.0f);
    v2 end = V2(0.0f);
    v2 advance = V2(0.0f);
    f32 lineHeight = (font->ascent - font->descent + font->lineGap) * pixelSize.y;
    advance.y -= (font->ascent + font->lineGap) * pixelSize.y;
    for (const char16* at = string; *at; at++) {
        char16 ch = *at;
        if (ch == u'\n') {
            if (advance.x > end.x) {
                end.x = advance.x;
            }

            advance.x = 0.0f;
            advance.y -= lineHeight;
        } else {
            assert((u32)ch < array_count(font->glyphIndexTable));
            auto glyphIndex = font->glyphIndexTable[(u16)ch];
            auto glyph = font->glyphs + glyphIndex;
            advance.x += glyph->xAdvance * pixelSize.x;
        }
    }

    if (advance.x > end.x) {
        end.x = advance.x;
    }

    end.y = advance.y + font->descent * pixelSize.y;

    v2 min = V2(Min(begin.x, end.x), Min(-begin.y, -end.y));
    v2 max = V2(Max(begin.x, end.x), Max(-begin.y, -end.y));
    v2 dim = V2(max.x - min.x, max.y - min.y);
    result.min = min - Hadamard(dim, anchor);
    result.max = max - Hadamard(dim, anchor);

    return result;
}
