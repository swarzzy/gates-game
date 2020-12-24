#pragma once

#include "Common.h"
#include "Math.h"

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

struct RendererAPI {
    void(*SetCamera)(m4x4* projection);
    void(*SubmitDrawList)(DrawList* list);
    TextureID(*UploadTexture)(TextureID id, u32 width, u32 height, TextureFormat format, TextureFilter filter, TextureWrapMode wrapMode, void* data);
};
