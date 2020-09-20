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

BakeFontResult ResourceLoaderBakeFont(const char* filename, Allocator* allocator, f32 height, u32 bitmapDim) {
    BakeFontResult result {};
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
                    result.bitmap = allocator->Alloc(sizeof(u8) * bitmapDim * bitmapDim, false);
                    if (result.bitmap) {
                        if (stbtt_PackBegin(&pack, (unsigned char*)result.bitmap, bitmapDim, bitmapDim, 0, 1, nullptr)) {
                            stbtt_PackSetOversampling(&pack, 1, 1);
                            stbtt_pack_range ranges[1] {};
                            stbtt_packedchar chars[94];
                            ranges[0].font_size = height;
                            ranges[0].first_unicode_codepoint_in_range = 32;
                            ranges[0].num_chars = 94;
                            ranges[0].chardata_for_range = chars;
                            stbtt_PackFontRanges(&pack, data, 0, ranges, array_count(ranges));
                            stbtt_PackEnd(&pack);
                        }
                    }
                }
            }
        }
    }
    return result;
}
