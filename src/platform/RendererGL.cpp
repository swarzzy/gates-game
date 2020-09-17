#include "RendererGL.h"

#include "../Draw.h"

static Renderer* _GlobalRenderer;
inline Renderer* GetRenderer() { return _GlobalRenderer; }

const char* StandardShaderVert = R"(
#version 330 core

layout (location = 0) in vec3 Position;
layout (location = 1) in vec4 Color;

out vec4 VertexColor;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(Position.xyz, 1.0f);
    VertexColor = Color;
})";

const char* StandardShaderFrag = R"(
#version 330 core

out vec4 FragmentColor;

in vec4 VertexColor;

void main() {
    FragmentColor = vec4(VertexColor.xyz, 1.0f);
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

    //GL.glLineWidth(3.0f);

    GL.glClearDepth(1.0f);

    GL.glGenBuffers(1, &renderer->vertexBuffer);
    assert(renderer->vertexBuffer);

    GL.glGenBuffers(1, &renderer->indexBuffer);
    assert(renderer->indexBuffer);

    renderer->standardShader.handle = CompileGLSL("StandardShader", StandardShaderVert, StandardShaderFrag);
    assert(renderer->standardShader.handle);

    BindShaderUniform(&renderer->standardShader, MVP);
}

void RendererBeginFrame(const m4x4* projection, v4 clearColor) {

}

void RenderDrawList(DrawList* list) {
    auto renderer = GetRenderer();

    if (list->vertexBuffer.Count() && list->indexBuffer.Count()) {
        GL.glBindBuffer(GL_ARRAY_BUFFER, renderer->vertexBuffer);
        GL.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->indexBuffer);

        GL.glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * list->vertexBuffer.Count(), list->vertexBuffer.Begin(), GL_STREAM_DRAW);
        GL.glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * list->indexBuffer.Count(), list->indexBuffer.Begin(), GL_STREAM_DRAW);

        GL.glEnableVertexAttribArray(0);
        GL.glEnableVertexAttribArray(1);

        GL.glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), 0);
        GL.glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Vertex), (void*)sizeof(v3));

        GL.glUseProgram(renderer->standardShader.handle);

        auto mvp = OrthoGLRH(0.0f, 16.0f, 0.0f, 9.0f, -1.0f, 1.0f);
        GL.glUniformMatrix4fv(renderer->standardShader.MVP, 1, false, mvp.data);

        for (auto& cmd : list->commandBuffer) {
            GL.glDrawElements(GL_TRIANGLES, cmd.indexCount, GL_UNSIGNED_INT, (void*)(u64)(cmd.indexBufferOffset * sizeof(u32)));
        }
    }
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
