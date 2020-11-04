#pragma once

enum struct Tool {
    None = 0, Part, Wire, Pick
};

struct ToolManager {
    Tool currentTool;
    // TODO: Subject to change
    Part prefabPart;
    DeskPosition prefabPartPos;
    DeskPosition mouseDeskPos;
    v2 mouseCanvasPos;

    Pin* pendingWireBeginPin;

    Part* pickPart;
    iv2 pickPartOverridePos;
    v3 pickPartOverrideColor;
};

void ToolPartEnable(ToolManager* manager, Desk* desk, PartInitializerFn* initializer);
void ToolPartPrimaryAction(ToolManager* manager, Desk* desk);
void ToolPartUpdate(ToolManager* manager, Desk* desk);
void ToolPartRender(ToolManager* manager, Desk* desk);

void ToolNonePrimaryAction(ToolManager* manager, Desk* desk);
void ToolNoneSecondaryAction(ToolManager* manager, Desk* desk);

void ToolWireRender(ToolManager* manager, Desk* desk);
void ToolWirePrimaryAction(ToolManager* manager, Desk* desk);

void ToolPickEnable(ToolManager* manager, Desk* desk);
void ToolPickUpdate(ToolManager* manager, Desk* desk);
void ToolPickPrimaryAction(ToolManager* manager, Desk* desk);
void ToolPickSecondaryAction(ToolManager* manager, Desk* desk);
void ToolPickRender(ToolManager* manager, Desk* desk);

void ToolManagerDisableAll(ToolManager* manager);
void ToolManagerPrimaryAction(ToolManager* manager);
void ToolManagerSecondaryAction(ToolManager* manager);
void ToolManagerUpdate(ToolManager* manager);
void ToolManagerRender(ToolManager* manager);
