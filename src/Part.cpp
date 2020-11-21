#include "Part.h"

#include "PartInfo.h"

void InitPart(PartInfo* info, Desk* desk, Part* part, iv2 p, PartType type) {
    assert((u32)type < (u32)PartType::_Count);
    auto initializer = info->partInitializers[(u32)type];
    initializer(desk, part);
    part->p = DeskPosition(p).Normalized();
    part->wires = Array<WireRecord>(&desk->deskAllocator);
}

void DeinitPart(Desk* desk, Part* part) {
    DeallocatePins(desk, part);
    part->wires.FreeBuffers();
}

Pin* GetInput(Part* part, u32 index) {
    assert(index < part->inputCount);
    Pin* result = part->pins.Data() + index;
    return result;
}

Pin* GetOutput(Part* part, u32 index) {
    assert(index < part->outputCount);
    Pin* result = part->pins.Data() + (index + part->inputCount);
    return result;
}

u32 PinCount(Part* part) {
    u32 count = part->inputCount + part->outputCount;
    return count;
}

v4 GetPartColor(Part* element) {
    return element->active ? element->activeColor : element->inactiveColor;
}

void DrawPart(Desk* desk, Canvas* canvas, Part* part, DeskPosition overridePos, v3 overrideColor, f32 overrideColorFactor, f32 alpha) {
    DeskPosition maxP = DeskPosition(overridePos.cell + part->dim);
    v2 min = DeskPosition(overridePos.cell).RelativeTo(desk->origin) - DeskCellHalfSize;
    v2 max = maxP.RelativeTo(desk->origin) - DeskCellHalfSize;

    v2 p0 = min;
    v2 p1 = V2(max.x, min.y);
    v2 p2 = max;
    v2 p3 = V2(min.x, max.y);

    v3 partColor = GetPartColor(part).xyz;
    v4 color = V4(Lerp(partColor, overrideColor, overrideColorFactor), alpha);

    if (part->selected) {
        color.xyz *= 0.5f;
    }

    DrawListPushRect(&canvas->drawList, min, max, 0.0f, V4(0.0f, 0.0f, 0.0f, 1.0f));
    DrawListPushRect(&canvas->drawList, min + V2(0.1f), max - V2(0.1f), 0.0f, color);

    for (u32 pinIndex = 0; pinIndex < part->inputCount; pinIndex++) {
        Pin* pin = GetInput(part, pinIndex);
        v4 color = V4(0.0f, 0.0f, 0.0f, 1.0f);
        DeskPosition pinPos = ComputePinPosition(pin, overridePos);
        v2 relPos = pinPos.RelativeTo(desk->origin);
        v2 pinMin = relPos - V2(0.1);
        v2 pinMax = relPos + V2(0.1);
        DrawListPushRect(&canvas->drawList, pinMin, pinMax, 0.0f, color);
    }
    for (u32 pinIndex = 0; pinIndex < part->outputCount; pinIndex++) {
        Pin* pin = GetOutput(part, pinIndex);
        v4 color = V4(0.0f, 0.0f, 0.0f, 1.0f);
        DeskPosition pinPos = ComputePinPosition(pin, overridePos);
        v2 relPos = pinPos.RelativeTo(desk->origin);
        v2 pinMin = relPos - V2(0.1);
        v2 pinMax = relPos + V2(0.1);
        DrawListPushRect(&canvas->drawList, pinMin, pinMax, 0.0f, color);
    }
    if (part->label) {
        v2 center = min + (max - min) * 0.5f;
        v3 p = V3(center, 0.0f);
        auto context = GetContext();
        DrawText(&canvas->drawList, &context->sdfFont, part->label, p, V4(0.0f, 0.0f, 0.0f, 1.0f), V2(canvas->cmPerPixel), V2(0.5f), F32::Infinity, TextAlign::Left, canvas->scale);
    }
}

void DrawPart(Desk* desk, Canvas* canvas, Part* element, v3 overrideColor, f32 overrideColorFactor, f32 alpha) {
    DrawPart(desk, canvas, element, element->p, overrideColor, overrideColorFactor, alpha);
}

DeskPosition ComputePinPosition(Pin* pin,  DeskPosition partPosition) {
    DeskPosition result {};
    iv2 cell = partPosition.cell + pin->pRelative;
    switch (pin->type) {
    // Subtract a small number to prevent flooring in the wrong cell
    case PinType::Input: { result = DeskPosition(cell, V2(DeskCellHalfSize - 0.0001f, 0.0f)); } break;
    case PinType::Output: { result = DeskPosition(cell, V2(-DeskCellHalfSize, 0.0f)); } break;
        invalid_default();
    }
    return result;
}

DeskPosition ComputePinPosition(Pin* pin) {
    return ComputePinPosition(pin, pin->part->p);
}

// TODO: Think about better 'architecture' of this wire caching and updating stuff
// noheckin remove this
void UpdateCachedWirePositions(Part* part) {
    ForEach(&part->wires, record) {
        Wire* wire = record->wire;
        Pin* pin = record->pin;
        assert(wire->nodes.Count() >= 4);
        if (pin->type == PinType::Output) {
            DeskPosition p = ComputePinPosition(pin);
            wire->nodes[0] = p;
            wire->nodes[1] = DeskPosition(p.cell);
            if (wire->nodes.Count() >= 6) {
                wire->nodes[2] = DeskPosition(IV2(wire->nodes[2].cell.x, p.cell.y));
            }
        } else {
            DeskPosition p = ComputePinPosition(pin);
            wire->nodes[wire->nodes.Count() - 1] = p;
            wire->nodes[wire->nodes.Count() - 2] = DeskPosition(p.cell);
            if (wire->nodes.Count() >= 6) {
                wire->nodes[wire->nodes.Count() - 3] = DeskPosition(IV2(wire->nodes[wire->nodes.Count() - 3].cell.x, p.cell.y));
            }
        }
    } EndEach;
}

