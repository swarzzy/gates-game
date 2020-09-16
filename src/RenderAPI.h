#pragma once

#include "Common.h"
#include "Draw.h"

typedef void(RenderSetStateFn)(const m4x4* projection, v4 clearColor);
typedef void(RenderDrawListFn)(DrawList* list);
