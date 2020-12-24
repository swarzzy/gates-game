#include "Assets.h"

#include "String.h"
#include "StringBuilder.h"

#include "../ext/stb_image-2.26/stb_image.h"
#include "../ext/stb_truetype-1.24/stb_truetype.h"

void FontFreeResources(Font* font, Allocator* allocator) {
    if (font->glyphs) {
        allocator->Dealloc(font->glyphs);
    }
    if (font->bitmap) {
        allocator->Dealloc(font->bitmap);
    }
}

u32 CalcGlyphTableLength(CodepointRange* ranges, u32 rangeCount) {
    u32 totalCodepointCount = 0;
    for (u32 i = 0; i < rangeCount; i++) {
        assert(ranges[i].end > ranges[i].begin);
        totalCodepointCount += ranges[i].end - ranges[i].begin + 1;
    }
    return totalCodepointCount + 1; // One for dummy char
}

LoadImageResult* LoadImage(const char* filename, b32 flipY, u32 forceBPP, Allocator* allocator) {
    void* data = nullptr;
    int width;
    int height;
    i32 channels;
    u32 channelSize;

    stbi_set_flip_vertically_on_load(flipY ? 1 : 0);

    int n;
    data = stbi_load(filename, &width, &height, &n, forceBPP);
    channelSize = sizeof(u8);
    channels = forceBPP ? forceBPP : n;

    LoadImageResult* header = nullptr;

    if (data) {
        auto bitmapSize = channelSize * width * height * channels;
        auto size = sizeof(LoadImageResult) + bitmapSize;
        auto memory = allocator->Alloc(size, false);
        header = (LoadImageResult*)memory;
        header->base = memory;
        header->bits = (byte*)memory + sizeof(LoadImageResult);
        header->width = width;
        header->height = height;
        header->channels = channels;

        memcpy(header->bits, data, bitmapSize);
        stbi_image_free(data);

        log_print("[Resource loader] Successfully load image %s\n", filename);
    } else {
        log_print("[Resource loader] Failed to image %s\n", filename);
    }


    return header;
}

// TODO: remove stb heap

