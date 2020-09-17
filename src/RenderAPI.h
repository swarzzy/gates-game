#pragma once

#include "Common.h"
#include "Draw.h"

typedef void(RendererBeginFrameFn)(const m4x4* projection, v4 clearColor);
typedef void(RendererDrawListFn)(DrawList* list);

struct RendererAPI {
    RendererBeginFrameFn* BeginFrame;
    RendererDrawListFn* RenderDrawList;
};
