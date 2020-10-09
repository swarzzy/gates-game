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

void DrawListBeginBatch(DrawList* list, TextureMode mode, TextureID texture = 0) {
    assert(!list->pendingCommand);
    // TODO: Make shure command are cleared or all fields will be overwriten
    list->pendingCommand = true;
    list->scratchCommand.vertexBufferOffset = list->vertexBuffer.Count();
    list->scratchCommand.indexBufferOffset = list->indexBuffer.Count();
    list->scratchCommand.textureMode = mode;
    list->scratchCommand.texture = texture;
}

forceinline void DrawListPushVertexBatch(DrawList* list, Vertex vertex) {
    assert(list->pendingCommand);
    list->vertexBuffer.PushBack(vertex);
}

forceinline void DrawListPushIndexBatch(DrawList* list, u32 index) {
    assert(list->pendingCommand);
    list->indexBuffer.PushBack(index);
}

forceinline void DrawListPushQuadBatch(DrawList* list, v2 min, v2 max, f32 z, v2 uv0, v2 uv1, v4 color, f32 blend) {
    assert(list->pendingCommand);

    auto vertexOffset = list->vertexBuffer.Count();
    list->vertexBuffer.PushBack(Vertex(V3(min.x, min.y, z), blend, color, V2(uv0.x, uv0.y)));
    list->vertexBuffer.PushBack(Vertex(V3(max.x, min.y, z), blend, color, V2(uv1.x, uv0.y)));
    list->vertexBuffer.PushBack(Vertex(V3(max.x, max.y, z), blend, color, V2(uv1.x, uv1.y)));
    list->vertexBuffer.PushBack(Vertex(V3(min.x, max.y, z), blend, color, V2(uv0.x, uv1.y)));

    auto indexOffset = list->indexBuffer.Count();

    list->indexBuffer.PushBack(vertexOffset + 0);
    list->indexBuffer.PushBack(vertexOffset + 1);
    list->indexBuffer.PushBack(vertexOffset + 2);
    list->indexBuffer.PushBack(vertexOffset + 2);
    list->indexBuffer.PushBack(vertexOffset + 3);
    list->indexBuffer.PushBack(vertexOffset + 0);
}