bool LoadFontTrueType(Font* result, const char* filename, Allocator* allocator, u32 bitmapDim, f32 height, CodepointRange* ranges, u32 rangeCount) {
    bool success = false;
    auto fileSize = Platform.DebugGetFileSize(filename);
    if (fileSize) {
        auto data = (unsigned char*)Platform.HeapAlloc(GetContext()->mainHeap, fileSize, false);
        defer { Platform.Free(data); };
        if (data) {
            u32 bytesRead = Platform.DebugReadFile(data, fileSize, filename);
            if (bytesRead) {
                stbtt_fontinfo font;
                if (stbtt_InitFont(&font, data, 0)) {
                    auto scale = stbtt_ScaleForPixelHeight(&font, height);
                    int ascent = 0;
                    int descent = 0;
                    int lineGap = 0;
                    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
                    stbtt_pack_context pack;

                    uptr glyphTableLength = CalcGlyphTableLength(ranges, rangeCount);
                    result->bitmap = (u8*)allocator->Alloc(sizeof(u8) * bitmapDim * bitmapDim, false);
                    result->glyphs = (GlyphInfo*)allocator->Alloc((u32)sizeof(GlyphInfo) * (u32)glyphTableLength, false);

                    if (result->bitmap && result->glyphs) {
                        result->bitmapSize = bitmapDim;
                        result->glyphCount = (u32)glyphTableLength;

                        if (stbtt_PackBegin(&pack, (unsigned char*)result->bitmap, result->bitmapSize, result->bitmapSize, 0, 1, nullptr)) {
                            defer { stbtt_PackEnd(&pack); };

                            stbtt_PackSetOversampling(&pack, 2, 2);
                            stbtt_PackSetSkipMissingCodepoints(&pack, 0);

                            // Calculate range lengths
                            u32 totalCodepointCount = 0;
                            assert(rangeCount);
                            for (u32 i = 0; i < rangeCount; i++) {
                                assert(ranges[i].end > ranges[i].begin);
                                ranges[i]._count = ranges[i].end - ranges[i].begin + 1;
                                totalCodepointCount += ranges[i]._count;
                            }

                            assert(totalCodepointCount == (result->glyphCount - 1));

                            // TODO: Joint allocations
                            stbtt_pack_range* stbttRanges = (stbtt_pack_range*)Platform.HeapAlloc(GetContext()->mainHeap, sizeof(stbtt_pack_range) * rangeCount, true);
                            stbtt_packedchar* chars = (stbtt_packedchar*)Platform.HeapAlloc(GetContext()->mainHeap, sizeof(stbtt_packedchar) * totalCodepointCount, true);
                            defer { if (stbttRanges) Platform.Free(stbttRanges); };
                            defer { if (chars) Platform.Free(chars); };

                            if (stbttRanges && chars) {
                                u32 charDataOffset = 0;
                                for (u32 i = 0; i < rangeCount; i++) {
                                    stbttRanges[i].font_size = height;
                                    stbttRanges[i].first_unicode_codepoint_in_range = ranges[i].begin;
                                    stbttRanges[i].num_chars = ranges[i]._count;
                                    stbttRanges[i].chardata_for_range = chars + charDataOffset;
                                    charDataOffset += ranges[i]._count;
                                }

                                stbtt_PackFontRanges(&pack, data, 0, stbttRanges, rangeCount);

                                result->ascent = (f32)ascent * scale;
                                result->descent = (f32)descent * scale;
                                result->lineGap = (f32)lineGap * scale;

                                result->height = height;

                                u16 dummyCodepoint = ranges[0].begin;
                                auto dummyChar = chars;
                                result->glyphs[0].codepoint = dummyCodepoint;
                                result->glyphs[0].uv0.x = dummyChar->x0 / (f32)result->bitmapSize;
                                result->glyphs[0].uv0.y = dummyChar->y0 / (f32)result->bitmapSize;
                                result->glyphs[0].uv1.x = dummyChar->x1 / (f32)result->bitmapSize;
                                result->glyphs[0].uv1.y = dummyChar->y1 / (f32)result->bitmapSize;
                                result->glyphs[0].quadMin.x = dummyChar->xoff;
                                result->glyphs[0].quadMin.y = dummyChar->yoff;
                                result->glyphs[0].quadMax.x = dummyChar->xoff2;
                                result->glyphs[0].quadMax.y = dummyChar->yoff2;
                                result->glyphs[0].xAdvance = dummyChar->xadvance;

                                u32 k = 1;
                                for (u32 i = 0; i < rangeCount; i++) {
                                    auto range = ranges + i;
                                    for (u32 j = 0; j < range->_count; j++) {
                                        auto c = chars + (k - 1);

                                        //TODO: Mind overflow
                                        u16 codepoint = range->begin + j;

                                        result->glyphs[k].codepoint = codepoint;
                                        result->glyphs[k].uv0.x = c->x0 / (f32)result->bitmapSize;
                                        result->glyphs[k].uv0.y = c->y0 / (f32)result->bitmapSize;
                                        result->glyphs[k].uv1.x = c->x1 / (f32)result->bitmapSize;
                                        result->glyphs[k].uv1.y = c->y1 / (f32)result->bitmapSize;
                                        result->glyphs[k].quadMin.x = c->xoff;// * scale;
                                        result->glyphs[k].quadMin.y = c->yoff;// * scale;
                                        result->glyphs[k].quadMax.x = c->xoff2;// * scale;
                                        result->glyphs[k].quadMax.y = c->yoff2;// * scale;
                                        result->glyphs[k].xAdvance = c->xadvance;// * scale;

                                        result->glyphIndexTable[codepoint] = (u16)k;
                                        k++;
                                    }
                                }
                                success = true;
                            }
                        }
                    }
                    if (!success) {
                        allocator->Dealloc(result->bitmap);
                        allocator->Dealloc(result->glyphs);
                    }
                }
            }
        }
    }

    // Allocate dummy empty glyph
    if (!success) {
        result->glyphs = (GlyphInfo*)allocator->Alloc((u32)sizeof(GlyphInfo), true);
    }

    return success;
}

