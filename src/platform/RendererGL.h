#pragma once

#include "OpenGL.h"

struct DrawList;

struct StandardShader {
    GLuint handle;
    GLint MVP;
};

struct Renderer {
    GLuint vertexBuffer;
    GLuint indexBuffer;
    StandardShader standardShader;
};

void RendererInit(Renderer* renderer);
void RenderSetState(const m4x4* projection, v4 clearColor);
void RenderDrawList(DrawList* list);


#define BindShaderUniform(s, uni) (s)->##uni = GL.glGetUniformLocation((s)->handle, #uni);

GLuint CompileGLSL(const char* name, const char* vertexSource, const char* fragmentSource);
//void RenderDrawList(DrawList* list);
