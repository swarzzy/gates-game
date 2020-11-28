#pragma once

enum struct Tool {
    None = 0, Part, Wire, Pick
};

struct ToolManager {
    inline static const f32 PickMinThreshold = 0.2f; // 1mm
    inline static const f32 RectSelectMinThreshold = 0.2f; // 1mm

    Tool currentTool;
    // TODO: Subject to change
    Part prefabPart;
    DeskPosition prefabPartPos;
    DeskPosition mouseDeskPos;
    v2 mouseCanvasPos;

    Pin* pendingWireBeginPin;
    Array<DeskPosition> pendingWireNodes;
    DeskPosition lastWireNodePos;

    v2 pickPressedMousePos;
    //b32 selectionMode;
    b32 dragStarted;
    b32 rectSelectStarted;
    b32 dragAttempt;
    b32 rectSelectAttempt;
    Array<Part*> selectedParts;
    Array<b32> selectedPartsBlockedStates;
    iv2 dragOffset;

    v3 pickPartOverrideColor;
    v3 pickPartOverrideColorBlocked;

    bool toolPickActuallyEnabled;
    iv2 toolPickLastMouseP;
    v2 clickedPartOffset;

    Array<Wire*> fullRebuildWiresBuffer;
    Array<Wire*> partialRebuildWiresBuffer;
    Array<u8> partialRebuildWiresSelectedEndsBuffer;
};

// TODO: Update order
// Something like
// EarlyUpdate() - precompute mouse positions and stuff
// ProcessInput()
// Update()
// Render()

void ToolManagerInit(ToolManager* manager, Desk* desk);

void ToolPartEnable(ToolManager* manager, Desk* desk, PartInitializerFn* initializer);
void ToolPartPrimaryAction(ToolManager* manager, Desk* desk);
void ToolPartUpdate(ToolManager* manager, Desk* desk);
void ToolPartRender(ToolManager* manager, Desk* desk);

void ToolNoneLeftMouseDown(ToolManager* manager, Desk* desk);
void ToolNoneLeftMouseUp(ToolManager* manager, Desk* desk);
void ToolNoneSecondaryAction(ToolManager* manager, Desk* desk);

void ToolWireRender(ToolManager* manager, Desk* desk);
void ToolWireUpdate(ToolManager* manager, Desk* desk);
void ToolWirePrimaryAction(ToolManager* manager, Desk* desk);

void ToolPickEnable(ToolManager* manager, Desk* desk);
void ToolPickUpdate(ToolManager* manager, Desk* desk);
void ToolPickLeftMouseDown(ToolManager* manager, Desk* desk);
void ToolPickLeftMouseUp(ToolManager* manager, Desk* desk);
void ToolPickSecondaryAction(ToolManager* manager, Desk* desk);
void ToolPickRender(ToolManager* manager, Desk* desk);

void ToolManagerDisableAll(ToolManager* manager);
void ToolManagerLeftMouseDown(ToolManager* manager);
void ToolManagerLeftMouseUp(ToolManager* manager);

void ToolManagerSecondaryAction(ToolManager* manager);
void ToolManagerUpdate(ToolManager* manager);
void ToolManagerRender(ToolManager* manager);
