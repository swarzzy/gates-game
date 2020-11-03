#pragma once

enum struct Tool {
    None = 0, Part, Wire
};

struct ToolManager {
    Tool currentTool;
    // TODO: Subject to change
    Part prefabPart;
    DeskPosition prefabPartPos;
    DeskPosition mouseDeskPos;
    v2 mouseCanvasPos;

    Pin* pendingWireBeginPin;
};

void ToolPartEnable(ToolManager* manager, Desk* desk, PartInitializerFn* initializer);
void ToolPartPrimaryAction(ToolManager* manager, Desk* desk);
void ToolPartUpdate(ToolManager* manager, Desk* desk);
void ToolPartRender(ToolManager* manager, Desk* desk);

void ToolNonePrimaryAction(ToolManager* manager, Desk* desk);
void ToolNoneSecondaryAction(ToolManager* manager, Desk* desk);

void ToolWireRender(ToolManager* manager, Desk* desk);
void ToolWirePrimaryAction(ToolManager* manager, Desk* desk);

void ToolManagerDisableAll(ToolManager* manager);
void ToolManagerPrimaryAction(ToolManager* manager);
void ToolManagerSecondaryAction(ToolManager* manager);
void ToolManagerUpdate(ToolManager* manager);
void ToolManagerRender(ToolManager* manager);
