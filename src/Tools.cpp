#include "Tools.h"

void ToolPartEnable(ToolManager* manager, Desk* desk, PartInitializerFn* initializer) {
    ToolManagerDisableAll(manager);
    manager->currentTool = Tool::Part;
    initializer(desk, &manager->prefabPart);
}

void ToolPartPrimaryAction(ToolManager* manager, Desk* desk) {
    CreatePart(desk, desk->partInfo, manager->prefabPartPos.cell, manager->prefabPart.type);
}

void ToolPartSecondaryAction(ToolManager* manager, Desk* desk) {
    ToolManagerDisableAll(manager);
}

void ToolPartUpdate(ToolManager* manager, Desk* desk) {
    manager->prefabPartPos = DeskPositionOffset(manager->mouseDeskPos, -V2(manager->prefabPart.dim) * 0.5f * DeskCellSize);
}

void ToolPartRender(ToolManager* manager, Desk* desk) {
    DrawPart(desk, desk->canvas, &manager->prefabPart, manager->prefabPartPos, {}, 0.0f, 0.5f);
}

void ToolWirePinClicked(ToolManager* manager, Desk* desk, Pin* pin) {
    if (manager->currentTool != Tool::Wire) {
        manager->pendingWireBeginPin = pin;
        assert(manager->currentTool == Tool::None);
        manager->currentTool = Tool::Wire;
    } else {
        if (pin != manager->pendingWireBeginPin) {
            Pin* input = nullptr;
            Pin* output = nullptr;

            switch (pin->type) {
            case PinType::Input: { input = pin; } break;
            case PinType::Output: { output = pin; } break;
            invalid_default();
            }

            switch (manager->pendingWireBeginPin->type) {
            case PinType::Input: { input = manager->pendingWireBeginPin; } break;
            case PinType::Output: { output = manager->pendingWireBeginPin; } break;
                invalid_default();
            }

            if (input && output) {
                TryWirePins(desk, input, output);
            }

            assert(manager->currentTool == Tool::Wire);
            manager->currentTool = Tool::None;
        }
    }
}

void ToolWireSecondaryAction(ToolManager* manager, Desk* desk) {
    ToolManagerDisableAll(manager);
}

void ToolWirePrimaryAction(ToolManager* manager, Desk* desk) {
    DeskCell* mouseCell = GetDeskCell(desk, manager->mouseDeskPos.cell, false);
    if (mouseCell) {
        if (mouseCell->value == CellValue::Pin) {
            assert(mouseCell->pin);
            ToolWirePinClicked(manager, desk, mouseCell->pin);
        }
    }
}

void ToolPickEnable(ToolManager* manager, Desk* desk) {
    manager->pickPart = nullptr;
    manager->pickPartOverridePos = manager->mouseDeskPos.cell;
    manager->pickPartOverrideColor = V3(0.7f, 0.2f, 0.2f);
    DeskCell* mouseCell = GetDeskCell(desk, manager->mouseDeskPos.cell, false);
    if (mouseCell->value == CellValue::Part) {
        manager->pickPart = mouseCell->part;
    } else {
        manager->currentTool = Tool::None;
    }
}

void ToolPickUpdate(ToolManager* manager, Desk* desk) {
    iv2 p = DeskPositionOffset(manager->mouseDeskPos, -V2(manager->pickPart->dim) * 0.5f * DeskCellSize).cell;
    if (manager->pickPartOverridePos != p) {
        manager->pickPartOverridePos = p;
        IRect bbox = CalcPartBoundingBox(manager->pickPart, p);
        if (CanPlacePart(desk, bbox)) {
            manager->pickPartOverrideColor = V3(0.7f, 0.7f, 0.7f);
        } else {
            manager->pickPartOverrideColor = V3(0.7f, 0.2f, 0.2f);
        }
    }
}

void ToolPickRender(ToolManager* manager, Desk* desk) {
    DrawPart(desk, desk->canvas, manager->pickPart, MakeDeskPosition(manager->pickPartOverridePos), manager->pickPartOverrideColor, 0.7f, 0.5f);
}

void ToolPickPrimaryAction(ToolManager* manager, Desk* desk) {
    TryChangePartLocation(desk, manager->pickPart, manager->pickPartOverridePos);
    manager->currentTool = Tool::None;
}

void ToolPickSecondaryAction(ToolManager* manager, Desk* desk) {
    manager->currentTool = Tool::None;
}

void ToolNonePartClicked(ToolManager* manager, Desk* desk, Part* part) {
    switch (part->type) {
    case PartType::Source: {
        part->active = !part->active;
    } break;
    default: {} break;
    }
}

void ToolNonePrimaryAction(ToolManager* manager, Desk* desk) {
    DeskCell* mouseCell = GetDeskCell(desk, manager->mouseDeskPos.cell, false);
    if (mouseCell) {
        if (mouseCell->value != CellValue::Empty) {
            switch (mouseCell->value) {
            case CellValue::Part: {
                manager->currentTool = Tool::Pick;
                ToolPickEnable(manager, desk);
                //ToolNonePartClicked(manager, desk, mouseCell->part);
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

void ToolNoneSecondaryAction(ToolManager* manager, Desk* desk) {
    DeskCell* mouseCell = GetDeskCell(desk, manager->mouseDeskPos.cell, false);
    if (mouseCell) {
        switch (mouseCell->value) {
        case CellValue::Part: {
            Part* part = mouseCell->part;
            assert(part);
            DestroyPart(desk, part);
        } break;
        default: {} break;
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

void ToolManagerPrimaryAction(ToolManager* manager) {
    auto desk = GetDesk();
    switch (manager->currentTool) {
    case Tool::Part: { ToolPartPrimaryAction(manager, desk); } break;
    case Tool::Pick: { ToolPickPrimaryAction(manager, desk); } break;
    case Tool::Wire: { ToolWirePrimaryAction(manager, desk); } break;
    case Tool::None: { ToolNonePrimaryAction(manager, desk); } break;
    default: {} break;
    }
}

void ToolManagerSecondaryAction(ToolManager* manager) {
    auto desk = GetDesk();
    switch (manager->currentTool) {
    case Tool::Part: { ToolPartSecondaryAction(manager, desk); } break;
    case Tool::Pick: { ToolPickSecondaryAction(manager, desk); } break;
    case Tool::Wire: { ToolWireSecondaryAction(manager, desk); } break;
    case Tool::None: { ToolNoneSecondaryAction(manager, desk); } break;
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
    case Tool::Pick: { ToolPickUpdate(manager, desk); } break;
  //case Tool::None: { ToolNoneUpdate(manager, desk); } break;
    default: {} break;
    }
}

void ToolManagerRender(ToolManager* manager) {
    auto desk = GetDesk();
    switch (manager->currentTool) {
    case Tool::Part: { ToolPartRender(manager, desk); } break;
    case Tool::Pick: { ToolPickRender(manager, desk); } break;
    case Tool::Wire: { ToolWireRender(manager, desk); } break;
    default: {} break;
    }
}
