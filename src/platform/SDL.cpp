#include "SDL.h"

#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB 0x8242
#define GL_DEBUG_TYPE_OTHER_ARB 0x8251
#define GL_DEBUG_SEVERITY_NOTIFICATION_ARB 0x826B
#define GL_DEBUG_SEVERITY_LOW_ARB 0x9148
typedef void (APIENTRY  *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
typedef void (APIENTRYP PFNGLDEBUGMESSAGECALLBACKPROC) (GLDEBUGPROC callback, const void *userParam);
typedef void (APIENTRYP PFNGLDEBUGMESSAGECONTROLPROC) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
void OpenglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam);

OpenGLLoadResult SDLLoadOpenGL() {
    OpenGL* context = (OpenGL*)malloc(sizeof(OpenGL));
    if (!context) {
        panic("Failed to allocate memory for OpenGL function table");
    }

    log_print("[OpenGL] Loading functions...\n");
    log_print("[OpenGL] Functions defined: %d\n", (int)OpenGL::FunctionCount);

    b32 success = true;
    for (u32 i = 0; i < OpenGL::FunctionCount; i++) {

        // NOTE(swarzzy): SDL_GL_GetProcAddress by itself tries to load a functions
        // using wglGetProcAddress or GetProcAddress if the first fails

        context->functions.raw[i] = SDL_GL_GetProcAddress(OpenGL::FunctionNames[i]);
        if (!context->functions.raw[i]) {
            log_print("[OpenGL]: Failed to load function: %s\n", OpenGL::FunctionNames[i]);
            success = false;
        }
    }

    if (success) {
        log_print("[OpenGL] Done\n");
    } else {
        log_print("[OpenGL] Failed to load some of OpenGL functions\n");
    }

    if (SDL_GL_ExtensionSupported("GL_ARB_debug_output")) {
        PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKPROC)SDL_GL_GetProcAddress("glDebugMessageCallbackARB");
        PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControlARB = (PFNGLDEBUGMESSAGECONTROLPROC)SDL_GL_GetProcAddress("glDebugMessageControlARB");
        if (glDebugMessageCallbackARB && glDebugMessageControlARB) {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
            glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION_ARB, 0, 0, GL_FALSE);
            glDebugMessageControlARB(GL_DONT_CARE, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_LOW_ARB, 0, 0, GL_FALSE);
            glDebugMessageCallbackARB(OpenglDebugCallback, 0);
        }
    }

    return {context, success};
}

void SDLGatherMouseMovement(SDLContext* context, PlatformState* platform) {
    if (platform->input.activeApp) {
        int mousePositionX;
        int mousePositionY;

        // TODO(swarzzy): This function in not what I actually need. It gets mouse
        // coordinates from event messgaes, but I want to use immediate OS getter
        // (i.e. GetCursorPos (SDL version: SDL_GetGlobalMouseState)). But GetCursorPos
        // returns screen-relative coordinates and i need window-relative.
        // So for now just go with SDL_GetMouseState.
        SDL_GetMouseState(&mousePositionX, &mousePositionY);

        if ((mousePositionX != context->mousePosX) || (mousePositionY != context->mousePosY)) {
            i32 offsetX = mousePositionX - context->mousePosX;
            i32 offsetY = -(mousePositionY - context->mousePosY);

            platform->input.mouseFrameOffsetX = offsetX / (f32)platform->windowWidth;
            platform->input.mouseFrameOffsetY = offsetY / (f32)platform->windowHeight;

            context->mousePosX = mousePositionX;
            context->mousePosY = mousePositionY;
            platform->input.mouseX = (f32)mousePositionX;
            platform->input.mouseY = (f32)mousePositionY;
            platform->input.mouseX = mousePositionX / (f32)platform->windowWidth;
            platform->input.mouseY = (platform->windowHeight - mousePositionY) / (f32)platform->windowHeight;
        }
    }
}

