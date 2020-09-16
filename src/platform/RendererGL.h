#pragma once

#include "OpenGL.h"

struct Renderer {

};

void RendererInit(Renderer* renderer);
void RenderSetState(const m4x4* projection, v4 clearColor);
//void RenderDrawList(DrawList* list);
