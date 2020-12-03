#include "Draw.h"

void DrawListInit(DrawList* list, u32 capacity, Allocator* allocator) {
    assert(!list->commandBuffer.Capacity());
    assert(!list->vertexBuffer.Capacity());
    assert(!list->indexBuffer.Capacity());

    list->commandBuffer = Array<DrawCommand>(allocator);
    list->vertexBuffer = Array<Vertex>(allocator);
    list->indexBuffer = Array<u32>(allocator);
}

void DrawListClear(DrawList* list) {
    list->commandBuffer.Clear();
    list->vertexBuffer.Clear();
    list->indexBuffer.Clear();
}

DrawCommand* DrawListBeginBatch(DrawList* list, TextureMode mode, TextureID texture) {
    assert(!list->pendingCommand);
    list->pendingCommand = true;
    memset(&list->scratchCommand, 0, sizeof(DrawCommand));
    list->scratchCommand.vertexBufferOffset = list->vertexBuffer.Count();
    list->scratchCommand.indexBufferOffset = list->indexBuffer.Count();
    list->scratchCommand.textureMode = mode;
    list->scratchCommand.texture = texture;
    return &list->scratchCommand;
}

forceinline void DrawListPushVertexBatch(DrawList* list, Vertex vertex) {
    assert(list->pendingCommand);
    list->vertexBuffer.PushBack(vertex);
}


forceinline void DrawListPushIndexBatch(DrawList* list, u32 index) {
    assert(list->pendingCommand);
    list->indexBuffer.PushBack(index);
}

forceinline void DrawListPushQuadBatch(DrawList* list, v2 p0, v2 p1, v2 p2, v2 p3, f32 z, v2 uv0, v2 uv1, v4 color, f32 blend) {
    assert(list->pendingCommand);

    auto vertexOffset = list->vertexBuffer.Count();
    list->vertexBuffer.PushBack(Vertex(V3(p0, z), blend, color, V2(uv0.x, uv0.y)));
    list->vertexBuffer.PushBack(Vertex(V3(p1, z), blend, color, V2(uv1.x, uv0.y)));
    list->vertexBuffer.PushBack(Vertex(V3(p2, z), blend, color, V2(uv1.x, uv1.y)));
    list->vertexBuffer.PushBack(Vertex(V3(p3, z), blend, color, V2(uv0.x, uv1.y)));

    auto indexOffset = list->indexBuffer.Count();

    list->indexBuffer.PushBack(vertexOffset + 0);
    list->indexBuffer.PushBack(vertexOffset + 1);
    list->indexBuffer.PushBack(vertexOffset + 2);
    list->indexBuffer.PushBack(vertexOffset + 2);
    list->indexBuffer.PushBack(vertexOffset + 3);
    list->indexBuffer.PushBack(vertexOffset + 0);
}


