#pragma once

#include "Common.h"

#include "GrowableArray.h"

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

struct DrawCommand {
    u32 vertexBufferOffset;
    u32 indexBufferOffset;
    u32 indexCount;
    TextureID texture;
};

struct DrawList {
    GrowableArray<DrawCommand> commandBuffer;
    GrowableArray<Vertex> vertexBuffer;
    GrowableArray<u32> indexBuffer;
};

void DrawListInit(DrawList* list, u32 capacity, Allocator allocator);

void DrawListClear(DrawList* list);

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, v4 color);
void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture);

// TODO: Calling convention
// TODO: Single call
// Renderer API
typedef void(RendererBeginFrameFn)(const m4x4* projection, v4 clearColor);
typedef void(RendererDrawListFn)(DrawList* list);
typedef TextureID(RendererUploadTextureFn)(TextureID id, u32 width, u32 height, TextureFormat format, TextureFilter filter, TextureWrapMode wrapMode, void* data);


struct RendererAPI {
    RendererBeginFrameFn* BeginFrame;
    RendererDrawListFn* RenderDrawList;
    RendererUploadTextureFn* UploadTexture;
};
