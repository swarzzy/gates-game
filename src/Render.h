#pragma once

#include "RenderQueue.h"

struct Canvas {
    m4x4 projection;
    v4 clearColor;
};

struct Renderer {
    // Should be less than or equal to 2^16 and multiple of 6
    static const u32 MaxBufferCapacity = 1536;

    Canvas canvas;

    GLuint rectColorOpaqueShader;
    GLuint lineShader;
    GLuint vertexBuffer;
    GLuint indexBuffer;
    GLint mvpLocation;
    GLint mvpLocationLine;
};

void RendererInit(Renderer* renderer);
void RendererBeginFrame(Renderer* renderer);
void RendererDraw(Renderer* renderer, RenderQueue* queue);
void RendererEndFrame(Renderer* renderer);