// Leave this unrolled for speed in debug build
forceinline void DrawListPushRectBatch(DrawList* list, v2 min, v2 max, f32 z, v2 uv0, v2 uv1, v4 color, f32 blend) {
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

forceinline void DrawSimpleLineBatch(DrawList* list, v2 begin, v2 end, f32 z, f32 thickness, v4 color) {
    v2 vec = Normalize(end - begin);
    v2 perp = Perp(vec);
    v2 offset = perp * thickness * 0.5f;

    v2 p0 = begin + offset;
    v2 p1 = begin - offset;
    v2 p2 = end - offset;
    v2 p3 = end + offset;

    DrawListPushQuadBatch(list, p0, p1, p2, p3, z, {}, {}, color, 0.0f);
}

forceinline void DrawBoxBatch(DrawList* list, Box2D box, f32 z, f32 thickness, v4 color) {
    v2 p0 = box.min;
    v2 p1 = V2(box.max.x, box.min.y);
    v2 p2 = box.max;
    v2 p3 = V2(box.min.x, box.max.y);

    DrawSimpleLineBatch(list, p0, p1, z, thickness, color);
    DrawSimpleLineBatch(list, p1, p2, z, thickness, color);
    DrawSimpleLineBatch(list, p2, p3, z, thickness, color);
    DrawSimpleLineBatch(list, p3, p0, z, thickness, color);
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

void DrawListPushRect(DrawList* list, v2 min, v2 max, f32 z, v4 color) {
    DrawListPushQuad(list, min, V2(max.x, min.y), max, V2(min.x, max.y), z, color);
}


// TODO: Maybe typo on texture mode here?
void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture) {
    DrawListPushQuad(list, lb, rb, rt, lt, V2(0.0f), V2(1.0f, 0.0f), V2(1.0f), V2(0.0f, 1.0f), z, {}, texture, 1.0f, TextureMode::Color);
}

void DrawListPushQuadAlphaMask(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture, v4 color) {
    DrawListPushQuad(list, lb, rb, rt, lt, V2(0.0f), V2(1.0f, 0.0f), V2(1.0f), V2(0.0f, 1.0f), z, color, texture, 1.0f, TextureMode::AlphaMask);
}

forceinline void PushGlyphInternal(DrawList* list, GlyphInfo* glyph, v3 p, v2 pixelSize, v4 color, f32 fontScale) {
    v2 minUV = V2(glyph->uv0.x, glyph->uv1.y);
    v2 maxUV = V2(glyph->uv1.x, glyph->uv0.y);

    v2 min = Hadamard(V2(p.x + glyph->quadMin.x * fontScale, p.y - glyph->quadMax.y * fontScale), pixelSize);
    v2 max = Hadamard(V2(p.x + glyph->quadMax.x * fontScale, p.y - glyph->quadMin.y * fontScale), pixelSize);
    DrawListPushRectBatch(list, min, max, p.z, minUV, maxUV, color, 1.0f);
}

forceinline GlyphInfo* GetGlyph(Font* font, u16 codepoint) {
    assert((u32)codepoint < array_count(font->glyphIndexTable));
    auto glyphIndex = font->glyphIndexTable[codepoint];
    auto glyph = font->glyphs + glyphIndex;
    return glyph;
}

// TODO: Maybe it is worth to cache text positioning data when we calculate it first time
Box2D DrawText(DrawList* list, Font* font, const char16* string, v3 p, v4 color, v2 pixelSize, v2 anchor, f32 maxWidth, TextAlign align, f32 fontScale) {
    Box2D bbox = {};
    v3 posPx = V3(p.x / pixelSize.x, p.y / pixelSize.y, p.z);
    // Currently width assumed to be in pixels
    f32 maxWidthPx = maxWidth;// / pixelSize.x;
    f32 maxWidthScaled = maxWidthPx / fontScale;// / pixelSize.x;

    f32 lineHeight = (font->ascent - font->descent + font->lineGap) * fontScale;

    v2 textDim = CalcTextSizeUnscaled(font, string, maxWidthScaled) * fontScale;

    v2 cursor = posPx.xy - Hadamard(textDim, anchor);
    v2 begin = cursor;
    cursor.y += textDim.y;
    cursor.y -= (font->ascent + font->lineGap) * fontScale;

    v2 boxMin = - Hadamard(textDim, anchor);
    v2 boxMax = boxMin + textDim;
    bbox = Box2D(Hadamard(boxMin, pixelSize), Hadamard(boxMax, pixelSize));

    auto command = DrawListBeginBatch(list, font->sdf ? TextureMode::DistanceField : TextureMode::AlphaMask, font->atlas);
    if (font->sdf) {
        command->distanceFieldParams = font->sdfParams;
    }

    auto at = string;
    while (*at) {
        auto[lineDim, strOffset] = CalcSingleLineBondingBoxUnscaled(font, at, maxWidthScaled);
        if (align == TextAlign::Center) {
            cursor.x = begin.x + (textDim.x - lineDim.x * fontScale) * 0.5f;
        } else {
            cursor.x = begin.x;
        }
        DrawTextLine(list, font, at, (u32)strOffset, V3(cursor, posPx.z), color, pixelSize, anchor, maxWidthPx, fontScale);
        cursor.y -= lineHeight;
        at += strOffset;
    }
    DrawListEndBatch(list);

    return bbox;
}

void DrawTextLine(DrawList* list, Font* font, const char16* string, u32 count, v3 p, v4 color, v2 pixelSize, v2 anchor, f32 maxWidth, f32 fontScale) {
    v2 begin = p.xy;
    f32 z = p.z;

    f32 xAdvance = begin.x;
    f32 yPos = begin.y;

    for (u32 i = 0; i < count; i++) {
        char16 c = string[i];
        auto glyph = GetGlyph(font, (u16)c);
        auto newAdvanceX = xAdvance + glyph->xAdvance * fontScale;
        PushGlyphInternal(list, glyph, V3(xAdvance, yPos, z), pixelSize, color, fontScale);
        xAdvance += glyph->xAdvance * fontScale;
    }
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
                if (advance != 0.0f && newAdvanceX > maxWidth) {
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
