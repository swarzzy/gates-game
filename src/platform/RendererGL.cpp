#include "RendererGL.h"
#if 0
static Renderer* _GlobalRenderer;
inline Renderer* GetRenderer() { return _GlobalRenderer; }

void RendererInit(Renderer* renderer) {
    _GlobalRenderer = renderer;

    GLuint globalVAO;
    GL.glGenVertexArrays(1, &globalVAO);
    GL.glBindVertexArray(globalVAO);

    GL.glEnable(GL_DEPTH_TEST);
    GL.glDepthFunc(GL_LEQUAL);
    GL.glEnable(GL_CULL_FACE);
    GL.glCullFace(GL_BACK);
    GL.glFrontFace(GL_CCW);
    // TODO(swarzzy): Multisampling
    //glEnable(GL_MULTISAMPLE);

    //GL.glLineWidth(3.0f);

    GL.glClearDepth(1.0f);

    glGenBuffers(1, &renderer->vertexBuffer);
    assert(renderer->vertexBuffer);

    glGenBuffers(1, &renderer->indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u16) * Renderer::MaxBufferCapacity, nullptr, GL_STATIC_DRAW);
    u16* indexData = (u16*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

    static_assert((Renderer::MaxBufferCapacity % 6) == 0);

    usize k = 0;
    for (usize i = 0; i < Renderer::MaxBufferCapacity; i+= 6) {
        indexData[i + 0] = k;
        indexData[i + 1] = k + 1;
        indexData[i + 2] = k + 2;
        indexData[i + 3] = k + 2;
        indexData[i + 4] = k + 3;
        indexData[i + 5] = k;
        k += 4;
    }

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    renderer->rectColorOpaqueShader = CompileGLSL("RectColorOpaque", ColorRectOpaqueShaderVertex, ColorRectOpaqueShaderFragment);
    assert(renderer->rectColorOpaqueShader);
    renderer->mvpLocation = glGetUniformLocation(renderer->rectColorOpaqueShader, "MVP");
    assert(renderer->mvpLocation != -1);

    renderer->lineShader = CompileGLSL("LineShader", LineShaderVertex, LineShaderFragment);
    assert(renderer->lineShader);
    renderer->mvpLocationLine = glGetUniformLocation(renderer->lineShader, "MVP");
    assert(renderer->mvpLocationLine != -1);
}
#endif