#define ParseBMEntryU32(text, name, result) _ParseBMEntryU32(text, name, sizeof(name) - 1, result)
bool _ParseBMEntryU32(char* text, const char* name, u32 nameLen, u32* result) {
    bool success = false;
    char* loc = strstr(text, name);
    if (loc) {
        loc += nameLen;
        auto parseResult = StringToInt(loc);
        if (parseResult.succeed) {
            if (parseResult.value >= 0) {
                *result = (u32)parseResult.value;
                success = true;
            }
        }
    }
    return success;
}

#define ParseBMEntryI32(text, name, result) _ParseBMEntryI32(text, name, sizeof(name) - 1, result)
bool _ParseBMEntryI32(char* text, const char* name, u32 nameLen, i32* result) {
    bool success = false;
    char* loc = strstr(text, name);
    if (loc) {
        loc += nameLen;
        auto parseResult = StringToInt(loc);
        if (parseResult.succeed) {
            *result = parseResult.value;
            success = true;
        }
    }
    return success;
}

struct BMFontDesc {
    u32 size;
    u32 lineHeight;
    u32 base;
    u32 scaleW;
    u32 scaleH;
    u32 charsCount;
    const char* file;
    char* chars;
};

bool ParseFontHeaderBM(char* text, BMFontDesc* desc) {
    bool result = true;
    if (!ParseBMEntryU32(text, "size=", &(desc->size))) result = false;
    if (!ParseBMEntryU32(text, "lineHeight=", &(desc->lineHeight))) result = false;
    if (!ParseBMEntryU32(text, "base=", &(desc->base))) result = false;
    if (!ParseBMEntryU32(text, "scaleW=", &(desc->scaleW))) result = false;
    if (!ParseBMEntryU32(text, "scaleH=", &(desc->scaleH))) result = false;
    if (!ParseBMEntryU32(text, "chars count=", &(desc->charsCount))) result = false;

    char* chars = strstr(text, "char id");
    if (!chars) {
        result = false;
    }

    desc->chars = chars;

    bool fileParsed = false;
    char* loc = strstr(text, "file");
    if (loc) {
        loc += sizeof("file=");
        char* end = strstr(loc + 1, "\"");
        if (end) {
            *end = 0;
            desc->file = loc;
            fileParsed = true;
        }
    }

    if (!fileParsed) {
        result = false;
    }

    return result;
}

struct BMCharDesc {
    u32 charID;
    u32 x;
    u32 y;
    u32 width;
    u32 height;
    i32 xOffset;
    i32 yOffset;
    i32 xAdvance;
};

bool ParseCharEntryBM(char** text, BMFontDesc* font, GlyphInfo* info) {
    u32 charID = 0;
    if (!ParseBMEntryU32(*text, "char id=", &charID)) return false;
    if (charID > U16::Max) return false;
    info->codepoint = (u16)charID;

    u32 x = 0;
    u32 y = 0;
    u32 width = 0;
    u32 height = 0;
    i32 xOffset = 0;
    i32 yOffset = 0;
    i32 xAdvance = 0;
    if (!ParseBMEntryU32(*text, "x=", &x)) return false;
    if (!ParseBMEntryU32(*text, "y=", &y)) return false;
    if (!ParseBMEntryU32(*text, "width=", &width)) return false;
    if (!ParseBMEntryU32(*text, "height=", &height)) return false;
    if (!ParseBMEntryI32(*text, "xoffset=", &xOffset)) return false;
    if (!ParseBMEntryI32(*text, "yoffset=", &yOffset)) return false;
    if (!ParseBMEntryI32(*text, "xadvance=", &xAdvance)) return false;

    info->uv0.x = (f32)x / font->scaleW;
    info->uv0.y = (f32)y / font->scaleH;
    info->uv1.x = ((f32)x + (f32)width) / font->scaleW;
    info->uv1.y = ((f32)y + (f32)height) / font->scaleH;
    info->quadMin.x = (f32)xOffset;
    info->quadMin.y = (f32)yOffset - font->base;
    info->quadMax.x = (f32)width + (f32)xOffset;//(f32)xOffset + (f32)width;
    info->quadMax.y = (f32)yOffset + (f32)height - font->base;
    info->xAdvance = (f32)Max(xAdvance, 0);

    while (**text && (**text != '\n')) (*text)++;
    (*text)++;

    return true;
}

