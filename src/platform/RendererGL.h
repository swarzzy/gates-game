#pragma once

#include "OpenGL.h"

struct DrawList;

struct StandardShader {
    GLuint handle;
    GLint MVP;
    GLuint uTexture;
    u32 textureSampler;
    GLuint textureSlot;
};

struct AlphaMaskShader {
    GLuint handle;
    GLint MVP;
    GLuint uTexture;
    u32 textureSampler;
    GLuint textureSlot;
};

struct DistanceFieldShader {
    GLuint handle;
    GLint MVP;
    GLuint uTexture;
    GLuint uParams;
    u32 textureSampler;
    GLuint textureSlot;
};

struct Renderer {
    GLuint vertexBuffer;
    GLuint indexBuffer;
    StandardShader standardShader;
    AlphaMaskShader alphaMaskShader;
    DistanceFieldShader distanceFieldShader;
};

void RendererInit(Renderer* renderer);
void RenderSetCamera(m4x4* projection);
void RenderDrawList(DrawList* list);

TextureID RendererUploadTexture(TextureID id, u32 width, u32 height, TextureFormat format, TextureFilter filterMag, TextureWrapMode wrapMode, void* data);

#define BindShaderUniform(s, uni) (s)->##uni = GL.glGetUniformLocation((s)->handle, #uni);

GLuint CompileGLSL(const char* name, const char* vertexSource, const char* fragmentSource);
//void RenderDrawList(DrawList* list);
