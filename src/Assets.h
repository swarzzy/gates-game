#pragma once

struct GlyphInfo {
    v2 uv0;
    v2 uv1;
    v2 quadMin;
    v2 quadMax;
    f32 xAdvance;
    u16 codepoint;
};

struct Font {
    // Should be filled by caller
    TextureID atlas;
    b32 sdf;
    v2 sdfParams;

    GlyphInfo* glyphs;
    u32 glyphCount;
    u32 bitmapSize;
    u8* bitmap;
    f32 height;
    f32 ascent;
    f32 descent;
    f32 lineGap;
    u16 glyphIndexTable[U16::Max];
};

struct CodepointRange {
    u32 begin;
    u32 end;

    u32 _count; // Do not fill. Used internally
};

struct LoadImageResult {
    void* base;
    void* bits;
    u32 width;
    u32 height;
    u32 channels;
};

static_assert((sizeof(LoadImageResult) % 4) == 0);

u32 CalcGlyphTableLength(CodepointRange* ranges, u32 rangeCount);
bool LoadFontTrueType(Font* result, const char* filename, Allocator* allocator, u32 bitmapDim, f32 height, CodepointRange* ranges, u32 rangeCount);
bool LoadFontBM(Font* result, const char* filename, Allocator* allocator);
void FontFreeResources(Font* font, Allocator* allocator);
LoadImageResult* LoadImage(const char* filename, b32 flipY, u32 forceBPP, Allocator* allocator);
