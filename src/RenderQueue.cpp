#include "RenderQueue.h"

void RenderQueueInit(RenderQueue* queue, u32 size) {
    queue->rectBufferSize = size;
    queue->rectBufferAt = 0;
    queue->lineBufferSize = size;
    queue->lineBufferAt = 0;
    queue->rectBuffer = (RenderCommand*)PlatformAllocate(sizeof(RenderCommand) * size, 0, nullptr);
    queue->lineBuffer = (RenderCommand*)PlatformAllocate(sizeof(RenderCommand) * size, 0, nullptr);
    assert(queue->rectBuffer);
    assert(queue->lineBuffer);
}

void RenderQueuePush(RenderQueue* queue, RenderCommand command) {
    switch (command.type) {
    case RenderCommandType::RectColor: {
        if (queue->rectBufferAt < queue->rectBufferSize) {
            queue->rectBuffer[queue->rectBufferAt] = command;
            queue->rectBufferAt++;
        }
    } break;
    case RenderCommandType::Line: {
        if (queue->lineBufferAt < queue->lineBufferSize) {
            queue->lineBuffer[queue->lineBufferAt] = command;
            queue->lineBufferAt++;
        }
    } break;
    invalid_default();
    }
}

void RenderQueueReset(RenderQueue* queue) {
    queue->rectBufferAt = 0;
    queue->lineBufferAt = 0;
}

void DrawQuad(RenderQueue* queue, v2 min, v2 max, f32 z, v4 color) {
    RenderCommand command {};
    command.type = RenderCommandType::RectColor;
    command.rectColor.min = min;
    command.rectColor.max = max;
    command.rectColor.color = color;
    command.rectColor.z = z;

    RenderQueuePush(queue, command);
}

void DrawLine(RenderQueue* queue, v3 begin, v3 end, v4 color) {
    RenderCommand command {};
    command.type = RenderCommandType::Line;
    command.line.begin = begin;
    command.line.end = end;
    command.line.color = color;

    RenderQueuePush(queue, command);
}
