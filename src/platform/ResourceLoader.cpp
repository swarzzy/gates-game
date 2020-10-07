#include "ResourceLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT(x) assert(x)
#define STBI_MALLOC(sz)           HeapAlloc(ResourceLoaderScratchHeap, (usize)(sz), false)
#define STBI_REALLOC(p,newsz)     HeapRealloc(ResourceLoaderScratchHeap, (p), (usize)(newsz), false)
#define STBI_FREE(p)              Free(p)

#include "../../ext/stb_image-2.26/stb_image.h"

#include "../../ext/stb_truetype-1.24/stb_truetype.h"

void __cdecl ResourceLoaderInvoke(ResourceLoaderCommand command, void* _args, void* _result) {
    switch (command) {
    case ResourceLoaderCommand::Image: {
        auto args = (LoadImageArgs*)_args;
        auto result = (LoadImageResult**)_result;
        *result = ResourceLoaderLoadImage(args->filename, args->flipY, args->forceBitsPerPixel, args->allocator);
    } break;
    invalid_default();
    }
}

LoadImageResult* ResourceLoaderLoadImage(const char* filename, b32 flipY, u32 forceBPP, Allocator* allocator) {
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

void ResourceLoaderBakeFont(BakeFontResult* result, const char* filename, Allocator* allocator, f32 height, CodepointRange* ranges, u32 rangeCount) {
    assert(result->bitmap);
    assert(result->glyphs);
    auto fileSize = DebugGetFileSize(filename);
    if (fileSize) {
        auto data = (unsigned char*)HeapAlloc(ResourceLoaderScratchHeap, fileSize, false);
        defer { Free(data); };
        if (data) {
            u32 bytesRead = DebugReadFileToBuffer(data, fileSize, filename);
            if (bytesRead) {
                stbtt_fontinfo font;
                if (stbtt_InitFont(&font, data, 0)) {
                    auto scale = stbtt_ScaleForPixelHeight(&font, height);
                    stbtt_pack_context pack;
                    //result.bitmap = allocator->Alloc(sizeof(u8) * bitmapDim * bitmapDim, false);
                    if (stbtt_PackBegin(&pack, (unsigned char*)result->bitmap, result->bitmapSize, result->bitmapSize, 0, 1, nullptr)) {
                        stbtt_PackSetOversampling(&pack, 1, 1);
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
                        stbtt_pack_range* stbttRanges = (stbtt_pack_range*)HeapAlloc(ResourceLoaderScratchHeap, sizeof(stbtt_pack_range) * rangeCount, true);
                        stbtt_packedchar* chars = (stbtt_packedchar*)HeapAlloc(ResourceLoaderScratchHeap, sizeof(stbtt_packedchar) * totalCodepointCount, true);
                        defer { if (stbttRanges) Free(stbttRanges); };
                        defer { if (chars) Free(chars); };

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
                            stbtt_PackEnd(&pack);

                            //result.glyphs = (GlyphInfo*)HeapAlloc(ResourceLoaderScratchHeap, sizeof(GlyphInfo) * totalCodepointCount + 1, false);
                            //memset(result.glyphIndexTable, 0, sizeof(u16) * array_count(result.glyphIndexTable));
                            //result->glyphCount = totalCodepointCount;

                            u16 dummyCodepoint = ranges[0].begin;
                            auto dummyChar = chars;
                            result->glyphs[0].codepoint = dummyCodepoint;
                            result->glyphs[0].x0 = dummyChar->x0 / (f32)result->bitmapSize;
                            result->glyphs[0].y0 = dummyChar->y0 / (f32)result->bitmapSize;
                            result->glyphs[0].x1 = dummyChar->x1 / (f32)result->bitmapSize;
                            result->glyphs[0].y1 = dummyChar->y1 / (f32)result->bitmapSize;
                            result->glyphs[0].xOff = dummyChar->xoff * scale;
                            result->glyphs[0].yOff = dummyChar->yoff * scale;
                            result->glyphs[0].xAdvance = dummyChar->xadvance * scale;
                            result->glyphs[0].xOff2 = dummyChar->xoff2 * scale;
                            result->glyphs[0].yOff2 = dummyChar->yoff2 * scale;

                            u32 k = 1;
                            for (u32 i = 0; i < rangeCount; i++) {
                                auto range = ranges + i;
                                for (u32 j = 0; j < range->_count; j++) {
                                    auto c = chars + (k - 1);

                                    //TODO: Mind overflow
                                    u16 codepoint = range->begin + j;

                                    result->glyphs[k].codepoint = codepoint;
                                    result->glyphs[k].x0 = c->x0 / (f32)result->bitmapSize;
                                    result->glyphs[k].y0 = c->y0 / (f32)result->bitmapSize;
                                    result->glyphs[k].x1 = c->x1 / (f32)result->bitmapSize;
                                    result->glyphs[k].y1 = c->y1 / (f32)result->bitmapSize;
                                    result->glyphs[k].xOff = c->xoff * scale;
                                    result->glyphs[k].yOff = c->yoff * scale;
                                    result->glyphs[k].xAdvance = c->xadvance * scale;
                                    result->glyphs[k].xOff2 = c->xoff2 * scale;
                                    result->glyphs[k].yOff2 = c->yoff2 * scale;

                                    result->glyphIndexTable[codepoint] = (u16)k;
                                    k++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
