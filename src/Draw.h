#pragma once

#include "Common.h"

#include "GrowableArray.h"

struct Font;

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
    Color, AlphaMask
};

struct DrawCommand {
    u32 vertexBufferOffset;
    u32 indexBufferOffset;
    u32 indexCount;
    TextureMode textureMode;
    TextureID texture;
};

struct DrawList {
    GrowableArray<DrawCommand> commandBuffer;
    GrowableArray<Vertex> vertexBuffer;
    GrowableArray<u32> indexBuffer;
    b32 pendingCommand;
    DrawCommand scratchCommand;
};

// TODO: Move somewhere else
struct Rectangle2 {
    v2 min;
    v2 max;
};

void DrawListInit(DrawList* list, u32 capacity, Allocator allocator);

void DrawListClear(DrawList* list);

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, v4 color);
void DrawListPushGlyph(DrawList* list, v2 min, v2 max, v2 uv0, v2 uv1, f32 z, v4 color, TextureID atlas);
void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture);
void DrawListPushQuadAlphaMask(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture, v4 color);
void DrawText(DrawList* list, Font* font, const char16* string, v2 p, f32 z, v4 color, v2 pixelSize, v2 anchor = {}, f32 maxWidth = F32::Infinity);
Rectangle2 CalcTextBoundingBox(Font* font, const char16* string, v2 pixelSize, v2 anchor = {}, f32 maxWidth = F32::Infinity);

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
