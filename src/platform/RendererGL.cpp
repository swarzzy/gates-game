#include "RendererGL.h"

#include "../Draw.h"

static Renderer* _GlobalRenderer;
inline Renderer* GetRenderer() { return _GlobalRenderer; }

const char* StandardShaderVert = R"(
#version 330 core

layout (location = 0) in vec4 aPosition;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aUV;

out vec4 VertexColor;
out float TexBlendFactor;
out vec2 UV;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(aPosition.xyz, 1.0f);
    VertexColor = aColor;
    TexBlendFactor = aPosition.w;
    UV = aUV;
})";

const char* StandardShaderFrag = R"(
#version 330 core

out vec4 FragmentColor;

in vec4 VertexColor;
in float TexBlendFactor;
in vec2 UV;

uniform sampler2D uTexture;

void main() {
    vec4 sample = texture(uTexture, UV);
    vec4 color = vec4(VertexColor.xyz, 1.0f);
    float factor = step(0.5f, TexBlendFactor);
    FragmentColor = vec4(mix(color, sample, factor));
})";

const char* AlphaMaskShaderVert = R"(
#version 330 core

layout (location = 0) in vec4 aPosition;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aUV;

out vec4 VertexColor;
out vec2 UV;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(aPosition.xyz, 1.0f);
    VertexColor = aColor;
    UV = aUV;
})";

const char* AlphaMaskShaderFrag = R"(
#version 330 core

out vec4 FragmentColor;

in vec4 VertexColor;
in vec2 UV;

uniform sampler2D uTexture;

void main() {
    float alpha = texture(uTexture, UV).r;
    vec4 color = vec4(VertexColor.xyz, alpha);
    FragmentColor = color;
})";


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

    GL.glEnable(GL_BLEND);
    GL.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //GL.glLineWidth(3.0f);

    GL.glClearDepth(1.0f);

    GL.glGenBuffers(1, &renderer->vertexBuffer);
    assert(renderer->vertexBuffer);

    GL.glGenBuffers(1, &renderer->indexBuffer);
    assert(renderer->indexBuffer);

    renderer->standardShader.handle = CompileGLSL("StandardShader", StandardShaderVert, StandardShaderFrag);
    assert(renderer->standardShader.handle);

    BindShaderUniform(&renderer->standardShader, MVP);
    BindShaderUniform(&renderer->standardShader, uTexture);

    renderer->standardShader.textureSampler = 0;
    renderer->standardShader.textureSlot = GL_TEXTURE0;

    GL.glUseProgram(renderer->standardShader.handle);
    GL.glUniform1i(renderer->standardShader.uTexture, renderer->standardShader.textureSampler);

    renderer->alphaMaskShader.handle = CompileGLSL("AlphaMask", AlphaMaskShaderVert, AlphaMaskShaderFrag);
    assert(renderer->alphaMaskShader.handle);

    BindShaderUniform(&renderer->alphaMaskShader, MVP);
    BindShaderUniform(&renderer->alphaMaskShader, uTexture);

    renderer->alphaMaskShader.textureSampler = 0;
    renderer->alphaMaskShader.textureSlot = GL_TEXTURE0;

    GL.glUseProgram(renderer->alphaMaskShader.handle);
    GL.glUniform1i(renderer->alphaMaskShader.uTexture, renderer->alphaMaskShader.textureSampler);
    GL.glUseProgram(0);
}

void RenderSetCamera(m4x4* projection) {
    auto renderer = GetRenderer();
    GL.glUseProgram(renderer->standardShader.handle);
    GL.glUniformMatrix4fv(renderer->standardShader.MVP, 1, false, projection->data);

    GL.glUseProgram(renderer->alphaMaskShader.handle);
    GL.glUniformMatrix4fv(renderer->alphaMaskShader.MVP, 1, false, projection->data);
}

void RenderEndPass() {}

