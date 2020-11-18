#include "Tools.h"

// nocheckin
// Move somewhere
// TODO: Optimize this. Narrow down traversal subset. We cound for instance
// compute line bounding box and check only inside it.
struct GetWireAtResult {
    Wire* wire;
    u32 nodeIndex;
};

// Position is desk-relative
GetWireAtResult GetWireAt(Desk* desk, v2 p) {
    GetWireAtResult result {};

    ListForEach(&desk->wires, wire) {
        assert(wire->nodes.Count() >= 2);
        for (u32 i = 1; i < wire->nodes.Count(); i++) {
            DeskPosition* prev = wire->nodes.Data() + (i - 1);
            DeskPosition* curr = wire->nodes.Data() + i;

            v2 begin = prev->RelativeTo(desk->origin);
            v2 end = curr->RelativeTo(desk->origin);

            // TODO: Wire thickness
            f32 thickness = 0.1f;
            if (CheckWireSegmentHit(p, begin, end, thickness)) {
                result.wire = wire;
                result.nodeIndex = wire->nodes.IndexFromPtr(prev);
                break;
            }
        }
    } ListEndEach;

    return result;
}

void ToolManagerInit(ToolManager* manager, Desk* desk) {
    manager->pendingWireNodes = Array<DeskPosition>(&desk->deskAllocator);
}


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
    manager->prefabPartPos = manager->mouseDeskPos.Offset(-V2(manager->prefabPart.dim) * 0.5f * DeskCellSize);
}

void ToolPartRender(ToolManager* manager, Desk* desk) {
    DrawPart(desk, desk->canvas, &manager->prefabPart, manager->prefabPartPos, {}, 0.0f, 0.5f);
}

void ToolWirePinClicked(ToolManager* manager, Desk* desk, Pin* pin) {
    if (manager->currentTool != Tool::Wire) {
        assert(manager->currentTool == Tool::None);
        manager->pendingWireNodes.Clear();

        DeskPosition pPin = ComputePinPosition(pin);
        manager->lastWireNodePos = pPin;
        manager->pendingWireBeginPin = pin;
        manager->pendingWireNodes.PushBack(pPin);
        manager->pendingWireNodes.PushBack(DeskPosition(pPin.cell));

        manager->currentTool = Tool::Wire;
    } else {
        if (pin != manager->pendingWireBeginPin) {
            DeskPosition p = ComputePinPosition(pin);
            // Ensure we can make straight line from pin to last wire node
            DeskPosition lastNodeP = *manager->pendingWireNodes.Last();
            if (p.cell.x == lastNodeP.cell.x || p.cell.y == lastNodeP.cell.y) {

                manager->pendingWireNodes.PushBack(DeskPosition(p.cell));
                manager->pendingWireNodes.PushBack(p);

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

                if (pin == output) {
                    manager->pendingWireNodes.Reverse();
                }

                if (input && output) {
                    Wire* wire = TryWirePins(desk, input, output);
                    if (wire) {
                        manager->pendingWireNodes.CopyTo(&wire->nodes);
                    }
                }

                assert(manager->currentTool == Tool::Wire);
                manager->currentTool = Tool::None;
            }
        }
    }
}

void ToolWireSecondaryAction(ToolManager* manager, Desk* desk) {
    ToolManagerDisableAll(manager);
}

