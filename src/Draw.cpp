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

void DrawListPushQuad(DrawList* list, v2 lb, v2 rb, v2 rt, v2 lt, f32 z, v4 color) {
    auto vertexOffset = list->vertexBuffer.Count();
    list->vertexBuffer.PushBack({V3(lb, z), color});
    list->vertexBuffer.PushBack({V3(rb, z), color});
    list->vertexBuffer.PushBack({V3(rt, z), color});
    list->vertexBuffer.PushBack({V3(lt, z), color});

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

    list->commandBuffer.PushBack(command);
}