void DrawListEndBatch(DrawList* list) {
    assert(list->pendingCommand);
    u32 indexCount = list->indexBuffer.Count() - list->scratchCommand.indexBufferOffset;
    list->scratchCommand.indexCount = indexCount;

    list->commandBuffer.PushBack(list->scratchCommand);

    list->pendingCommand = false;
}

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, v2 uv0, v2 uv1, v2 uv2, v2 uv3, f32 z, v4 color, TextureID texture, f32 texBlend, TextureMode mode) {
    assert(!list->pendingCommand);

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

forceinline void PushGlyphInternal(DrawList* list, GlyphInfo* glyph, v2 advance, v2 pixelSize, f32 z, v4 color) {
    v2 minUV = V2(glyph->uv0.x, glyph->uv1.y);
    v2 maxUV = V2(glyph->uv1.x, glyph->uv0.y);

    v2 min = V2(advance.x + glyph->quadMin.x * pixelSize.x, advance.y - glyph->quadMax.y * pixelSize.y);
    v2 max = V2(advance.x + glyph->quadMax.x * pixelSize.x, advance.y - glyph->quadMin.y * pixelSize.y);
    DrawListPushQuadBatch(list, min, max, z, minUV, maxUV, color, 1.0f);
}

forceinline GlyphInfo* GetGlyph(Font* font, u16 codepoint) {
    assert((u32)codepoint < array_count(font->glyphIndexTable));
    auto glyphIndex = font->glyphIndexTable[codepoint];
    auto glyph = font->glyphs + glyphIndex;
    return glyph;
}

void DrawText(DrawList* list, Font* font, const char16* string, v2 p, f32 z, v4 color, v2 pixelSize, v2 anchor, f32 maxWidth) {
    Rectangle2 bbox = CalcTextBoundingBox(font, string, pixelSize, {}, maxWidth);
    v2 dim = V2(bbox.max.x - bbox.min.x, bbox.max.y - bbox.min.y);
    f32 lineHeight = (font->ascent - font->descent + font->lineGap) * pixelSize.y;
    f32 maxWidthS = maxWidth * pixelSize.x;

    v2 begin = p;
    // Put cursor to left bottom corner
    begin.y -= (font->ascent + font->lineGap) * pixelSize.y - dim.y;
    // Apply anchor
    begin -= Hadamard(dim, anchor);

    v2 advance = begin;

    DrawListBeginBatch(list, TextureMode::AlphaMask, font->atlas);

    const char16* at = string;
    while (*at) {
        auto wordBegin = at;
        auto wordEnd = at;
        auto wordAdvance = 0.0f;

        // Find a word
        while (true) {
            if (*wordEnd == 0) break;
            if (IsSpace(*wordEnd)) break;

            auto glyph = GetGlyph(font, (u16)(*wordEnd));

            wordAdvance += glyph->xAdvance * pixelSize.x;

            wordEnd++;
        }

        // wordEnd points on space after word or \0

        if (wordAdvance <= maxWidthS) {
            // If the word is shorter that maxWidth then fit it on
            // a current position or on a new linw
            if ((advance.x + wordAdvance - begin.x) > maxWidthS) {
                advance.y -= lineHeight;
                advance.x = begin.x;
            }

            for (const char16* c = wordBegin; c != wordEnd; c++) {
                auto glyph = GetGlyph(font, (u16)(*c));
                PushGlyphInternal(list, glyph, advance, pixelSize, z, color);
                advance.x += glyph->xAdvance * pixelSize.x;
            }
        } else {
            // Otherwise if the word is longer than maxWidth
            // then wrap it on a new line
            for (const char16* c = wordBegin; c != wordEnd; c++) {
                auto glyph = GetGlyph(font, (u16)(*c));
                auto newAdvanceX = advance.x + glyph->xAdvance * pixelSize.x;
                if (newAdvanceX - begin.x > maxWidthS) {
                    advance.x = begin.x;
                    advance.y -= lineHeight;
                }
                PushGlyphInternal(list, glyph, advance, pixelSize, z, color);
                advance.x += glyph->xAdvance * pixelSize.x;
            }
        }

        if (*wordEnd) {
            if (*wordEnd == '\n') {
                advance.y -= lineHeight;
                advance.x = begin.x;
            } else {
                auto glyph = GetGlyph(font, (u16)(*wordEnd));
                PushGlyphInternal(list, glyph, advance, pixelSize, z, color);
                advance.x += glyph->xAdvance * pixelSize.x;
            }
            wordEnd++;
        }

        at = wordEnd;
    }

    DrawListEndBatch(list);
}

Rectangle2 CalcTextBoundingBox(Font* font, const char16* string, v2 pixelSize, v2 anchor, f32 maxWidth) {
    Rectangle2 result;
    //v2 begin = V2(0.0f);
    v2 end = V2(0.0f);
    v2 advance = V2(0.0f);
    f32 lineHeight = (font->ascent - font->descent + font->lineGap) * pixelSize.y;
    f32 maxWidthS = maxWidth * pixelSize.x;

    advance.y -= (font->ascent + font->lineGap) * pixelSize.y;

    const char16* at = string;
    while (*at) {
        auto wordBegin = at;
        auto wordEnd = at;
        auto wordAdvance = 0.0f;

        // Find a word
        while (true) {
            if (*wordEnd == 0) break;
            if (IsSpace(*wordEnd)) break;

            auto glyph = GetGlyph(font, (u16)(*wordEnd));

            wordAdvance += glyph->xAdvance * pixelSize.x;

            wordEnd++;
        }

        // wordEnd points on space after word or \0

        if (wordAdvance <= maxWidthS) {
            // If the word is shorter that maxWidth then fit it on
            // a current position or on a new linw
            if ((advance.x + wordAdvance) > maxWidthS) {
                if (advance.x > end.x) end.x = advance.x;
                advance.y -= lineHeight;
                advance.x = 0.0f;
            }
            advance.x += wordAdvance;
        } else {
            // Otherwise if the word is longer than maxWidth
            // then wrap it on a new line
            for (const char16* c = wordBegin; c != wordEnd; c++) {
                auto glyph = GetGlyph(font, (u16)(*c));
                auto newAdvanceX = advance.x + glyph->xAdvance * pixelSize.x;
                if (newAdvanceX  > maxWidthS) {
                    if (advance.x > end.x) end.x = advance.x;
                    advance.x = 0.0f;
                    advance.y -= lineHeight;
                }
                advance.x += glyph->xAdvance * pixelSize.x;
            }
        }

        if (*wordEnd) {
            if (*wordEnd == '\n') {
                if (advance.x > end.x) end.x = advance.x;
                advance.y -= lineHeight;
                advance.x = 0.0f;
            } else {
                auto glyph = GetGlyph(font, (u16)(*wordEnd));
                advance.x += glyph->xAdvance * pixelSize.x;
            }
            wordEnd++;
        }

        at = wordEnd;
    }

    end.y = advance.y + font->descent * pixelSize.y;

    v2 min = V2(0.0f);
    v2 max = V2(end.x, -end.y);
    v2 dim = V2(max.x - min.x, max.y - min.y);
    result.min = min - Hadamard(dim, anchor);
    result.max = max - Hadamard(dim, anchor);

    //result.min.x /= pixelSize.x;
    //result.min.y /= pixelSize.y;
    //result.max.x /= pixelSize.x;
    //result.max.y /= pixelSize.y;

    return result;
}