void ToolWirePrimaryAction(ToolManager* manager, Desk* desk) {
    DeskCell* mouseCell = GetDeskCell(desk, manager->mouseDeskPos.cell, true);
    if (mouseCell) {
        if (mouseCell->value == CellValue::Pin) {
            assert(mouseCell->pin);
            ToolWirePinClicked(manager, desk, mouseCell->pin);
        } else if (mouseCell->value == CellValue::Empty) {
            bool handled = false;
            if (manager->pendingWireBeginPin->type == PinType::Input) {
                auto wireAt = GetWireAt(desk, manager->mouseCanvasPos);
                if (wireAt.wire) {
                    assert(wireAt.wire->input);
                    assert(wireAt.wire->output);
                    Pin* input = manager->pendingWireBeginPin;
                    Pin* output = wireAt.wire->output;
                    Wire* wire = TryWirePins(desk, input, output);
                    if (wire) {
                        // Search for nodes at this position
                        i32 nodeAtThisPIndex = -1;
                        ForEach(&wireAt.wire->nodes, node) {
                            if (node->cell == manager->mouseDeskPos.cell) {
                                nodeAtThisPIndex = index;
                                break;
                            }
                        } EndEach;

                        if (nodeAtThisPIndex == -1) {
                            // Insert new node
                            DeskPosition* newNode = wireAt.wire->nodes.Insert(wireAt.nodeIndex + 1);
                            *newNode = DeskPosition(manager->mouseDeskPos.cell);
                            nodeAtThisPIndex = wireAt.nodeIndex + 1;
                        }

                        assert(nodeAtThisPIndex != -1);

                        manager->pendingWireNodes.Reverse();
                        manager->pendingWireNodes.Prepend(wireAt.wire->nodes.data, nodeAtThisPIndex + 1);
                        manager->pendingWireNodes.CopyTo(&wire->nodes);
                        handled = true;
                        assert(manager->currentTool == Tool::Wire);
                        manager->currentTool = Tool::None;
                    }
                }
            }

            if (!handled) {
                manager->pendingWireNodes.PushBack(DeskPosition(manager->lastWireNodePos.cell));
            }
        }
    }
}

void ToolWireUpdate(ToolManager* manager, Desk* desk) {
    DeskPosition last = *manager->pendingWireNodes.Last();
    v2 pRel = manager->mouseDeskPos.Sub(last);
    DeskPosition offset;
    if (Abs(pRel.x) > Abs(pRel.y)) {
        manager->lastWireNodePos = last.Offset(V2(pRel.x, 0.0f));
    } else {
        manager->lastWireNodePos = last.Offset(V2(0.0f, pRel.y));
    }
}

void ToolPickEnable(ToolManager* manager, Desk* desk) {
    manager->pickPart = nullptr;
    manager->pickPartOverridePos = manager->mouseDeskPos.cell;
    manager->pickPartOverrideColor = V3(0.7f, 0.2f, 0.2f);
    DeskCell* mouseCell = GetDeskCell(desk, manager->mouseDeskPos.cell, false);
    if (mouseCell->value == CellValue::Part) {
        manager->pickPart = mouseCell->part;
        manager->toolPickActuallyEnabled = false;
        manager->toolPickLastMouseP = manager->mouseDeskPos.cell;
    } else {
        manager->currentTool = Tool::None;
    }
}

void ToolPickUpdate(ToolManager* manager, Desk* desk) {
    if (!manager->toolPickActuallyEnabled) {
        if (manager->mouseDeskPos.cell != manager->toolPickLastMouseP) {
            manager->toolPickActuallyEnabled = true;
        } else {
            manager->toolPickLastMouseP = manager->mouseDeskPos.cell;

            // TODO: For now primary action of kind of input button agnostic
            // That was probably a bad decision. Tool manager should alway know
            // his input layout and do things based on that layout
            if (!MouseButtonDown(MouseButton::Left)) {
                manager->currentTool = Tool::None;

                // TODO: Here will be more complicated stuff
                switch (manager->pickPart->type) {
                case PartType::Source: {
                    manager->pickPart->active = !manager->pickPart->active;
                } break;
                default: {} break;
                }
            }
        }
    } else {
        iv2 p = manager->mouseDeskPos.Offset(-V2(manager->pickPart->dim) * 0.5f * DeskCellSize).cell;
        if (manager->pickPartOverridePos != p) {
            manager->pickPartOverridePos = p;
            IRect bbox = CalcPartBoundingBox(manager->pickPart, p);
            if (CanPlacePart(desk, bbox, manager->pickPart)) {
                manager->pickPartOverrideColor = V3(0.7f, 0.7f, 0.7f);
            } else {
                manager->pickPartOverrideColor = V3(0.7f, 0.2f, 0.2f);
            }
        }

        if (!MouseButtonDown(MouseButton::Left)) {
            TryChangePartLocation(desk, manager->pickPart, manager->pickPartOverridePos);
            manager->currentTool = Tool::None;
        }
    }
}

void ToolPickRender(ToolManager* manager, Desk* desk) {
    if (manager->toolPickActuallyEnabled) {
        DrawPart(desk, desk->canvas, manager->pickPart, DeskPosition(manager->pickPartOverridePos), manager->pickPartOverrideColor, 0.7f, 0.5f);
    }
}

