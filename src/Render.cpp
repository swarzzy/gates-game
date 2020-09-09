#include "Render.h"

const char* ColorRectOpaqueShaderVertex = R"(
#version 330 core

layout (location = 0) in vec4 Position;
layout (location = 1) in vec4 Color;

out vec4 VertexColor;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(Position.xyz, 1.0f);
    VertexColor = Color;
})";

const char* ColorRectOpaqueShaderFragment = R"(
#version 330 core

out vec4 FragmentColor;

in vec4 VertexColor;

void main() {
    FragmentColor = vec4(VertexColor.xyz, 1.0f);
})";

const char* LineShaderVertex = R"(
#version 330 core

layout (location = 0) in vec4 Position;
layout (location = 1) in vec4 Color;

out vec4 VertexColor;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(Position.xyz, 1.0f);
    VertexColor = Color;
})";

const char* LineShaderFragment = R"(
#version 330 core

out vec4 FragmentColor;

in vec4 VertexColor;

void main() {
    FragmentColor = vec4(VertexColor.xyz, 1.0f);
})";

struct Vertex {
    v4 position;
    v4 color;
};

struct LineVertex {
    v4 position;
    v4 color;
};

GLuint CompileGLSL(const char* name, const char* vertexSource, const char* fragmentSource);

void RendererInit(Renderer* renderer) {
    GLuint globalVAO;
    glGenVertexArrays(1, &globalVAO);
    glBindVertexArray(globalVAO);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    // TODO(swarzzy): Multisampling
    //glEnable(GL_MULTISAMPLE);

    glLineWidth(3.0f);

    glClearDepth(1.0f);

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

void RendererBeginFrame(Renderer* renderer) {
    glClearColor(renderer->canvas.clearColor.r, renderer->canvas.clearColor.g, renderer->canvas.clearColor.b, renderer->canvas.clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(renderer->rectColorOpaqueShader);
    glUniformMatrix4fv(renderer->mvpLocation, 1, GL_FALSE, renderer->canvas.projection.data);

    glUseProgram(renderer->lineShader);
    glUniformMatrix4fv(renderer->mvpLocationLine, 1, GL_FALSE, renderer->canvas.projection.data);

    const PlatformState* platform = GetPlatform();
    glViewport(0, 0, platform->windowWidth, platform->windowHeight);
}

void RendererFlushRectQueue(Renderer* renderer, RenderQueue* queue) {
    if (queue->rectBufferAt) {
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vertexBuffer);
        // TODO(swarzzy): Is GL_STREAM_DRAW hint ok here?

        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * queue->rectBufferAt * 4, nullptr, GL_STREAM_DRAW);
        Vertex* buffer = (Vertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        assert(buffer);

        usize bufferAt = 0;

        for (usize i = 0; i < queue->rectBufferAt; i++) {
            auto command = queue->rectBuffer + i;

            // TODO(swarzzy): Only this command supported for now
            assert(command->type == RenderCommandType::RectColor);
            assert(command->transparent == false);

            buffer[bufferAt + 0].position = V4(command->rectColor.min, command->rectColor.z, 1.0f);
            buffer[bufferAt + 0].color = command->rectColor.color;

            buffer[bufferAt + 1].position = V4(command->rectColor.max.x, command->rectColor.min.y, command->rectColor.z, 1.0f);
            buffer[bufferAt + 1].color = command->rectColor.color;

            buffer[bufferAt + 2].position = V4(command->rectColor.max, command->rectColor.z, 1.0f);
            buffer[bufferAt + 2].color = command->rectColor.color;

            buffer[bufferAt + 3].position = V4(command->rectColor.min.x, command->rectColor.max.y, command->rectColor.z, 1.0f);
            buffer[bufferAt + 3].color = command->rectColor.color;

            bufferAt += 4;
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(Vertex), 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Vertex), (void*)sizeof(v4));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->indexBuffer);
        glUseProgram(renderer->rectColorOpaqueShader);

        glDrawElements(GL_TRIANGLES, queue->rectBufferAt * 6, GL_UNSIGNED_SHORT, 0);
    }
}