void WireCleanupNodes(Wire* wire, Array<DeskPosition>* buffer) {
    assert(wire->nodes.Count() >= 4);
    buffer->Clear();
    buffer->PushBack(wire->nodes[0]);
    for (u32 i = 1; i < wire->nodes.Count() - 1; i++) {
        buffer->PushBack(wire->nodes[i]);
        bool xWasTheSame = true;
        bool yWasTheSame = true;
        for (u32 j = i + 1; j < wire->nodes.Count() - 1; j++) {
            if ((wire->nodes[i].cell.x != wire->nodes[j].cell.x)) {
                xWasTheSame = false;
            }

            if (wire->nodes[i].cell.y != wire->nodes[j].cell.y) {
                yWasTheSame = false;
            }

            if (!xWasTheSame && !yWasTheSame) {
                buffer->PushBack(wire->nodes[j - 1]);
                i = j - 2;
                break;
            }
        }
    }

    buffer->PushBack(*wire->nodes.Last());
    buffer->CopyTo(&wire->nodes);
}

IRect CalcPartBoundingBox(Part* part, iv2 overridePos) {
    IRect rect {};
    rect.min = overridePos;
    rect.max = overridePos + part->dim;
    ForEach(&part->pins, pin) {
        iv2 pinP = overridePos + pin->pRelative;
        rect.min = IV2(Min(rect.min.x, pinP.x), Min(rect.min.y, pinP.y));
        rect.max = IV2(Max(rect.max.x, pinP.x), Max(rect.max.y, pinP.y));
    } EndEach;
    return rect;
}

IRect CalcPartBoundingBox(Part* part) {
    return CalcPartBoundingBox(part, part->p.cell);
}

Part* CreatePart(Desk* desk, PartInfo* info, iv2 p, PartType type) {
    Part* result = nullptr;
    // TODO: Check if we can place the part before allocate it!!
    Part* part = GetPartMemory(desk);
    if (part) {
        InitPart(info, desk, part, p, type);
        if (AddPartToDesk(desk, part)) {
            result = part;
        } else {
            ReleasePartMemory(desk, part);
        }
    }

    return result;
}

void UnwirePart(Desk* desk, Part* part) {
    ForEach(&part->wires, record) {
        Pin* pin = record->pin;
        Wire* wire = record->wire;
        if (pin->type == PinType::Input) {
            bool unwired = UnwirePin(wire->output, wire);
            assert(unwired);
        } else if (pin->type == PinType::Output) {
            bool unwired = UnwirePin(wire->input, wire);
            assert(unwired);
        } else {
            unreachable();
        }
        RemoveWire(desk, wire);
    } EndEach;
}

void DestroyPart(Desk* desk, Part* part) {
    UnwirePart(desk, part);
    UnregisterPartPlacement(desk, part);
    DeinitPart(desk, part);
    ReleasePartMemory(desk, part);
}

void PartProcessSignals(PartInfo* info, Part* part) {
    assert((u32)part->type < (u32)PartType::_Count);
    auto partFn = info->partFunctions[(u32)part->type];
    assert(partFn);
    partFn(part);
}

bool ArePinsWired(Pin* input, Pin* output) {
    bool result = false;
    ForEach(&input->part->wires, record) {
        if (record->pin == input) {
            Wire* wire = record->wire;
            if (wire->output == output) {
                result = true;
                break;
            }
        }
    } EndEach;
    return result;
}

Wire* TryWirePins(Desk* desk, Pin* input, Pin* output) {
    assert(input->type == PinType::Input);
    assert(output->type == PinType::Output);

    Wire* result = nullptr;

    bool inputIsFree = true;
    ForEach(&input->part->wires, record) {
        if (record->pin == input) {
            inputIsFree = false;
            break;
        }
    } EndEach;

    if (inputIsFree) {
        if (!ArePinsWired(input, output)) {
            Wire* wire = AddWire(desk);

            auto inputRecord = input->part->wires.PushBack();
            auto outputRecord = output->part->wires.PushBack();
            wire->input = input;
            inputRecord->wire = wire;
            inputRecord->pin = input;

            wire->output = output;
            outputRecord->wire = wire;
            outputRecord->pin = output;

            DeskPosition* inputNode = wire->nodes.PushFront();
            DeskPosition* outputNode = wire->nodes.PushBack();

            *inputNode = ComputePinPosition(input);
            *outputNode = ComputePinPosition(output);

            //wire->pInput = inputNode->p;
            //wire->pOutput = outputNode->p;

            result = wire;
        }
    }

    return result;
}

bool UnwirePin(Pin* pin, Wire* wire) {
    // Ensure wire and pin are connected
    bool result = false;
    WireRecord* record = nullptr;
    ForEach(&pin->part->wires, current) {
        if (current->wire == wire) {
            record = current;
            result = true;
            break;
        }
    } EndEach;

    if (record) {
        pin->part->wires.EraseUnsorted(record);
    }

    return result;
}

Pin CreatePin(Part* part, i32 xRel, i32 yRel, PinType type) {
    Pin pin {};
    pin.type = type;
    pin.part = part;
    pin.pRelative = IV2(xRel, yRel);
    return pin;
}

void DestroyWire(Desk* desk, Wire* wire) {
    bool unwired = UnwirePin(wire->input, wire);
    assert(unwired);
    unwired = UnwirePin(wire->output, wire);
    RemoveWire(desk, wire);
}
