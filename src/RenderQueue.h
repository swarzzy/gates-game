#pragma once

#include "Common.h"

enum struct RenderCommandType : u32 {
    RectColor, Line
};

struct RenderCommand {
    RenderCommandType type;

    b32 transparent;

    union {
        struct {
            v2 min;
            v2 max;
            v4 color;
            f32 z;
        } rectColor;
        struct {
            v3 begin;
            v3 end;
            v4 color;
            // Not supported for now
            f32 thickness;
        } line;
    };
};

struct RenderQueue {
    u32 rectBufferSize;
    u32 rectBufferAt;
    u32 lineBufferSize;
    u32 lineBufferAt;
    RenderCommand* rectBuffer;
    // TODO(swarzzy): Should we use separate buffers for different command or just
    // use one and sort it?
    RenderCommand* lineBuffer;
};

void RenderQueueInit(RenderQueue* queue, u32 size);
void RenderQueuePush(RenderQueue* queue, RenderCommand command);
void RenderQueueReset(RenderQueue* queue);

// Helpers
void DrawQuad(RenderQueue* queue, v2 min, v2 max, f32 z, v4 color);
void DrawLine(RenderQueue* queue, v3 begin, v3 end, v4 color);