void RendererFlushLineQueue(Renderer* renderer, RenderQueue* queue) {
    if (queue->lineBufferAt) {
        glBindBuffer(GL_ARRAY_BUFFER, renderer->vertexBuffer);
        // TODO(swarzzy): Is GL_STREAM_DRAW hint ok here?
        glBufferData(GL_ARRAY_BUFFER, sizeof(LineVertex) * queue->lineBufferAt * 2, nullptr, GL_STREAM_DRAW);
        Vertex* buffer = (Vertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        assert(buffer);

        usize bufferAt = 0;

        for (usize i = 0; i < queue->lineBufferAt; i++) {
            auto command = queue->lineBuffer + i;

            // TODO(swarzzy): Only this command supported for now
            assert(command->type == RenderCommandType::Line);
            assert(command->transparent == false);

            buffer[bufferAt + 0].position = V4(command->line.begin, 1.0f);
            buffer[bufferAt + 0].color = command->line.color;

            buffer[bufferAt + 1].position = V4(command->line.end, 1.0f);
            buffer[bufferAt + 1].color = command->line.color;

            bufferAt += 2;
        }

        glUnmapBuffer(GL_ARRAY_BUFFER);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(Vertex), 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Vertex), (void*)sizeof(v4));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glUseProgram(renderer->lineShader);

        glDrawArrays(GL_LINES, 0, queue->lineBufferAt * 2);
    }
}

void RendererDraw(Renderer* renderer, RenderQueue* queue) {
    RendererFlushRectQueue(renderer, queue);
    RendererFlushLineQueue(renderer, queue);
}

void RendererEndFrame(Renderer* renderer) {
}

GLuint CompileGLSL(const char* name, const char* vertexSource, const char* fragmentSource) {
    GLuint resultHandle = 0;

    GLuint vertexHandle = glCreateShader(GL_VERTEX_SHADER);
    if (vertexHandle) {
        glShaderSource(vertexHandle, 1, &vertexSource, nullptr);
        glCompileShader(vertexHandle);

        GLint vertexResult = 0;
        glGetShaderiv(vertexHandle, GL_COMPILE_STATUS, &vertexResult);
        if (vertexResult) {
            GLuint fragmentHandle;
            fragmentHandle = glCreateShader(GL_FRAGMENT_SHADER);
            if (fragmentHandle) {
                glShaderSource(fragmentHandle, 1, &fragmentSource, nullptr);
                glCompileShader(fragmentHandle);

                GLint fragmentResult = 0;
                glGetShaderiv(fragmentHandle, GL_COMPILE_STATUS, &fragmentResult);
                if (fragmentResult) {
                    GLint programHandle;
                    programHandle = glCreateProgram();
                    if (programHandle) {
                        glAttachShader(programHandle, vertexHandle);
                        glAttachShader(programHandle, fragmentHandle);
                        glLinkProgram(programHandle);

                        GLint linkResult = 0;
                        glGetProgramiv(programHandle, GL_LINK_STATUS, &linkResult);
                        if (linkResult) {
                            glDeleteShader(vertexHandle);
                            glDeleteShader(fragmentHandle);
                            resultHandle = programHandle;
                        } else {
                            i32 logLength;
                            glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLength);
                            char* message = (char*)PlatformAllocate(logLength, 0, nullptr);
                            glGetProgramInfoLog(programHandle, logLength, 0, message);
                            log_print("[Error]: Failed to link shader program (%s) \n%s\n", name, message);
                            PlatformDeallocate(message, nullptr);
                        }
                    } else {
                        log_print("Failed to create shader program\n");
                    }
                } else {
                    GLint logLength;
                    glGetShaderiv(fragmentHandle, GL_INFO_LOG_LENGTH, &logLength);
                    char* message = (char*)PlatformAllocate(logLength, 0, nullptr);
                    glGetShaderInfoLog(fragmentHandle, logLength, nullptr, message);
                    log_print("[Error]: Failed to compile frag shader (%s)\n%s\n", name, message);
                    PlatformDeallocate(message, nullptr);
                }
            } else {
                log_print("Failed to create fragment shader\n");
            }
        } else {
            GLint logLength;
            glGetShaderiv(vertexHandle, GL_INFO_LOG_LENGTH, &logLength);
            char* message = (char*)PlatformAllocate(logLength, 0, nullptr);
            glGetShaderInfoLog(vertexHandle, logLength, nullptr, message);
            log_print("[Error]: Failed to compile vertex shader (%s)\n%s", name, message);
            PlatformDeallocate(message, nullptr);
        }
    } else {
        log_print("Falled to create vertex shader\n");
    }
    return resultHandle;
}