void SDLPollEvents(SDLContext* context, PlatformState* platform) {

    SDLGatherMouseMovement(context, platform);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {

        case SDL_WINDOWEVENT: {
            switch (event.window.event) {

            case SDL_WINDOWEVENT_SIZE_CHANGED: {
                platform->windowWidth = event.window.data1;
                platform->windowHeight = event.window.data2;
            } break;

            case SDL_WINDOWEVENT_ENTER: {
                platform->input.mouseInWindow = true;
            } break;

            case SDL_WINDOWEVENT_LEAVE: {
                platform->input.mouseInWindow = false;
            } break;

            case SDL_WINDOWEVENT_FOCUS_GAINED: {
                // TODO(swarzzy): This probably does not correspond to win32 api
                // implementation of platform layer. In win32 this flag is toggled
                // on WM_ACTIVEAPP event, but it seems like SDL throws this event
                // on WM_ACTIVE. SDL doc says that this event is for keyboard focus
                // [https://wiki.libsdl.org/SDL_WindowEventID]
                platform->input.activeApp = true;
            } break;

            case SDL_WINDOWEVENT_FOCUS_LOST: {
                platform->input.activeApp = false;
            } break;

            default: {} break;
            }
        } break;

        case SDL_MOUSEBUTTONDOWN: {
            MouseButton button = SDLMouseButtonConvert(event.button.button);
            platform->input.mouseButtons[(u32)button].wasPressed =  platform->input.mouseButtons[(u32)button].pressedNow;
            platform->input.mouseButtons[(u32)button].pressedNow = true;
            //printf("Mouse button pressed: %s\n", ToString(button));
        } break;

        case SDL_MOUSEBUTTONUP: {
            MouseButton button = SDLMouseButtonConvert(event.button.button);
            platform->input.mouseButtons[(u32)button].wasPressed =  platform->input.mouseButtons[(u32)button].pressedNow;
            platform->input.mouseButtons[(u32)button].pressedNow = false;
            //printf("Mouse button released: %s\n", ToString(button));
        } break;

        case SDL_MOUSEWHEEL: {
            // TODO(swarzzy): Check for SDL_MOUSEWHEEL_NORMAL and SDL_MOUSEWHEEL_FLIPPED
            // TODO(swarzzy): x!
            i32 steps = event.wheel.y;
            platform->input.scrollOffset = steps;
            platform->input.scrollFrameOffset = steps;
        } break;

        case SDL_KEYDOWN: {
            // TODO(swarzzy): Is SDL can throw KEYDOWN event which has state SDL_RELEASED???
            // We need test this
            assert(event.key.state == SDL_PRESSED);

            // TODO(swarzzy): What it the real difference between scancodes and keycodes
            // and which of them we sould use

            // TODO(swarzzy): Repeat count

            u32 keyCode = (u32)SDLKeycodeConvert(event.key.keysym.sym);
            u16 repeatCount =  event.key.repeat;
            platform->input.keys[keyCode].wasPressed = platform->input.keys[keyCode].pressedNow;
            platform->input.keys[keyCode].pressedNow = true;
        } break;

        case SDL_KEYUP: {
            assert(event.key.state == SDL_RELEASED);

            u32 keyCode = (u32)SDLKeycodeConvert(event.key.keysym.sym);
            u16 repeatCount =  event.key.repeat;
            platform->input.keys[keyCode].wasPressed = platform->input.keys[keyCode].pressedNow;
            platform->input.keys[keyCode].pressedNow = false;
        } break;

        case SDL_QUIT: { context->running = false; } break;
        default: {} break;
        }
    }
}

void SDLInit(SDLContext* context, const PlatformState* platform, i32 glMajorVersion, i32 glMinorVersion) {
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        panic("[SDL] Initialization failed: %s", SDL_GetError());
    }

    context->running = true;

#define sdl_gl_attrib_check(x)  if ((x) != 0) panic("[SDL] Failed to init OpneGL with attribute %s. %s", #x, SDL_GetError());

    // TODO(swarzzy): These attributes might be to strict for simple ping pong game
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajorVersion));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinorVersion));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1));
    sdl_gl_attrib_check(SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG));

#undef sdl_gl_attrib_check

    auto windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
    context->window = SDL_CreateWindow("Ping pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, platform->windowWidth, platform->windowHeight, windowFlags);
    if (!context->window) {
        panic("[SDL] Failed to create window: %s", SDL_GetError());
    }

    context->glContext = SDL_GL_CreateContext(context->window);
    if (!context->glContext) {
        panic("[SDL] Failed to initialize OpenGL context-> %s", SDL_GetError());
    }

    if (SDL_GL_SetSwapInterval(1) != 0) {
        log_print("[SDL] Warning! V-sync is not supported\n");
    }
}

