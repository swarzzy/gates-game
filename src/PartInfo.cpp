#include "PartInfo.h"

void FuncPartAnd(Part* part) {
    u8 value = 1;
    for (u32 i = 0; i < part->inputCount; i++) {
        value = value && GetInput(part, i)->value;
    }
    GetOutput(part, 0)->value = value;
}

void FuncPartOr(Part* part) {
    u8 value = 0;
    for (u32 i = 0; i < part->inputCount; i++) {
        value = value || GetInput(part, i)->value;
    }
    GetOutput(part, 0)->value = value;
}

void FuncPartNot(Part* part) {
    u8 value = !GetInput(part, 0)->value;
    GetOutput(part, 0)->value = value;
}

void FuncPartSource(Part* part) {
    GetOutput(part, 0)->value = (u8)part->active;
    if (part->active) {
        part->label = u"0";
    } else {
        part->label = u"I";
    }
}

void FuncPartLed(Part* part) {
    part->active = GetInput(part, 0)->value;
}

void AllocatePins(Desk* desk, Part* part, u32 inputCount, u32 outputCount) {
    u32 count = inputCount + outputCount;
    part->pins = Array<Pin>(&desk->deskAllocator, count);
    part->inputCount = inputCount;
    part->outputCount = outputCount;
}

void DeallocatePins(Desk* desk, Part* part) {
    part->pins.FreeBuffers();
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

void InitPartAnd(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::And;
    part->dim = IV2(3, 5);

    part->activeColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->inactiveColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->label = u"&";

    AllocatePins(desk, part, 2, 1);
    *GetInput(part, 0) = CreatePin(desk, part, -1, 1, PinType::Input);
    *GetInput(part, 1) = CreatePin(desk, part, -1, 3, PinType::Input);
    *GetOutput(part, 0) = CreatePin(desk, part, 3, 3, PinType::Output);
}

void InitPartOr(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Or;
    part->dim = IV2(3, 5);

    part->activeColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->inactiveColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->label = u"1";

    AllocatePins(desk, part, 2, 1);
    *GetInput(part, 0) = CreatePin(desk, part, -1, 1, PinType::Input);
    *GetInput(part, 1) = CreatePin(desk, part, -1, 3, PinType::Input);
    *GetOutput(part, 0) = CreatePin(desk, part, 3, 3, PinType::Output);
}

void InitPartNot(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Not;
    part->dim = IV2(3, 3);

    part->activeColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->inactiveColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    //part->label = u"0";

    AllocatePins(desk, part, 1, 1);
    *GetInput(part, 0) = CreatePin(desk, part, -1, 1, PinType::Input);
    *GetOutput(part, 0) = CreatePin(desk, part, 3, 1, PinType::Output);
}

void InitPartLed(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Led;
    part->dim = IV2(3, 3);

    part->activeColor = V4(0.9f, 0.0f, 0.0f, 1.0f);
    part->inactiveColor = V4(0.3f, 0.3f, 0.3f, 1.0f);

    AllocatePins(desk, part, 1, 0);
    *GetInput(part, 0) = CreatePin(desk, part, -1, 1, PinType::Input);
}

void InitPartSource(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Source;
    part->dim = IV2(3, 3);

    part->label = u"0";

    part->activeColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->inactiveColor = V4(1.0f, 1.0f, 1.0f, 1.0f);

    AllocatePins(desk, part, 0, 1);
    *GetOutput(part, 0) = CreatePin(desk, part, 3, 1, PinType::Output);
}

void PartInfoInit(PartInfo* info) {
    // TODO: Ensure memory are zero
    memset(info, 0, sizeof(PartInfo));

    info->partInitializers[(u32)PartType::And] = InitPartAnd;
    info->partInitializers[(u32)PartType::Or] = InitPartOr;
    info->partInitializers[(u32)PartType::Not] = InitPartNot;
    info->partInitializers[(u32)PartType::Source] = InitPartSource;
    info->partInitializers[(u32)PartType::Led] = InitPartLed;

    info->partFunctions[(u32)PartType::And] = FuncPartAnd;
    info->partFunctions[(u32)PartType::Or] = FuncPartOr;
    info->partFunctions[(u32)PartType::Not] = FuncPartNot;
    info->partFunctions[(u32)PartType::Source] = FuncPartSource;
    info->partFunctions[(u32)PartType::Led] = FuncPartLed;
}

u32 GetPartID(PartInfo* info) {
    u32 id = { ++info->partSerialCount };
    return id;
}

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

void PartProcessSignals(PartInfo* info, Part* part) {
    assert((u32)part->type < (u32)PartType::_Count);
    auto partFn = info->partFunctions[(u32)part->type];
    assert(partFn);
    partFn(part);
}

Pin CreatePin(Desk* desk, Part* part, i32 xRel, i32 yRel, PinType type) {
    Pin pin {};
    pin.type = type;
    pin.part = part;
    pin.pRelative = IV2(xRel, yRel);
    return pin;
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
            Wire* wire = desk->wires.Add();

            auto inputRecord = input->part->wires.Push();
            auto outputRecord = output->part->wires.Push();
            wire->input = input;
            inputRecord->wire = wire;
            inputRecord->pin = input;

            wire->output = output;
            outputRecord->wire = wire;
            outputRecord->pin = output;

            wire->pInput = ComputePinPosition(input);
            wire->pOutput = ComputePinPosition(output);

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
