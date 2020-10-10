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

forceinline void PushGlyphInternal(DrawList* list, GlyphInfo* glyph, v3 p, v2 pixelSize, v4 color) {
    v2 minUV = V2(glyph->uv0.x, glyph->uv1.y);
    v2 maxUV = V2(glyph->uv1.x, glyph->uv0.y);

    v2 min = Hadamard(V2(p.x + glyph->quadMin.x, p.y - glyph->quadMax.y), pixelSize);
    v2 max = Hadamard(V2(p.x + glyph->quadMax.x, p.y - glyph->quadMin.y), pixelSize);
    DrawListPushQuadBatch(list, min, max, p.z, minUV, maxUV, color, 1.0f);
}

forceinline GlyphInfo* GetGlyph(Font* font, u16 codepoint) {
    assert((u32)codepoint < array_count(font->glyphIndexTable));
    auto glyphIndex = font->glyphIndexTable[codepoint];
    auto glyph = font->glyphs + glyphIndex;
    return glyph;
}

// TODO: Maybe it is worth to cache text positioning data when we calculate it first time
void DrawText(DrawList* list, Font* font, const char16* string, v3 p, v4 color, v2 pixelSize, v2 anchor, f32 maxWidth, TextAlign align) {
    v3 posPx = V3(p.x / pixelSize.x, p.y / pixelSize.y, p.z);
    // Currently width assumed to be in pixels
    f32 maxWidthPx = maxWidth;// / pixelSize.x;

    f32 lineHeight = font->ascent - font->descent + font->lineGap;

    v2 textDim = CalcTextSizeUnscaled(font, string, maxWidthPx);

    v2 cursor = posPx.xy - Hadamard(textDim, anchor);
    cursor.y += textDim.y;
    cursor.y -= font->ascent + font->lineGap;

    auto at = string;
    while (*at) {
        auto[lineDim, strOffset] = CalcSingleLineBondingBoxUnscaled(font, at, maxWidthPx);
        if (align == TextAlign::Center) {
            cursor.x = posPx.x + (textDim.x - lineDim.x) * 0.5f;
        } else {
            cursor.x = posPx.x;
        }
        uptr drawOffset = DrawTextLine(list, font, at, V3(cursor, posPx.z), color, pixelSize, anchor, maxWidthPx);
        assert(drawOffset == strOffset);
        cursor.y -= lineHeight;
        at += strOffset;
    }
}

uptr DrawTextLine(DrawList* list, Font* font, const char16* string, v3 p, v4 color, v2 pixelSize, v2 anchor, f32 maxWidth) {
    v2 begin = p.xy;
    f32 z = p.z;

    f32 xAdvance = begin.x;
    f32 yPos = begin.y;

    DrawListBeginBatch(list, TextureMode::AlphaMask, font->atlas);

    const char16* at = string;
    f32 lastWordEndAdvance = 0.0f;
    while (*at) {
        auto wordBegin = at;
        auto wordEnd = at;
        auto wordAdvance = 0.0f;

        // Find a word
        while (true) {
            if (*wordEnd == 0) break;
            if (IsSpace(*wordEnd)) break;

            auto glyph = GetGlyph(font, (u16)(*wordEnd));

            wordAdvance += glyph->xAdvance;

            wordEnd++;
        }

        bool end = false;

        // wordEnd points on space after word or \0
        if (wordAdvance <= maxWidth) {
            // If the word is shorter that maxWidth then fit it on
            // a current position or on a new line
            if ((xAdvance + wordAdvance - begin.x) > maxWidth) {
                at = wordBegin;
                xAdvance = lastWordEndAdvance;
                end = true;
                break;
            }

            for (const char16* c = wordBegin; c != wordEnd; c++) {
                auto glyph = GetGlyph(font, (u16)(*c));
                PushGlyphInternal(list, glyph, V3(xAdvance, yPos, z), pixelSize, color);
                xAdvance += glyph->xAdvance;
            }
        } else {
            // Otherwise if the word is longer than maxWidth
            // then wrap it on a new line
            for (const char16* c = wordBegin; c != wordEnd; c++) {
                auto glyph = GetGlyph(font, (u16)(*c));
                auto newAdvanceX = xAdvance + glyph->xAdvance;
                if (newAdvanceX - begin.x > maxWidth) {
                    at = c;
                    end = true;
                    break;
                }
                PushGlyphInternal(list, glyph, V3(xAdvance, yPos, z), pixelSize, color);
                xAdvance += glyph->xAdvance;
            }
        }

        if (end) {
            break;
        }

        if (*wordEnd) {
            if (*wordEnd == '\n') {
                at = wordEnd;
                break;
            } else {
                auto glyph = GetGlyph(font, (u16)(*wordEnd));
                PushGlyphInternal(list, glyph, V3(xAdvance, yPos, z), pixelSize, color);
                xAdvance += glyph->xAdvance;
            }
            wordEnd++;
        }

        at = wordEnd;
    }

    DrawListEndBatch(list);

    uptr result = (uptr)(at - string);
    return result;
}

Tuple<v2, uptr> CalcSingleLineBondingBoxUnscaled(Font* font, const char16* string, f32 maxWidth) {
    f32 advance = 0.0f;
    f32 lineHeight = font->ascent - font->descent + font->lineGap;

    auto at = string;
    f32 lastWordEndAdvance = 0.0f;
    while (*at) {
        auto wordBegin = at;
        auto wordEnd = at;
        auto wordAdvance = 0.0f;

        // Find a word
        while (true) {
            if (*wordEnd == 0) break;
            if (IsSpace(*wordEnd)) break;

            auto glyph = GetGlyph(font, (u16)(*wordEnd));
            wordAdvance += glyph->xAdvance;

            wordEnd++;
        }

        bool end = false;

        // wordEnd points on space after word or \0
        if (wordAdvance <= maxWidth) {
            // If the word is shorter that maxWidth then fit it on
            // a current position or on a new linw
            if ((advance + wordAdvance) > maxWidth) {
                at = wordBegin;
                advance = lastWordEndAdvance;
                end = true;
                break;
            }
            advance += wordAdvance;
        } else {
            // Otherwise if the word is longer than maxWidth
            // then wrap it on a new line
            for (const char16* c = wordBegin; c != wordEnd; c++) {
                auto glyph = GetGlyph(font, (u16)(*c));
                auto newAdvanceX = advance + glyph->xAdvance;
                if (newAdvanceX > maxWidth) {
                    at = c;
                    end = true;
                    break;
                }
                advance += glyph->xAdvance;
            }
        }

        if (end) {
            break;
        }

        if (*wordEnd) {
            if (*wordEnd == '\n') {
                at = wordEnd;
                break;
            } else {
                auto glyph = GetGlyph(font, (u16)(*wordEnd));
                lastWordEndAdvance = advance;
                advance += glyph->xAdvance;
            }
            wordEnd++;
        }

        at = wordEnd;
    }

    v2 result = V2(advance, lineHeight);
    return MakeTuple(result, (uptr)(at - string));
}

v2 CalcTextSizeUnscaled(Font* font, const char16* string, f32 maxWidth) {
    f32 width = 0.0f;
    f32 height = 0.0f;

    auto at = string;
    while (*at) {
        auto[lineDim, strOffset] = CalcSingleLineBondingBoxUnscaled(font, at, maxWidth);
        width = Max(width, lineDim.x);
        height += lineDim.y;
        at += strOffset;
    }

    v2 result = V2(width, height);

    return result;
}