void SDLSwapBuffers(SDLContext* context) {
    SDL_GL_SwapWindow(context->window);
}

Key SDLKeycodeConvert(i32 sdlKeycode) {
    // TODO(swarzzy): Test this
   switch (sdlKeycode) {
    case SDLK_BACKSPACE:    return Key::Backspace;
    case SDLK_TAB:          return Key::Tab;
    case SDLK_CLEAR:        return Key::Clear;
    case SDLK_RETURN:       return Key::Enter;
    case SDLK_LSHIFT:       return Key::LeftShift;
    case SDLK_LCTRL:        return Key::LeftCtrl;
    case SDLK_LALT:         return Key::LeftAlt;
    case SDLK_RSHIFT:       return Key::RightShift;
    case SDLK_RCTRL:        return Key::RightCtrl;
    case SDLK_RALT:         return Key::RightAlt;
    case SDLK_PAUSE:        return Key::Pause;
    case SDLK_CAPSLOCK:     return Key::CapsLock;
    case SDLK_ESCAPE:       return Key::Escape;
    case SDLK_SPACE:        return Key::Space;
    case SDLK_PAGEUP:       return Key::PageUp;
    case SDLK_PAGEDOWN:     return Key::PageDown;
    case SDLK_END:          return Key::End;
    case SDLK_HOME:         return Key::Home;
    case SDLK_LEFT:         return Key::Left;
    case SDLK_UP:           return Key::Up;
    case SDLK_RIGHT:        return Key::Right;
    case SDLK_DOWN:         return Key::Down;
    case SDLK_PRINTSCREEN:  return Key::PrintScreen;
    case SDLK_INSERT:       return Key::Insert;
    case SDLK_DELETE:       return Key::Delete;
    case SDLK_0:            return Key::_0;
    case SDLK_1:            return Key::_1;
    case SDLK_2:            return Key::_2;
    case SDLK_3:            return Key::_3;
    case SDLK_4:            return Key::_4;
    case SDLK_5:            return Key::_5;
    case SDLK_6:            return Key::_6;
    case SDLK_7:            return Key::_7;
    case SDLK_8:            return Key::_8;
    case SDLK_9:            return Key::_9;
    case SDLK_a:            return Key::A;
    case SDLK_b:            return Key::B;
    case SDLK_c:            return Key::C;
    case SDLK_d:            return Key::D;
    case SDLK_e:            return Key::E;
    case SDLK_f:            return Key::F;
    case SDLK_g:            return Key::G;
    case SDLK_h:            return Key::H;
    case SDLK_i:            return Key::I;
    case SDLK_j:            return Key::J;
    case SDLK_k:            return Key::K;
    case SDLK_l:            return Key::L;
    case SDLK_m:            return Key::M;
    case SDLK_n:            return Key::N;
    case SDLK_o:            return Key::O;
    case SDLK_p:            return Key::P;
    case SDLK_q:            return Key::Q;
    case SDLK_r:            return Key::R;
    case SDLK_s:            return Key::S;
    case SDLK_t:            return Key::T;
    case SDLK_u:            return Key::U;
    case SDLK_v:            return Key::V;
    case SDLK_w:            return Key::W;
    case SDLK_x:            return Key::X;
    case SDLK_y:            return Key::Y;
    case SDLK_z:            return Key::Z;
    case SDLK_LGUI:         return Key::LeftSuper;
    case SDLK_RGUI:         return Key::RightSuper;
    case SDLK_KP_0:         return Key::Num0;
    case SDLK_KP_1:         return Key::Num1;
    case SDLK_KP_2:         return Key::Num2;
    case SDLK_KP_3:         return Key::Num3;
    case SDLK_KP_4:         return Key::Num4;
    case SDLK_KP_5:         return Key::Num5;
    case SDLK_KP_6:         return Key::Num6;
    case SDLK_KP_7:         return Key::Num7;
    case SDLK_KP_8:         return Key::Num8;
    case SDLK_KP_9:         return Key::Num9;
    case SDLK_KP_MULTIPLY:  return Key::NumMultiply;
    case SDLK_KP_PLUS:      return Key::NumAdd;
    case SDLK_KP_MINUS:     return Key::NumSubtract;
    case SDLK_KP_DIVIDE:    return Key::NumDivide;
    case SDLK_KP_ENTER:     return Key::NumEnter;
    // They are the same
    case SDLK_KP_DECIMAL:   return Key::NumDecimal;
    case SDLK_KP_PERIOD:    return Key::NumDecimal;

    case SDLK_F1:           return Key::F1;
    case SDLK_F2:           return Key::F2;
    case SDLK_F3:           return Key::F3;
    case SDLK_F4:           return Key::F4;
    case SDLK_F5:           return Key::F5;
    case SDLK_F6:           return Key::F6;
    case SDLK_F7:           return Key::F7;
    case SDLK_F8:           return Key::F8;
    case SDLK_F9:           return Key::F9;
    case SDLK_F10:          return Key::F10;
    case SDLK_F11:          return Key::F11;
    case SDLK_F12:          return Key::F12;
    case SDLK_F13:          return Key::F13;
    case SDLK_F14:          return Key::F14;
    case SDLK_F15:          return Key::F15;
    case SDLK_F16:          return Key::F16;
    case SDLK_F17:          return Key::F17;
    case SDLK_F18:          return Key::F18;
    case SDLK_F19:          return Key::F19;
    case SDLK_F20:          return Key::F20;
    case SDLK_F21:          return Key::F21;
    case SDLK_F22:          return Key::F22;
    case SDLK_F23:          return Key::F23;
    case SDLK_F24:          return Key::F24;
    case SDLK_NUMLOCKCLEAR: return Key::NumLock;
    case SDLK_SCROLLLOCK:   return Key::ScrollLock;
    case SDLK_APPLICATION:  return Key::Menu;
    case SDLK_SEMICOLON:    return Key::Semicolon;
    case SDLK_EQUALS:       return Key::Equal;
    case SDLK_COMMA:        return Key::Comma;
    case SDLK_MINUS:        return Key::Minus;
    case SDLK_PERIOD:       return Key::Period;
    case SDLK_SLASH:        return Key::Slash;
    case SDLK_BACKQUOTE:    return Key::Tilde;
    case SDLK_LEFTBRACKET:  return Key::LeftBracket;
    case SDLK_BACKSLASH:    return Key::BackSlash;
    case SDLK_RIGHTBRACKET: return Key::RightBracket;
    case SDLK_QUOTE:        return Key::Apostrophe;
    default:                return Key::Invalid;
    }
}

