#include "Tools.h"

void ToolPartEnable(ToolManager* manager, Desk* desk, PartInitializerFn* initializer) {
    ToolManagerDisableAll(manager);
    manager->currentTool = Tool::Part;
    initializer(desk, &manager->prefabPart);
}

void ToolPartUse(ToolManager* manager, Desk* desk) {
    Part* clone = (Part*)desk->deskAllocator.Alloc(sizeof(Part), false);
    InitPart(desk->partInfo, desk, clone, manager->prefabPartPos.cell, manager->prefabPart.type);
    AddPart(desk, clone);
}

void ToolPartUpdate(ToolManager* manager, Desk* desk) {
    manager->prefabPartPos = DeskPositionOffset(manager->mouseDeskPos, -V2(manager->prefabPart.dim) * 0.5f * DeskCellSize);
}

void ToolPartRender(ToolManager* manager, Desk* desk) {
    DrawPart(desk, desk->canvas, &manager->prefabPart, manager->prefabPartPos, 0.5f);
}

void ToolWirePinClicked(ToolManager* manager, Desk* desk, Pin* pin) {
    if (manager->currentTool != Tool::Wire) {
        manager->pendingWireBeginPin = pin;
        assert(manager->currentTool == Tool::None);
        manager->currentTool = Tool::Wire;
    } else {
        if (pin != manager->pendingWireBeginPin) {
            TryWirePins(desk, manager->pendingWireBeginPin, pin);
            assert(manager->currentTool == Tool::Wire);
            manager->currentTool = Tool::None;
        }
    }
}

void ToolWireUse(ToolManager* manager, Desk* desk) {
    DeskCell* mouseCell = GetDeskCell(desk, manager->mouseDeskPos.cell, false);
    if (mouseCell) {
        if (mouseCell->value == CellValue::Pin) {
            assert(mouseCell->pin);
            ToolWirePinClicked(manager, desk, mouseCell->pin);
        }
    }
}

void ToolNonePartClicked(ToolManager* manager, Desk* desk, Part* part) {
    switch (part->type) {
    case PartType::Source: {
        part->active = !part->active;
    } break;
    default: {} break;
    }
}

void ToolNoneUse(ToolManager* manager, Desk* desk) {
    DeskCell* mouseCell = GetDeskCell(desk, manager->mouseDeskPos.cell, false);
    if (mouseCell) {
        if (mouseCell->value != CellValue::Empty) {
            switch (mouseCell->value) {
            case CellValue::Part: {
                ToolNonePartClicked(manager, desk, mouseCell->part);
            } break;
            case CellValue::Pin: {
                assert(mouseCell->pin);
                ToolWirePinClicked(manager, desk, mouseCell->pin);
            } break;
            default: {} break;
            }
        }
    }
}

void ToolWireRender(ToolManager* manager, Desk* desk) {
    DrawListBeginBatch(&desk->canvas->drawList, TextureMode::Color);
    DeskPosition p1 = ComputePinPosition(manager->pendingWireBeginPin);
    v2 lineBeg = DeskPositionRelative(desk->origin, p1);
    v2 lineEnd = manager->mouseCanvasPos;
    f32 thickness = 0.1f;
    DrawSimpleLineBatch(&desk->canvas->drawList, lineBeg, lineEnd, 0.0f, thickness, V4(0.5f, 0.2f, 0.0f, 1.0f));
    DrawListEndBatch(&desk->canvas->drawList);
}

void ToolManagerDisableAll(ToolManager* manager) {
    manager->currentTool = Tool::None;
}

void ToolManagerUse(ToolManager* manager) {
    auto desk = GetDesk();
    switch (manager->currentTool) {
    case Tool::Part: { ToolPartUse(manager, desk); } break;
    case Tool::Wire: { ToolWireUse(manager, desk); } break;
    case Tool::None: { ToolNoneUse(manager, desk); } break;
    default: {} break;
    }
}

void ToolManagerUpdate(ToolManager* manager) {
    auto input = GetInput();
    auto desk = GetDesk();
    v2 mouseScreenPos = V2(input->mouseX, input->mouseY);
    v2 mouseCanvasPos = CanvasProjectScreenPos(desk->canvas, mouseScreenPos);
    DeskPosition mouseDeskPos = DeskPositionOffset(desk->origin, mouseCanvasPos);
    manager->mouseDeskPos = mouseDeskPos;
    manager->mouseCanvasPos = mouseCanvasPos;

    switch (manager->currentTool) {
    case Tool::Part: { ToolPartUpdate(manager, desk); } break;
        //case Tool::None: { ToolNoneUpdate(manager, desk); } break;
    default: {} break;
    }
}

void ToolManagerRender(ToolManager* manager) {
    auto desk = GetDesk();
    switch (manager->currentTool) {
    case Tool::Part: { ToolPartRender(manager, desk); } break;
    case Tool::Wire: { ToolWireRender(manager, desk); } break;
    default: {} break;
    }
}