void RenderDrawList(DrawList* list) {
    auto renderer = GetRenderer();

    if (list->vertexBuffer.Count() && list->indexBuffer.Count()) {
        GL.glBindBuffer(GL_ARRAY_BUFFER, renderer->vertexBuffer);
        GL.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->indexBuffer);

        GL.glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * list->vertexBuffer.Count(), list->vertexBuffer.Begin(), GL_STREAM_DRAW);
        GL.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * list->indexBuffer.Count(), list->indexBuffer.Begin(), GL_STREAM_DRAW);

        GL.glEnableVertexAttribArray(0);
        GL.glEnableVertexAttribArray(1);
        GL.glEnableVertexAttribArray(2);

        GL.glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(Vertex), 0);
        GL.glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Vertex), (void*)sizeof(v4));
        GL.glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), (void*)(sizeof(v4) * 2));

        for (auto& cmd : list->commandBuffer) {
            GLuint textureSlot = 0;
            switch (cmd.textureMode) {
            case TextureMode::Color: { GL.glUseProgram(renderer->standardShader.handle); textureSlot = renderer->standardShader.textureSlot; } break;
            case TextureMode::AlphaMask: { GL.glUseProgram(renderer->alphaMaskShader.handle); textureSlot = renderer->alphaMaskShader.textureSlot; } break;
            invalid_default();
            }

            if (cmd.texture) {
                GL.glActiveTexture(textureSlot);
                GL.glBindTexture(GL_TEXTURE_2D, (GLuint)cmd.texture);
            }

            GL.glDrawElements(GL_TRIANGLES, cmd.indexCount, GL_UNSIGNED_INT, (void*)(u64)(cmd.indexBufferOffset * sizeof(u32)));
        }
    }
}

GLenum ToOpenGL(TextureWrapMode mode) {
    GLenum result;
    switch (mode) {
    case TextureWrapMode::Repeat: { result = GL_REPEAT; } break;
    case TextureWrapMode::ClampToEdge: { result = GL_CLAMP_TO_EDGE; } break;
        invalid_default();
    }
    return result;
}

struct GLTextureFormat {
    GLenum internal;
    GLenum format;
    GLenum type;
};

GLTextureFormat ToOpenGL(TextureFormat format) {
    GLTextureFormat result;
    switch (format) {
    case TextureFormat::SRGBA8: { result.internal = GL_SRGB8_ALPHA8; result.format = GL_RGBA; result.type = GL_UNSIGNED_BYTE; } break;
    case TextureFormat::SRGB8: { result.internal = GL_SRGB8; result.format = GL_RGB; result.type = GL_UNSIGNED_BYTE; } break;
    case TextureFormat::RGBA8: { result.internal = GL_RGBA8; result.format = GL_RGBA; result.type = GL_UNSIGNED_BYTE; } break;
    case TextureFormat::RGB8: { result.internal = GL_RGB8; result.format = GL_RGB; result.type = GL_UNSIGNED_BYTE; } break;
    case TextureFormat::RGB16F: { result.internal = GL_RGB16F; result.format = GL_RGB; result.type = GL_FLOAT; } break;
    case TextureFormat::RG16F: { result.internal = GL_RG16F; result.format = GL_RG; result.type = GL_FLOAT; } break;
    case TextureFormat::RG32F: { result.internal = GL_RG32F; result.format = GL_RG; result.type = GL_FLOAT; } break;
    case TextureFormat::R8: { result.internal = GL_R8; result.format = GL_RED; result.type = GL_UNSIGNED_BYTE; } break;
    case TextureFormat::RG8: { result.internal = GL_RG8; result.format = GL_RG; result.type = GL_UNSIGNED_BYTE; } break;
        invalid_default();
    }
    return result;
}

struct GLTextureFilter {
    GLenum min;
    GLenum mag;
    bool anisotropic;
};

GLTextureFilter ToOpenGL(TextureFilter filter) {
    GLTextureFilter result = {};
    switch (filter) {
    case TextureFilter::None: { result.min = GL_NEAREST; result.mag = GL_NEAREST; } break;
    case TextureFilter::Bilinear: { result.min = GL_LINEAR; result.mag = GL_LINEAR; } break;
    case TextureFilter::Trilinear: { result.min = GL_LINEAR_MIPMAP_LINEAR; result.mag = GL_LINEAR; } break;
    case TextureFilter::Anisotropic: { result.min = GL_LINEAR_MIPMAP_LINEAR; result.mag = GL_LINEAR; result.anisotropic = true; } break;
    }
    return result;
}