MouseButton SDLMouseButtonConvert(u8 button) {
    switch (button) {
    case SDL_BUTTON_LEFT:   return MouseButton::Left;
    case SDL_BUTTON_RIGHT:  return MouseButton::Right;
    case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
    case SDL_BUTTON_X1:     return MouseButton::XButton1;
    case SDL_BUTTON_X2:     return MouseButton::XButton2;
    invalid_default();
    }
    // Dummy return for the compiler
    return MouseButton::Left;
}

void OpenglDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const GLvoid* userParam) {
    const char* sourceStr;
    const char* typeStr;
    const char* severityStr;

    switch (source) {
    case GL_DEBUG_SOURCE_API: { sourceStr = "API"; } break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: { sourceStr = "window system"; } break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: { sourceStr = "shader compiler"; } break;
    case GL_DEBUG_SOURCE_THIRD_PARTY: { sourceStr = "third party"; } break;
    case GL_DEBUG_SOURCE_APPLICATION: { sourceStr = "application"; } break;
    case GL_DEBUG_SOURCE_OTHER: { sourceStr = "other"; } break;
    invalid_default();
    }

    switch (type) {
    case GL_DEBUG_TYPE_ERROR: { typeStr = "error"; } break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: { typeStr = "deprecated behavior"; } break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: { typeStr = "undefined behavior"; } break;
    case GL_DEBUG_TYPE_PORTABILITY: { typeStr = "portability problem"; } break;
    case GL_DEBUG_TYPE_PERFORMANCE: { typeStr = "performance problem"; } break;
    case GL_DEBUG_TYPE_OTHER: { typeStr = "other"; } break;
    invalid_default();
    }

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: { severityStr = "high"; } break;
    case GL_DEBUG_SEVERITY_MEDIUM: { severityStr = "medium"; } break;
    case GL_DEBUG_SEVERITY_LOW: { severityStr = "low"; } break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: { severityStr = "notification"; } break;
    default: { severityStr = "unknown"; } break;
    }
    log_print("[OpenGL] Debug message (source: %s, type: %s, severity: %s): %s\n", sourceStr, typeStr, severityStr, message);
    assert(false);
}
