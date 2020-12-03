#include "Tools.h"

void ToolManagerInit(ToolManager* manager, Desk* desk) {
    manager->pendingWireNodes = Array<DeskPosition>(&desk->deskAllocator);
    manager->selectedParts = Array<Part*>(&desk->deskAllocator);
    manager->selectedPartsBlockedStates = Array<b32>(&desk->deskAllocator);
    manager->fullRebuildWiresBuffer = Array<Wire*>(&desk->deskAllocator);
    manager->partialRebuildWiresBuffer = Array<Wire*>(&desk->deskAllocator);
    manager->partialRebuildWiresSelectedEndsBuffer = Array<u8>(&desk->deskAllocator);
    manager->pickPartOverrideColorBlocked = V3(0.7f, 0.2f, 0.2f);
    manager->pickPartOverrideColor = V3(0.2f, 0.2f, 0.2f);
}

void ToolPartEnable(ToolManager* manager, Desk* desk, PartInitializerFn* initializer) {
    ToolManagerDisableAll(manager);
    manager->currentTool = Tool::Part;
    initializer(desk, &manager->prefabPart);
}

void ToolPartLeftMouseDown(ToolManager* manager, Desk* desk) {
    TryCreatePart(desk, desk->partInfo, manager->prefabPartPos.cell, manager->prefabPart.type);
}

void ToolPartRightMouseDown(ToolManager* manager, Desk* desk) {
    ToolManagerDisableAll(manager);
}

void ToolPartUpdate(ToolManager* manager, Desk* desk) {
    manager->prefabPartPos = manager->mouseDeskPos.Offset(-V2(manager->prefabPart.dim) * 0.5f * DeskCellSize);
}