bool LoadFontBM(Font* result, const char* filename, Allocator* allocator) {
    bool success = false;
    auto fileSize = Platform.DebugGetFileSize(filename);
    if (fileSize) {
        auto data = (unsigned char*)Platform.HeapAlloc(GetContext()->mainHeap, fileSize + 1, false);
        defer { Platform.Free(data); };
        if (data) {
            u32 bytesRead = Platform.DebugReadTextFile(data, fileSize + 1, filename);
            if (bytesRead) {
                char* text = (char*)data;
                BMFontDesc desc {};
                if (ParseFontHeaderBM(text, &desc)) {
                    if (desc.scaleW == desc.scaleH) {
                        result->glyphs = (GlyphInfo*)allocator->Alloc((u32)sizeof(GlyphInfo) * desc.charsCount, false);
                        result->bitmap = (u8*)allocator->Alloc(sizeof(u8) * desc.scaleW * desc.scaleW, false);
                        if (result->glyphs) {
                            result->bitmapSize = desc.scaleW;
                            result->glyphCount = desc.charsCount;

                            u32 count = 0;
                            char* at  = desc.chars;
                            char** atptr = &at;
                            GlyphInfo info {};
                            while (ParseCharEntryBM(atptr, &desc, &info)) {
                                result->glyphs[count] = info;
                                result->glyphIndexTable[info.codepoint] = count;
                                count++;
                            }

                            // prepare path
                            auto dirEndIndex = FindLastDirSeparator(filename);
                            bool alloc = false;
                            const char* bitmapName;
                            if (dirEndIndex != -1) {
                                auto bitmapNameLen = (u32)strlen(desc.file);
                                auto builderAllocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, GetContext()->mainHeap);
                                StringBuilder builder {};
                                StringBuilderInit(&builder, builderAllocator, filename, dirEndIndex + 1, bitmapNameLen);
                                StringBuilderAppend(&builder, desc.file, bitmapNameLen);
                                bitmapName = StringBuilderToString(&builder);
                                alloc = true;
                            } else {
                                bitmapName = desc.file;
                            }

                            stbi_set_flip_vertically_on_load(0);

                            int n;
                            int width;
                            int height;
                            auto imageData = stbi_load(bitmapName, &width, &height, &n, 1);

                            if (alloc) {
                                // NOTE: Converting const ptr to nonconst might cause
                                // some UB craziness. Is const_cast prevents this?
                                Platform.Free(const_cast<char*>(bitmapName));
                            }

                            if (imageData && width == result->bitmapSize && height == result->bitmapSize) {
                                memcpy(result->bitmap, imageData, sizeof(u8) * desc.scaleW * desc.scaleW);
                                stbi_image_free(imageData);
                                f32 pixelSize = 1.0f / desc.scaleW;
                                result->height = (f32)desc.size;
                                result->ascent = (f32)desc.base;
                                result->descent = -(f32)(desc.lineHeight - desc.base);
                                result->lineGap = 0.0f;
                                success = true;
                            } else {
                                allocator->Dealloc(result->glyphs);
                                allocator->Dealloc(result->bitmap);
                            }
                        }
                    }
                }
            }
        }
    }
    // Allocate dummy empty glyph
    if (!success) {
        result->glyphs = (GlyphInfo*)allocator->Alloc((u32)sizeof(GlyphInfo), true);
    }

    return success;
}
