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

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, v2 uv0, v2 uv1, v2 uv2, v2 uv3, f32 z, v4 color, TextureID texture, f32 texBlend, TextureMode mode) {
    auto vertexOffset = list->vertexBuffer.Count();
    list->vertexBuffer.PushBack(Vertex(V3(lb, z), texBlend, color, uv0));
    list->vertexBuffer.PushBack(Vertex(V3(rb, z), texBlend, color, uv1));
    list->vertexBuffer.PushBack(Vertex(V3(rt, z), texBlend, color, uv2));
    list->vertexBuffer.PushBack(Vertex(V3(lt, z), texBlend, color, uv3));

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
    DrawListPushQuad(list, lb, rb, rt, lt, V2(0.0f), V2(1.0f, 0.0f), V2(1.0f), V2(0.0f, 1.0f), z, color, 0, 0.0f, TextureMode::Color);
}

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture) {
    DrawListPushQuad(list, lb, rb, rt, lt, V2(0.0f), V2(1.0f, 0.0f), V2(1.0f), V2(0.0f, 1.0f), z, {}, texture, 1.0f, TextureMode::Color);
}

void DrawListPushQuadAlphaMask(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, TextureID texture, v4 color) {
    DrawListPushQuad(list, lb, rb, rt, lt, V2(0.0f), V2(1.0f, 0.0f), V2(1.0f), V2(0.0f, 1.0f), z, color, texture, 1.0f, TextureMode::AlphaMask);
}

void DrawListPushGlyph(DrawList* list, v2 min, v2 max, v2 uv0, v2 uv1, f32 z, v4 color, TextureID atlas) {
    DrawListPushQuad(list, min, V2(max.x, min.y), max, V2(min.x, max.y), uv0, V2(uv1.x, uv0.y), uv1, V2(uv0.x, uv1.y), z, color, atlas, 1.0f, TextureMode::AlphaMask);
}
