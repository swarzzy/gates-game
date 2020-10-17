#pragma once

struct ImGuiContext;

#include "SDL.h"
#include "../../ext/imgui-1.78/imgui.h"

ImGuiContext* InitImGuiForGL3(PlatformHeap* heap, SDL_Window* window, SDL_GLContext* glContext);
void ImGuiNewFrameForGL3(SDL_Window* window, u32 wWindow, u32 hWindow);
void ImGuiEndFrameForGL3();
void ImGuiProcessEventFromSDL(SDL_Event* event);

void* ImguiAllocWrapper(size_t size, void* heap);
void ImguiFreeWrapper(void* ptr, void*_);