TextureID RendererUploadTexture(TextureID id, u32 width, u32 height, TextureFormat _format, TextureFilter _filter, TextureWrapMode _wrapMode, void* data) {
    TextureID result = 0;
    GLuint handle;
    if (!id) {
        GL.glGenTextures(1, &handle);
    } else {
        // TODO: Validate handle
        handle = (GLuint)id;
    }
    if (handle) {
        GL.glBindTexture(GL_TEXTURE_2D, handle);

        auto wrapMode = ToOpenGL(_wrapMode);
        auto format = ToOpenGL(_format);
        auto filter = ToOpenGL(_filter);

        GL.glTexImage2D(GL_TEXTURE_2D, 0, format.internal, width, height, 0, format.format, format.type, data);

        GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter.mag);
        GL.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter.min);

        // TODO: Mips control
        GL.glGenerateMipmap(GL_TEXTURE_2D);

        GL.glBindTexture(GL_TEXTURE_2D, 0);
        result = (TextureID)handle;
    }
    return result;
}

GLuint CompileGLSL(const char* name, const char* vertexSource, const char* fragmentSource) {
    GLuint resultHandle = 0;

    GLuint vertexHandle = GL.glCreateShader(GL_VERTEX_SHADER);
    if (vertexHandle) {
        GL.glShaderSource(vertexHandle, 1, &vertexSource, nullptr);
        GL.glCompileShader(vertexHandle);

        GLint vertexResult = 0;
        GL.glGetShaderiv(vertexHandle, GL_COMPILE_STATUS, &vertexResult);
        if (vertexResult) {
            GLuint fragmentHandle;
            fragmentHandle = GL.glCreateShader(GL_FRAGMENT_SHADER);
            if (fragmentHandle) {
                GL.glShaderSource(fragmentHandle, 1, &fragmentSource, nullptr);
                GL.glCompileShader(fragmentHandle);

                GLint fragmentResult = 0;
                GL.glGetShaderiv(fragmentHandle, GL_COMPILE_STATUS, &fragmentResult);
                if (fragmentResult) {
                    GLint programHandle;
                    programHandle = GL.glCreateProgram();
                    if (programHandle) {
                        GL.glAttachShader(programHandle, vertexHandle);
                        GL.glAttachShader(programHandle, fragmentHandle);
                        GL.glLinkProgram(programHandle);

                        GLint linkResult = 0;
                        GL.glGetProgramiv(programHandle, GL_LINK_STATUS, &linkResult);
                        if (linkResult) {
                            GL.glDeleteShader(vertexHandle);
                            GL.glDeleteShader(fragmentHandle);
                            resultHandle = programHandle;
                        } else {
                            i32 logLength;
                            GL.glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLength);
                            char* message = (char*)alloca(logLength);
                            GL.glGetProgramInfoLog(programHandle, logLength, 0, message);
                            log_print("[Error]: Failed to link shader program (%s) \n%s\n", name, message);
                        }
                    } else {
                        log_print("Failed to create shader program\n");
                    }
                } else {
                    GLint logLength;
                    GL.glGetShaderiv(fragmentHandle, GL_INFO_LOG_LENGTH, &logLength);
                    char* message = (char*)alloca(logLength);
                    GL.glGetShaderInfoLog(fragmentHandle, logLength, nullptr, message);
                    log_print("[Error]: Failed to compile frag shader (%s)\n%s\n", name, message);
                }
            } else {
                log_print("Failed to create fragment shader\n");
            }
        } else {
            GLint logLength;
            GL.glGetShaderiv(vertexHandle, GL_INFO_LOG_LENGTH, &logLength);
            char* message = (char*)alloca(logLength);
            GL.glGetShaderInfoLog(vertexHandle, logLength, nullptr, message);
            log_print("[Error]: Failed to compile vertex shader (%s)\n%s", name, message);
        }
    } else {
        log_print("Falled to create vertex shader\n");
    }
    return resultHandle;
}
