#include "DebugOverlay.h"
#include "Globals.h"

#define CHECK_IMGUI() if (!ImGuiAvailable()) return

void DrawDebugPerformanceCounters() {
    CHECK_IMGUI();
    const float DISTANCE = 10.0f;
    int corner = 1;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
    ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.35f);
    bool open = true;
    if (ImGui::Begin("Overlay", &open, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
        char fpsBuffer[128];
        auto platform = GetPlatform();
        sprintf_s(fpsBuffer, 128, "fps: %11d\nups: %11d\nsim/s: %11d", platform->framesPerSecond, platform->updatesPerSecond, platform->simStepsPerSecond);
        ImGui::Text("%s", fpsBuffer);
    }
    ImGui::End();
}

static const auto DebugOverlayFlags = 0;//ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
//ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration

void BeginDebugOverlay() {
    CHECK_IMGUI();
    if (Global_ShowDebugOverlay) {
        const float xPos = 10.0f;
        const float yPos = 10.0f;

        ImGuiIO& io = ImGui::GetIO();
        ImVec2 windowPos = ImVec2(xPos, yPos);
        ImVec2 windosPosPivot = ImVec2(0.0f, 0.0f);
        ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windosPosPivot);
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags);
        ImGui::End();
    }
}

bool DebugOverlayBeginCustom() {
    bool result = false;
    if (ImGuiAvailable() && Global_ShowDebugOverlay) {
        result = ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags);
    }
    return result;
}

void DebugOverlayEndCustom() {
    CHECK_IMGUI();
    if (Global_ShowDebugOverlay) {
        ImGui::End();
    }
}

void DebugOverlayPushInternal(const char* string) {
    CHECK_IMGUI();
    if (Global_ShowDebugOverlay) {
        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            ImGui::Text("%s", string);
        }
        ImGui::End();
    }
}

void DebugOverlayPushString(const char* string) {
    CHECK_IMGUI();
    DebugOverlayPushInternal(string);
}

void DebugOverlayPushVar(const char* title, uv3 var) {
    CHECK_IMGUI();
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %lu; y: %lu; z: %lu", title, (unsigned long)var.x, (unsigned long)var.y, (unsigned long)var.z);
    DebugOverlayPushInternal(buffer);
}

void DebugOverlayPushVar(const char* title, iv3 var) {
    CHECK_IMGUI();
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %ld; y: %ld; z: %ld", title, (long)var.x, (long)var.y, (long)var.z);
    DebugOverlayPushInternal(buffer);
}

void DebugOverlayPushVar(const char* title, v2 var) {
    CHECK_IMGUI();
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %.3f; y: %.3f", title, var.x, var.y);
    DebugOverlayPushInternal(buffer);
}

void DebugOverlayPushVar(const char* title, v3 var) {
    CHECK_IMGUI();
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %.3f; y: %.3f; z: %.3f", title, var.x, var.y, var.z);
    DebugOverlayPushInternal(buffer);
}

void DebugOverlayPushVar(const char* title, v4 var) {
    CHECK_IMGUI();
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %.3f; y: %.3f; z: %.3f; w: %.3f", title, var.x, var.y, var.z, var.w);
    DebugOverlayPushInternal(buffer);
}


void DebugOverlayPushVar(const char* title, u32 var) {
    CHECK_IMGUI();
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %lu", title, (unsigned long)var);
    DebugOverlayPushInternal(buffer);
}

void DebugOverlayPushVar(const char* title, i32 var) {
    CHECK_IMGUI();
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %ld", title, (long)var);
    DebugOverlayPushInternal(buffer);
}

void DebugOverlayPushVar(const char* title, f32 var) {
    CHECK_IMGUI();
    char buffer[128];
    sprintf_s(buffer, 128, "%s: x: %.5f", title, var);
    DebugOverlayPushInternal(buffer);
}

void DebugOverlayPushSlider(const char* title, f32* var, f32 min, f32 max) {
    CHECK_IMGUI();
    if (Global_ShowDebugOverlay) {
        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            ImGui::SliderFloat(title, var, min, max);
        }
        ImGui::End();
    }
}

void DebugOverlayPushSlider(const char* title, i32* var, i32 min, i32 max) {
    CHECK_IMGUI();
    if (Global_ShowDebugOverlay) {
        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            ImGui::SliderInt(title, var, min, max);
        }
        ImGui::End();
    }
}

void DebugOverlayPushSlider(const char* title, u32* var, u32 min, u32 max) {
    CHECK_IMGUI();
    if (Global_ShowDebugOverlay) {
        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            i32 v = (i32)*var;
                ImGui::SliderInt(title, &v, (i32)min, (i32)max);
            *var = v;
        }
        ImGui::End();
    }
}

void DebugOverlayPushSlider(const char* title, v3* var, f32 min, f32 max) {
    CHECK_IMGUI();
    if (Global_ShowDebugOverlay) {

        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            ImGui::SliderFloat3(title, var->data, min, max);
        }
        ImGui::End();
    }
}

void DebugOverlayPushSlider(const char* title, v4* var, f32 min, f32 max) {
    CHECK_IMGUI();
    if (Global_ShowDebugOverlay) {
        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            ImGui::SliderFloat4(title, var->data, min, max);
        }
        ImGui::End();
    }
}

void DebugOverlayPushToggle(const char* title, bool* var) {
    CHECK_IMGUI();
    if (Global_ShowDebugOverlay) {
        if (ImGui::Begin("Debug overlay", nullptr, DebugOverlayFlags)) {
            ImGui::Separator();
            ImGui::Checkbox(title, var);
        }
        ImGui::End();
    }
}
