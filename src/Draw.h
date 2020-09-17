#pragma once

#include "Common.h"

#include "GrowableArray.h"

struct Vertex {
    v3 position;
    v4 color;
};

struct DrawCommand {
    u32 vertexBufferOffset;
    u32 indexBufferOffset;
    u32 indexCount;
};

struct DrawList {
    GrowableArray<DrawCommand> commandBuffer;
    GrowableArray<Vertex> vertexBuffer;
    GrowableArray<u32> indexBuffer;
};

void DrawListInit(DrawList* list, u32 capacity, Allocator allocator);

void DrawListClear(DrawList* list);

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, v4 color);
