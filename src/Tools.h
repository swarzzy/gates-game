#pragma once

enum struct Tool {
    None = 0, Part, Wire
};

struct ToolManager {
    Tool currentTool;
    Part* prefabPart; // Read-only pointer
    DeskPosition prefabPartPos;
    DeskPosition mouseDeskPos;
    v2 mouseCanvasPos;

    Pin* pendingWireBeginPin;
};

void ToolPartEnable(ToolManager* manager, Part* prefabPart);
void ToolPartUse(ToolManager* manager, Desk* desk);
void ToolPartUpdate(ToolManager* manager, Desk* desk);
void ToolPartRender(ToolManager* manager, Desk* desk);

void ToolNoneUse(ToolManager* manager, Desk* desk);

void ToolWireRender(ToolManager* manager, Desk* desk);

void ToolManagerDisableAll(ToolManager* manager);
void ToolManagerUse(ToolManager* manager);
void ToolManagerUpdate(ToolManager* manager);
void ToolManagerRender(ToolManager* manager);
