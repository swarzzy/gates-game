#include "ImGui.h"

#include "../../ext/imgui-1.78/imgui.h"
#include "../../ext/imgui-1.78/imgui_impl_gates_sdl.h"
#include "../../ext/imgui-1.78/imgui_impl_gates_opengl3.h"

#define gl_function(func) GlobalContext.sdl.gl.functions.fn. func
#define glViewport gl_function(glViewport)
#define glClearColor gl_function(glClearColor)
#define glClear gl_function(glClear)

void* ImguiAllocWrapper(size_t size, void* heap) { return mi_heap_malloc((mi_heap_t*)heap, size); }
void ImguiFreeWrapper(void* ptr, void*_) { mi_free(ptr); }

ImGuiContext* InitImGuiForGL3(mi_heap_t* heap, SDL_Window* window, SDL_GLContext* glContext) {
    ImGuiContext* result = nullptr;
    IMGUI_CHECKVERSION();
    ImGui::SetAllocatorFunctions(ImguiAllocWrapper, ImguiFreeWrapper, heap);
    auto imguiContext = ImGui::CreateContext();
    if (imguiContext) {
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
        ImGui_ImplSDL2_InitForOpenGL(window, glContext);
        auto imguiGlInitResult = ImGui_ImplOpenGL3_Init("#version 130");
        if (imguiGlInitResult) {
            io.IniFilename = nullptr;
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            result = imguiContext;
        }
    }
    return result;
}

void ImGuiNewFrameForGL3(SDL_Window* window, u32 wWindow, u32 hWindow) {
    // TODO: Remove this temporary gl calls
    glViewport(0, 0, (int)wWindow, (int)hWindow);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
}

void ImGuiEndFrameForGL3() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiProcessEventFromSDL(SDL_Event* event) {
    ImGui_ImplSDL2_ProcessEvent(event);
}

#define glGetIntegerv gl_function(glGetIntegerv)
#define glBindSampler gl_function(glBindSampler)
#define glIsEnabled gl_function(glIsEnabled)
#define glScissor gl_function(glScissor)
#define glDrawElementsBaseVertex gl_function(glDrawElementsBaseVertex)
#define glDeleteVertexArrays gl_function(glDeleteVertexArrays)
#define glBindSampler gl_function(glBindSampler)
#define glBlendEquationSeparate gl_function(glBlendEquationSeparate)
#define glBlendFuncSeparate gl_function(glBlendFuncSeparate)
#define glPixelStorei gl_function(glPixelStorei)
#define glGetAttribLocation gl_function(glGetAttribLocation)
#define glDeleteBuffers gl_function(glDeleteBuffers)
#define glDetachShader gl_function(glDetachShader)
#define glDeleteProgram gl_function(glDeleteProgram)
#define glEnable gl_function(glEnable)
#define glBlendEquation gl_function(glBlendEquation)
#define glBlendFunc gl_function(glBlendFunc)
#define glDisable gl_function(glDisable)
#define glPolygonMode gl_function(glPolygonMode)
#define glUseProgram gl_function(glUseProgram)
#define glUniform1i gl_function(glUniform1i)
#define glUniformMatrix4fv gl_function(glUniformMatrix4fv)
#define glBindVertexArray gl_function(glBindVertexArray)
#define glBindBuffer gl_function(glBindBuffer)
#define glEnableVertexAttribArray gl_function(glEnableVertexAttribArray)
#define glVertexAttribPointer gl_function(glVertexAttribPointer)
#define glActiveTexture gl_function(glActiveTexture)
#define glGenVertexArrays gl_function(glGenVertexArrays)
#define glBufferData gl_function(glBufferData)
#define glBindTexture gl_function(glBindTexture)
#define glTexParameteri gl_function(glTexParameteri)
#define glTexImage2D gl_function(glTexImage2D)
#define glGenTextures gl_function(glGenTextures)
#define glDeleteTextures gl_function(glDeleteTextures)
#define glGetShaderiv gl_function(glGetShaderiv)
#define glGetShaderInfoLog gl_function(glGetShaderInfoLog)
#define glGetProgramiv gl_function(glGetProgramiv)
#define glCreateShader gl_function(glCreateShader)
#define glShaderSource gl_function(glShaderSource)
#define glCompileShader gl_function(glCompileShader)
#define glCreateProgram gl_function(glCreateProgram)
#define glAttachShader gl_function(glAttachShader)
#define glLinkProgram gl_function(glLinkProgram)
#define glGetUniformLocation gl_function(glGetUniformLocation)
#define glGetProgramInfoLog gl_function(glGetProgramInfoLog)
#define glGenBuffers gl_function(glGenBuffers)
#define glDeleteShader gl_function(glDeleteShader)
#define glDrawElements gl_function(glDrawElements)

#include "../../ext/imgui-1.78/imconfig.h"
#include "../../ext/imgui-1.78/imgui.cpp"
#include "../../ext/imgui-1.78/imgui_draw.cpp"
#include "../../ext/imgui-1.78/imgui_widgets.cpp"
#include "../../ext/imgui-1.78/imgui_demo.cpp"
#include "../../ext/imgui-1.78/imgui_impl_gates_opengl3.cpp"
#include "../../ext/imgui-1.78/imgui_impl_gates_sdl.cpp"
