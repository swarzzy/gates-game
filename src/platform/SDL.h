#pragma once

#include "../Platform.h"
#include "../Common.h"

#define SDL_MAIN_HANDLED
#define HAVE_LIBC
#include <SDL.h>

#include "OpenGL.h"

#define NO_SDL_GLEXT
#include <SDL_opengl.h>
#include <SDL_keycode.h>

struct SDLContext {
    b32 running;

    SDL_Window* window;
    SDL_Surface* surface;
    SDL_GLContext glContext;

    // Internal. Should not be used. Use values from PlatformState.input
    i32 mousePosX;
    i32 mousePosY;

    OpenGL gl;
};

struct OpenGLLoadResult {
    OpenGL* context;
    b32 success;
};

Key SDLKeycodeConvert(i32 sdlKeycode);
MouseButton SDLMouseButtonConvert(u8 button);

OpenGLLoadResult SDLLoadOpenGL(SDLContext* sdlContext);

void SDLInit(SDLContext* context, PlatformState* platform, i32 glMajorVersion, i32 glMinorVersion);

void SDLPollEvents(SDLContext* context, PlatformState* platform);

void SDLSwapBuffers(SDLContext* context);

void SDLSetVsync(VSyncMode mode);
