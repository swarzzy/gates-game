#pragma once

struct ImGuiContext;

#include "SDL.h"
#include "../../ext/mimalloc-1.6.4/include/mimalloc.h"
#include "../../ext/imgui-1.78/imgui.h"

ImGuiContext* InitImGuiForGL3(mi_heap_t* heap, SDL_Window* window, SDL_GLContext* glContext);
void ImGuiNewFrameForGL3(SDL_Window* window);
void ImGuiEndFrameForGL3(u32 wWindow, u32 hWindow);
void ImGuiProcessEventFromSDL(SDL_Event* event);

void* ImguiAllocWrapper(size_t size, void* heap);
void ImguiFreeWrapper(void* ptr, void*_);
