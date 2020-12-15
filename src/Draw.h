#pragma once

#include "Common.h"

#include "Array.h"

struct Font;
struct DrawList;

struct ParticleSource {
    v2 p;
    u32 maxCount;
    u32 count;
    RandomSeries randomSeries;
    Array<v2> positions;
    Array<v2> velocities;
    Array<f32> lifetimes;
    Array<f32> sizes;
    Array<v4> colors;
};

void InitParticleSource(ParticleSource* source, Allocator* allocator, v2 p, u32 poolSize);
void UpdateParticleSource(ParticleSource* source);
void RenderParticleSource(ParticleSource* source, DrawList* list);

enum struct TextureFilter : u32 {
    None = 0, Bilinear, Trilinear, Anisotropic, Default = Bilinear
};

enum struct TextureFormat : u32 {
    Unknown = 0,
    SRGBA8,
    SRGB8,
    RGBA8,
    RGB8,
    RGB16F,
    RG16F,
    R8,
    RG8,
    RG32F,
};

enum struct TextureWrapMode : u32 {
    Repeat = 0, ClampToEdge, Default = Repeat
};

struct Vertex {
    v3 position;
    f32 texBlendFactor;
    v4 color;
    v2 uv;
    Vertex(v3 p, f32 texFactor, v4 c, v2 texcoord) : position(p), texBlendFactor(texFactor), color(c), uv(texcoord) {}
};

typedef u64 TextureID;

enum struct TextureMode : u32 {
    Color, AlphaMask, DistanceField
};

struct DrawCommand {
    u32 vertexBufferOffset;
    u32 indexBufferOffset;
    u32 indexCount;
    TextureMode textureMode;
    TextureID texture;
    v2 distanceFieldParams;
};

struct DrawList {
    Array<DrawCommand> commandBuffer;
    Array<Vertex> vertexBuffer;
    Array<u32> indexBuffer;
    b32 pendingCommand;
    DrawCommand scratchCommand;
};

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

// TODO: Calling convention
// TODO: Single call
// Renderer API
typedef void(RenderSetCameraFn)(m4x4* projection);
typedef void(RendererDrawListFn)(DrawList* list);
typedef TextureID(RendererUploadTextureFn)(TextureID id, u32 width, u32 height, TextureFormat format, TextureFilter filter, TextureWrapMode wrapMode, void* data);


struct RendererAPI {
    RenderSetCameraFn* SetCamera;
    RendererDrawListFn* RenderDrawList;
    RendererUploadTextureFn* UploadTexture;
};
