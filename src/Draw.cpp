#include "Draw.h"

void DrawListInit(DrawList* list, u32 capacity, Allocator allocator) {
    assert(!list->commandBuffer.Capacity());
    assert(!list->vertexBuffer.Capacity());
    assert(!list->indexBuffer.Capacity());

    list->commandBuffer = GrowableArray<DrawCommand>(allocator);
    list->vertexBuffer = GrowableArray<Vertex>(allocator);
    list->indexBuffer = GrowableArray<u32>(allocator);
}

void DrawListClear(DrawList* list) {
    list->commandBuffer.Clear();
    list->vertexBuffer.Clear();
    list->indexBuffer.Clear();
}

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, v4 color, TextureID texture, f32 texBlend, TextureMode mode) {
    auto vertexOffset = list->vertexBuffer.Count();
    list->vertexBuffer.PushBack(Vertex(V3(lb, z), texBlend, color, V2(0.0f, 0.0f)));
    list->vertexBuffer.PushBack(Vertex(V3(rb, z), texBlend, color, V2(1.0f, 0.0f)));
    list->vertexBuffer.PushBack(Vertex(V3(rt, z), texBlend, color, V2(1.0f, 1.0f)));
    list->vertexBuffer.PushBack(Vertex(V3(lt, z), texBlend, color, V2(0.0f, 1.0f)));

    auto indexOffset = list->indexBuffer.Count();;

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
    DrawListPushQuad(list, lb, rb, rt, lt, z, color, 0, 0.0f, TextureMode::Color);
}

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture) {
    DrawListPushQuad(list, lb, rb, rt, lt, z, {}, texture, 1.0f, TextureMode::Color);
}

void DrawListPushQuadAlphaMask(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture, v4 color) {
    DrawListPushQuad(list, lb, rb, rt, lt, z, color, texture, 1.0f, TextureMode::AlphaMask);
}