void ToolPartRender(ToolManager* manager, Desk* desk) {
    DrawPart(desk, &desk->canvas, &manager->prefabPart, manager->prefabPartPos, {}, 0.0f, 0.5f);
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

void ToolWireRightMouseDown(ToolManager* manager, Desk* desk) {
    ToolManagerDisableAll(manager);
}

void ToolWireLeftMouseDown(ToolManager* manager, Desk* desk) {
    DeskEntity entity = GetAnyAt(desk, manager->mouseDeskPos);
    if (entity.type == DeskEntityType::Pin) {
        assert(entity.pin);
        ToolWirePinClicked(manager, desk, entity.pin);
    } else if (entity.type == DeskEntityType::None) {
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
                            nodeAtThisPIndex = _index_node_;
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

void ToolWireRender(ToolManager* manager, Desk* desk) {
    DrawListBeginBatch(&desk->canvas.drawList, TextureMode::Color);
    for (u32 i = 1; i < manager->pendingWireNodes.Count(); i++) {
        DeskPosition* prev = manager->pendingWireNodes.Data() + (i - 1);
        DeskPosition* curr = manager->pendingWireNodes.Data() + i;

        v2 begin = prev->RelativeTo(desk->origin);
        v2 end = curr->RelativeTo(desk->origin);

        DrawSimpleLineBatch(&desk->canvas.drawList, begin, end, 0.0f, 0.1f, V4(0.3f, 0.3f, 0.3f, 1.0f));
    }

    v2 begin = manager->pendingWireNodes.Last()->RelativeTo(desk->origin);
    v2 end = DeskPosition(manager->lastWireNodePos.cell).RelativeTo(desk->origin);

    DrawSimpleLineBatch(&desk->canvas.drawList, begin, end, 0.0f, 0.1f, V4(0.3f, 0.3f, 0.3f, 1.0f));
    DrawListEndBatch(&desk->canvas.drawList);
}

void ToolPickUnselectSelected(ToolManager* manager) {
    manager->selectedParts.Each([](Part** it) { (*it)->selected = false; });
    manager->selectedParts.Clear();
    manager->selectedPartsBlockedStates.Clear();
}

void ToolPickResetState(ToolManager* manager) {
    manager->dragOffset = {};
    manager->dragAttempt = false;
    manager->dragStarted = false;
    manager->rectSelectAttempt = false;
}

void ToolPickSelectPart(ToolManager* manager, Part* part, bool toggle) {
    Part** selected = manager->selectedParts.FindFirst([&part](Part** it) { return (*it) == part; });
    if (!selected) {
        manager->selectedParts.PushBack(part);
        manager->selectedPartsBlockedStates.PushBack(false);
        part->selected = true;
    } else if (toggle) {
        u32 index = manager->selectedParts.IndexFromPtr(selected);
        manager->selectedParts[index]->selected = false;
        manager->selectedParts.EraseUnsorted(index);
        manager->selectedPartsBlockedStates.EraseUnsorted(index);
    }
}

void ToolPickOffsetWire(Wire* wire, iv2 offset) { // TODO: v2?
    ForEach(&wire->nodes, node) {
        *node = node->Offset(offset);
    } EndEach;
}

// TODO: This function is extremely slow and unoptimized.
// Need otpimize this in the future.
// Also probably it is a good idea to execute this function asyncronously
void ToolPickRebuildWires(ToolManager* manager, Desk* desk) {
    // TODO: Ensure these buffers are not getting too large
    manager->fullRebuildWiresBuffer.Clear();
    manager->partialRebuildWiresBuffer.Clear();
    manager->partialRebuildWiresSelectedEndsBuffer.Clear();

    ForEach(&manager->selectedParts, partPtr) {
        Part* part = *partPtr;
        ForEach(&part->wires, record) {
            Wire* wire = record->wire;
            bool bothEndsSelected = false;
            if (wire->input->part == part && manager->selectedParts.FindFirst([&wire](Part** it) { return (*it) == wire->output->part; })) {
                bothEndsSelected = true;
            } else if (wire->output->part == part && manager->selectedParts.FindFirst([&wire](Part** it) { return (*it) == wire->input->part; })) {
                bothEndsSelected = true;
            }

            if (bothEndsSelected) {
                if (!manager->fullRebuildWiresBuffer.FindFirst([&record](Wire** it) { return (*it) == record->wire; })) {
                    manager->fullRebuildWiresBuffer.PushBack(wire);
                }
            } else {
                assert(wire->input->part == part || wire->output->part == part);
                if (!manager->partialRebuildWiresBuffer.FindFirst([&record](Wire** it) { return (*it) == record->wire; })) {
                    manager->partialRebuildWiresBuffer.PushBack(wire);
                    manager->partialRebuildWiresSelectedEndsBuffer.PushBack(wire->input->part == part ? 1 : 0);
                }
            }
        } EndEach;
    } EndEach;

    ForEach(&manager->fullRebuildWiresBuffer, wirePtr) {
        ToolPickOffsetWire(*wirePtr, manager->dragOffset);
    } EndEach;

    ForEach(&manager->partialRebuildWiresBuffer, wirePtr) {
        Wire* wire = *wirePtr;
        bool outputConnected = manager->partialRebuildWiresSelectedEndsBuffer[_index_wirePtr_] == 0 ? true : false;

        DeskPosition newPositions[3];
        DeskPosition pinPos = ComputePinPosition(outputConnected ? wire->output : wire->input);

        i32 x1 = pinPos.cell.x;
        i32 y1 = outputConnected ? wire->nodes[0].cell.y : wire->nodes.Last()->cell.y;
        i32 x2 = x1;
        i32 y2 = pinPos.cell.y;
        newPositions[0] = DeskPosition(IV2(x1, y1));
        newPositions[1] = DeskPosition(IV2(x2, y2));
        newPositions[2] = pinPos;

        if (outputConnected) {
            wire->nodes[0] = newPositions[0];
            DeskPosition tmp = newPositions[1];
            newPositions[1] = newPositions[2];
            newPositions[2] = tmp;
            wire->nodes.Prepend(newPositions + 1, 2);
        } else {
            *wire->nodes.Last() = newPositions[0];
            wire->nodes.Append(newPositions + 1, 2);
        }

        WireCleanupNodes(wire, &desk->wireNodeCleanerBuffer);
    } EndEach;
}

void ToolPickLeftMouseDown(ToolManager* manager, Desk* desk) {
    bool selectionMode = KeyDown(Key::LeftShift);

    if (manager->currentTool != Tool::Pick) {
        manager->dragOffset = {};
        manager->dragAttempt = false;
        manager->dragStarted = false;
        manager->rectSelectAttempt = false;
    }

    DeskEntity entity = GetAnyAt(desk, manager->mouseDeskPos);
    Part* part = entity.type == DeskEntityType::Part ? entity.part : nullptr;

    manager->currentTool = Tool::Pick;
    manager->pickPressedMousePos = manager->mouseCanvasPos;

    if (part) {
        manager->dragAttempt = true;
    } else {
        manager->rectSelectAttempt = true;
    }
}

void ToolPickLeftMouseUp(ToolManager* manager, Desk* desk) {
    manager->dragAttempt = false;
    manager->rectSelectAttempt = false;

    bool selectionMode = KeyDown(Key::LeftShift);

    // Apply rectangle selection
    if (manager->rectSelectStarted) {
        manager->rectSelectStarted = false;

        v2 cnvBegin = manager->pickPressedMousePos;
        v2 cnvEnd = manager->mouseCanvasPos;

        auto rect = MinMax(cnvBegin, cnvEnd);

        DeskPosition min = desk->origin.Offset(rect.min);
        DeskPosition max = desk->origin.Offset(rect.max);

        if (!selectionMode) {
            ToolPickUnselectSelected(manager);
        }

        u32 prevIndex = manager->selectedParts.Count();
        GetCollisions(desk, min, max, &manager->selectedParts);
        manager->selectedParts.KillDuplicatesUnsorted();

        for (u32 i = prevIndex; i < manager->selectedParts.Count(); i++) {
            manager->selectedPartsBlockedStates.PushBack(false);
            manager->selectedParts[i]->selected = true;
        }

        assert_paranoid(manager->selectedParts.Count() == manager->selectedPartsBlockedStates.Count());

        return;
    }

    if (manager->dragStarted) {
        manager->dragStarted = false;

        u32 blockedCount = manager->selectedPartsBlockedStates.CountIf([](b32* it) { return *it; });
        if (blockedCount == 0) {
            ForEach(&manager->selectedParts, partPtr) {
                Part* part = *partPtr;
                iv2 p = DeskPosition(part->p.cell + manager->dragOffset, part->p.offset).cell;
                ChangePartLocation(desk, part, DeskPosition(p));
            } EndEach;
            // TODO: Do this asyncronously
            ToolPickRebuildWires(manager, desk);
        }

        manager->dragOffset = {};
        return;
    }

    DeskEntity entity = GetAnyAt(desk, desk->origin.Offset(manager->pickPressedMousePos));
    Part* part = entity.type == DeskEntityType::Part ? entity.part : nullptr;

    if (!part && !selectionMode) {
        ToolPickUnselectSelected(manager);
        manager->currentTool = Tool::None;
        return;
    }

    if (!selectionMode) {
        ToolPickUnselectSelected(manager);
    }

    if (part) {
        ToolPickSelectPart(manager, part, true);
    }

    if (!selectionMode) {
        HandlePartClick(desk->partInfo, desk, part);
    }
}

void ToolPickRightMouseDown(ToolManager* manager, Desk* desk) {
    manager->currentTool = Tool::None;
}

void ToolPickUpdate(ToolManager* manager, Desk* desk) {
    if (!manager->dragStarted && !manager->rectSelectStarted) {
        auto input = GetInput();
        v2 offset = manager->mouseCanvasPos - manager->pickPressedMousePos;
        f32 delta = Length(offset);

        if (manager->dragAttempt) {
            assert(!manager->rectSelectAttempt);
            if (delta > ToolManager::PickMinThreshold) {
                manager->dragStarted = true;
                manager->dragOffset = DeskPosition(offset).cell;

                DeskEntity entity = GetAnyAt(desk, desk->origin.Offset(manager->pickPressedMousePos));
                Part* part = entity.type == DeskEntityType::Part ? entity.part : nullptr;
                if (part) {
                    bool selectionMode = KeyDown(Key::LeftShift);
                    if (!part->selected) {
                        if (!selectionMode) {
                            ToolPickUnselectSelected(manager);
                        }
                        ToolPickSelectPart(manager, part);
                    }
                }
            }
        } else if (manager->rectSelectAttempt) {
            if (delta > ToolManager::RectSelectMinThreshold) {
                manager->rectSelectStarted = true;
            }
        }
    } else if (manager->dragStarted) {
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
                manager->selectedPartsBlockedStates[_index_partPtr_] = CheckCollisions(desk, DeskPosition(bbox.min), DeskPosition(bbox.max), manager->selectedParts.AsRef());
            } EndEach;
        }
    }
}

void ToolPickRender(ToolManager* manager, Desk* desk) {
    if (manager->dragStarted) {
        ForEach(&manager->selectedParts, partPtr) {
            Part* part = *partPtr;
            b32 blocked = manager->selectedPartsBlockedStates[_index_partPtr_];
            v3 overrideColor = blocked ? manager->pickPartOverrideColorBlocked : manager->pickPartOverrideColor;
            DeskPosition p = DeskPosition(part->p.cell + manager->dragOffset, part->p.offset);
            DrawPart(desk, &desk->canvas, part, p, overrideColor, 0.7f, 0.5f);
        } EndEach;
    } else if (manager->rectSelectStarted) {
        DrawListBeginBatch(&desk->canvas.drawList, TextureMode::Color, 0);
        v2 cnvBegin = manager->pickPressedMousePos;
        v2 cnvEnd = manager->mouseCanvasPos;
        auto rect = MinMax(cnvBegin, cnvEnd);
        Box2D box = Box2D(rect.min, rect.max);
        DrawBoxBatch(&desk->canvas.drawList, box, 0.0f, 0.1f, V4(0.2f, 0.2f, 0.2f, 1.0f));
        DrawListEndBatch(&desk->canvas.drawList);
    }
}

void ToolNoneLeftMouseDown(ToolManager* manager, Desk* desk) {
    DeskEntity entity = GetAnyAt(desk, manager->mouseDeskPos);

    if (entity.type == DeskEntityType::Pin) {
        assert(entity.pin);
        ToolWirePinClicked(manager, desk, entity.pin);
        return;
    }

    auto wireAt = GetWireAt(desk, manager->mouseCanvasPos);
    if (wireAt.wire) {
        // Beginning a new wire and copy all nodes from the beginning of hit wire to the hit index
        manager->pendingWireNodes.Clear();
        wireAt.wire->nodes.CopyTo(&manager->pendingWireNodes, wireAt.nodeIndex + 1);
        manager->pendingWireNodes.PushBack(DeskPosition(manager->mouseDeskPos.cell));
        manager->lastWireNodePos = *wireAt.wire->nodes.Last();
        manager->pendingWireBeginPin = wireAt.wire->output;
        manager->currentTool = Tool::Wire;
        return;
    }

    ToolPickLeftMouseDown(manager, desk);
}

void ToolNoneLeftMouseUp(ToolManager* manager, Desk* desk) {
}

void ToolNoneRightMouseDown(ToolManager* manager, Desk* desk) {
    DeskEntity entity = GetAnyAt(desk, manager->mouseDeskPos);
    bool handled = false;

    switch (entity.type) {
    case DeskEntityType::Part: {
        Part* part = entity.part;
        assert(part);
        DestroyPart(desk, part);
        handled = true;
    } break;
    default: {} break;
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

void ToolManagerDisableAll(ToolManager* manager) {
    manager->currentTool = Tool::None;
}

void ToolManagerLeftMouseDown(ToolManager* manager) {
    auto desk = GetDesk();
    switch (manager->currentTool) {
    case Tool::Part: { ToolPartLeftMouseDown(manager, desk); } break;
    case Tool::Pick: { ToolPickLeftMouseDown(manager, desk); } break;
    case Tool::Wire: { ToolWireLeftMouseDown(manager, desk); } break;
    case Tool::None: { ToolNoneLeftMouseDown(manager, desk); } break;
    default: {} break;
    }
}

void ToolManagerLeftMouseUp(ToolManager* manager) {
    auto desk = GetDesk();
    switch (manager->currentTool) {
    case Tool::None: { ToolNoneLeftMouseUp(manager, desk); } break;
    case Tool::Pick: { ToolPickLeftMouseUp(manager, desk); } break;
    default: {} break;
    }
}

void ToolManagerRightMouseDown(ToolManager* manager) {
    auto desk = GetDesk();
    switch (manager->currentTool) {
    case Tool::Part: { ToolPartRightMouseDown(manager, desk); } break;
    case Tool::Pick: { ToolPickRightMouseDown(manager, desk); } break;
    case Tool::Wire: { ToolWireRightMouseDown(manager, desk); } break;
    case Tool::None: { ToolNoneRightMouseDown(manager, desk); } break;
    default: {} break;
    }
}

void ToolManagerUpdate(ToolManager* manager) {
    auto input = GetInput();
    auto desk = GetDesk();
    v2 mouseScreenPos = V2(input->mouseX, input->mouseY);
    v2 mouseCanvasPos = CanvasProjectScreenPos(&desk->canvas, mouseScreenPos);
    DeskPosition mouseDeskPos = desk->origin.Offset(mouseCanvasPos);
    manager->mouseDeskPos = mouseDeskPos;
    manager->mouseCanvasPos = mouseCanvasPos;

    switch (manager->currentTool) {
    case Tool::Part: { ToolPartUpdate(manager, desk); } break;
    case Tool::Wire: { ToolWireUpdate(manager, desk); } break;
    case Tool::Pick: { ToolPickUpdate(manager, desk); } break;
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
