#include "Tools.h"

void ToolManagerInit(ToolManager* manager, Desk* desk) {
    manager->pendingWireNodes = Array<DeskPosition>(&desk->deskAllocator);
    manager->selectedParts = Array<Part*>(&desk->deskAllocator);
    manager->selectedPartsBlockedStates = Array<b32>(&desk->deskAllocator);
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
                                nodeAtThisPIndex = _index_;
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

void ToolPickPartPressed(ToolManager* manager, Desk* desk, Part* part) {
    if (!manager->pickStarted) {
        if (part) {
            manager->selectedParts.Clear();
            manager->selectedPartsBlockedStates.Clear();
            manager->dragOffset = {};
            manager->pickPartOverrideColorBlocked = V3(0.7f, 0.2f, 0.2f);
            manager->pickPartOverrideColor = V3(0.2f, 0.2f, 0.2f);

            manager->selectedParts.PushBack(part);
            manager->selectedPartsBlockedStates.PushBack(false);
            manager->pickPressedMousePos = manager->mouseCanvasPos;

            part->selected = true;
        }
    }
}

void ToolPickUpdate(ToolManager* manager, Desk* desk) {
    if (!manager->pickStarted) {
        assert(manager->selectedParts.Count() > 0);
        if (MouseButtonDown(MouseButton::Left)) {
            auto input = GetInput();
            f32 delta = Length(manager->mouseCanvasPos - manager->pickPressedMousePos);
            if (delta > ToolManager::PickMinThreshold) {
                manager->pickStarted = true;
            }
        } else {
            manager->currentTool = Tool::None;
            manager->pickStarted = false;

            assert(manager->selectedParts.Count() == 1);
            // TODO: Here will be more complicated stuff
            Part* part = *manager->selectedParts.Last();
            switch (part->type) {
            case PartType::Source: {
                part->active = !part->active;
            } break;
            default: {} break;
            }

            manager->selectedParts.Each([](Part** it) { (*it)->selected = false; });
        }
    } else {
        v2 offset = manager->mouseCanvasPos - manager->pickPressedMousePos;

        DeskPosition prevDesk = DeskPosition(manager->dragOffset);
        DeskPosition currDesk = DeskPosition(offset);

        if (prevDesk.cell != currDesk.cell) {
            manager->dragOffset = currDesk.cell;

            assert_paranoid(manager->selectedParts.Count() == manager->selectedPartsBlockedStates.Count());
            ForEach(&manager->selectedParts, partPtr) {
                Part* part = *partPtr;
                iv2 p = DeskPosition(part->p.cell + manager->dragOffset, part->p.offset).cell;
                IRect bbox = CalcPartBoundingBox(part, p);
                manager->selectedPartsBlockedStates[_index_] = !CanPlacePart(desk, bbox, part);
            } EndEach;
        }

        if (!MouseButtonDown(MouseButton::Left)) {
            u32 blockedCount = manager->selectedPartsBlockedStates.CountIf([](b32* it) { return *it; });
            if (blockedCount == 0) {
                ForEach(&manager->selectedParts, partPtr) {
                    Part* part = *partPtr;
                    iv2 p = DeskPosition(part->p.cell + manager->dragOffset, part->p.offset).cell;
                    TryChangePartLocation(desk, part, p);
                    //manager->pickPart->selected = false;
                } EndEach;
            }
            manager->currentTool = Tool::None;
            manager->pickStarted = false;

            manager->selectedParts.Each([](Part** it) { (*it)->selected = false; });
        }
    }
}

void ToolPickRender(ToolManager* manager, Desk* desk) {
    if (manager->pickStarted) {
        ForEach(&manager->selectedParts, partPtr) {
            Part* part = *partPtr;
            b32 blocked = manager->selectedPartsBlockedStates[_index_];
            v3 overrideColor = blocked ? manager->pickPartOverrideColorBlocked : manager->pickPartOverrideColor;
            DeskPosition p = DeskPosition(part->p.cell + manager->dragOffset, part->p.offset);
            DrawPart(desk, desk->canvas, part, p, overrideColor, 0.7f, 0.5f);
        } EndEach;
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
                ToolPickPartPressed(manager, desk, mouseCell->part);
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