void ToolPickSecondaryAction(ToolManager* manager, Desk* desk) {
    manager->currentTool = Tool::None;
}

void ToolNonePrimaryAction(ToolManager* manager, Desk* desk) {
    bool processed = false;
    DeskCell* mouseCell = GetDeskCell(desk, manager->mouseDeskPos.cell, false);
    if (mouseCell) {
        if (mouseCell->value != CellValue::Empty) {
            switch (mouseCell->value) {
            case CellValue::Part: {
                manager->currentTool = Tool::Pick;
                ToolPickEnable(manager, desk);
                //ToolNonePartClicked(manager, desk, mouseCell->part);
                processed = true;
            } break;
            case CellValue::Pin: {
                assert(mouseCell->pin);
                ToolWirePinClicked(manager, desk, mouseCell->pin);
                processed = true;
            } break;
            default: {} break;
            }
        }
    }

    if (!processed) {
        // Searching for wires under cursor
        auto wireAt = GetWireAt(desk, manager->mouseCanvasPos);
        if (wireAt.wire) {
            // Beginning a new wire and copy all nodes from the beginning of hit wire to the hit index
            manager->pendingWireNodes.Clear();
            wireAt.wire->nodes.CopyTo(&manager->pendingWireNodes, wireAt.nodeIndex + 1);
            manager->pendingWireNodes.PushBack(DeskPosition(manager->mouseDeskPos.cell));
            manager->lastWireNodePos = *wireAt.wire->nodes.Last();
            manager->pendingWireBeginPin = wireAt.wire->output;
            manager->currentTool = Tool::Wire;
        }
    }
}

void ToolNoneSecondaryAction(ToolManager* manager, Desk* desk) {
    DeskCell* mouseCell = GetDeskCell(desk, manager->mouseDeskPos.cell, false);
    bool handled = false;
    if (mouseCell) {
        switch (mouseCell->value) {
        case CellValue::Part: {
            Part* part = mouseCell->part;
            assert(part);
            DestroyPart(desk, part);
            handled = true;
        } break;
        default: {} break;
        }
    }
    if (!handled) {
        // Delete all wires under cursor
        while (true) {
            auto wire = GetWireAt(desk, manager->mouseCanvasPos);
            if (wire.wire) {
                DestroyWire(desk, wire.wire);
            } else {
                break;
            }
        }
    }
}

void ToolWireRender(ToolManager* manager, Desk* desk) {
    DrawListBeginBatch(&desk->canvas->drawList, TextureMode::Color);
    for (u32 i = 1; i < manager->pendingWireNodes.Count(); i++) {
        DeskPosition* prev = manager->pendingWireNodes.Data() + (i - 1);
        DeskPosition* curr = manager->pendingWireNodes.Data() + i;

        v2 begin = prev->RelativeTo(desk->origin);
        v2 end = curr->RelativeTo(desk->origin);

        DrawSimpleLineBatch(&desk->canvas->drawList, begin, end, 0.0f, 0.1f, V4(0.3f, 0.3f, 0.3f, 1.0f));
    }

    v2 begin = manager->pendingWireNodes.Last()->RelativeTo(desk->origin);
    v2 end = DeskPosition(manager->lastWireNodePos.cell).RelativeTo(desk->origin);

    DrawSimpleLineBatch(&desk->canvas->drawList, begin, end, 0.0f, 0.1f, V4(0.3f, 0.3f, 0.3f, 1.0f));
    DrawListEndBatch(&desk->canvas->drawList);
}

void ToolManagerDisableAll(ToolManager* manager) {
    manager->currentTool = Tool::None;
}

void ToolManagerPrimaryAction(ToolManager* manager) {
    auto desk = GetDesk();
    switch (manager->currentTool) {
    case Tool::Part: { ToolPartPrimaryAction(manager, desk); } break;
    //case Tool::Pick: { ToolPickPrimaryAction(manager, desk); } break;
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
    DeskPosition mouseDeskPos = desk->origin.Offset(mouseCanvasPos);
    manager->mouseDeskPos = mouseDeskPos;
    manager->mouseCanvasPos = mouseCanvasPos;

    switch (manager->currentTool) {
    case Tool::Part: { ToolPartUpdate(manager, desk); } break;
    case Tool::Wire: { ToolWireUpdate(manager, desk); } break;
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
