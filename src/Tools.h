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
    DArray<DeskPosition> pendingWireNodes;
    DeskPosition lastWireNodePos;

    v2 pickPressedMousePos;
    //b32 selectionMode;
    b32 dragStarted;
    b32 rectSelectStarted;
    b32 dragAttempt;
    b32 rectSelectAttempt;
    DArray<Part*> selectedParts;
    DArray<b32> selectedPartsBlockedStates;
    iv2 dragOffset;

    v3 pickPartOverrideColor;
    v3 pickPartOverrideColorBlocked;

    bool toolPickActuallyEnabled;
    iv2 toolPickLastMouseP;
    v2 clickedPartOffset;

    DArray<Wire*> fullRebuildWiresBuffer;
    DArray<Wire*> partialRebuildWiresBuffer;
    DArray<u8> partialRebuildWiresSelectedEndsBuffer;
};

// TODO: Update order
// Something like
// EarlyUpdate() - precompute mouse positions and stuff
// ProcessInput()
// Update()
// Render()

void ToolManagerInit(ToolManager* manager, Desk* desk);

void ToolPartEnable(ToolManager* manager, Desk* desk, PartInitializerFn* initializer);
void ToolPartLeftMouseDown(ToolManager* manager, Desk* desk);
void ToolPartRightMouseDown(ToolManager* manager, Desk* desk);
void ToolPartUpdate(ToolManager* manager, Desk* desk);
void ToolPartRender(ToolManager* manager, Desk* desk);

void ToolWirePinClicked(ToolManager* manager, Desk* desk, Pin* pin);
void ToolWireRightMouseDown(ToolManager* manager, Desk* desk);
void ToolWireLeftMouseDown(ToolManager* manager, Desk* desk);
void ToolWireRender(ToolManager* manager, Desk* desk);
void ToolWireUpdate(ToolManager* manager, Desk* desk);

void ToolPickUnselectSelected(ToolManager* manager);
void ToolPickResetState(ToolManager* manager);
void ToolPickSelectPart(ToolManager* manager, Part* part, bool toggle = false);
void ToolPickOffsetWire(Wire* wire, iv2 offset);
void ToolPickRebuildWires(ToolManager* manager, Desk* desk);
void ToolPickLeftMouseDown(ToolManager* manager, Desk* desk);
void ToolPickLeftMouseUp(ToolManager* manager, Desk* desk);
void ToolPickRightMouseDown(ToolManager* manager, Desk* desk);
void ToolPickUpdate(ToolManager* manager, Desk* desk);
void ToolPickRender(ToolManager* manager, Desk* desk);

void ToolNoneLeftMouseDown(ToolManager* manager, Desk* desk);
void ToolNoneLeftMouseUp(ToolManager* manager, Desk* desk);
void ToolNoneRightMouseDown(ToolManager* manager, Desk* desk);

void ToolManagerDisableAll(ToolManager* manager);
void ToolManagerLeftMouseDown(ToolManager* manager);
void ToolManagerLeftMouseUp(ToolManager* manager);
void ToolManagerRightMouseDown(ToolManager* manager);
void ToolManagerUpdate(ToolManager* manager);
void ToolManagerRender(ToolManager* manager);
