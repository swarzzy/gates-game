#include "PartInfo.h"

void FuncPartAnd(Part* part) {
    u8 value = 1;
    for (u32 i = 0; i < part->inputCount; i++) {
        value = value && part->inputs[i].value;
    }
    part->outputs[0].value = value;
}

void FuncPartOr(Part* part) {
    u8 value = 0;
    for (u32 i = 0; i < part->inputCount; i++) {
        value = value || part->inputs[i].value;
    }
    part->outputs[0].value = value;
}

void FuncPartNot(Part* part) {
    u8 value = !part->inputs[0].value;
    part->outputs[0].value = value;
}

void FuncPartSource(Part* part) {
    part->outputs[0].value = (u8)part->active;
}

void FuncPartLed(Part* part) {
    part->active = part->inputs[0].value;
}

void InitPartAnd(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::And;
    part->dim = IV2(3, 5);
    part->inputCount = 2;
    part->outputCount = 1;
    part->inputs[0] = CreatePin(desk, part, -1, 1, PinType::Input);
    part->inputs[1] = CreatePin(desk, part, -1, 3, PinType::Input);
    part->outputs[0] = CreatePin(desk, part, 3, 3, PinType::Output);
    part->activeColor = V4(0.4f, 0.6f, 0.0f, 1.0f);
    part->inactiveColor = V4(0.4f, 0.6f, 0.0f, 1.0f);
}

void InitPartOr(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Or;
    part->dim = IV2(3, 5);
    part->inputCount = 2;
    part->outputCount = 1;
    part->inputs[0] = CreatePin(desk, part, -1, 1, PinType::Input);
    part->inputs[1] = CreatePin(desk, part, -1, 3, PinType::Input);
    part->outputs[0] = CreatePin(desk, part, 3, 3, PinType::Output);
    part->activeColor = V4(0.0f, 0.0f, 0.6f, 1.0f);
    part->inactiveColor = V4(0.0f, 0.0f, 0.6f, 1.0f);
}

void InitPartNot(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Not;
    part->dim = IV2(3, 3);
    part->inputCount = 1;
    part->outputCount = 1;
    part->inputs[0] = CreatePin(desk, part, -1, 1, PinType::Input);
    part->outputs[0] = CreatePin(desk, part, 3, 1, PinType::Output);
    part->activeColor = V4(0.6f, 0.0f, 0.0f, 1.0f);
    part->inactiveColor = V4(0.6f, 0.0f, 0.0f, 1.0f);
}

void InitPartLed(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Led;
    part->dim = IV2(3, 3);
    part->inputCount = 1;
    part->inputs[0] = CreatePin(desk, part, -1, 1, PinType::Input);
    part->activeColor = V4(0.9f, 0.0f, 0.0f, 1.0f);
    part->inactiveColor = V4(0.1f, 0.1f, 0.1f, 1.0f);
}

void InitPartSource(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Source;
    part->dim = IV2(3, 3);
    part->outputCount = 1;
    part->outputs[0] = CreatePin(desk, part, 3, 1, PinType::Output);
    part->activeColor = V4(0.7f, 0.7f, 0.0f, 1.0f);
    part->inactiveColor = V4(0.0f, 0.0f, 0.5f, 1.0f);
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

PartID GetPartID(PartInfo* info) {
    PartID id = { ++info->partSerialCount };
    return id;
}

void InitPart(PartInfo* info, Desk* desk, Part* part, iv2 p, PartType type) {
    assert((u32)type < (u32)PartType::_Count);
    auto initializer = info->partInitializers[(u32)type];
    initializer(desk, part);
    part->id = GetPartID(info);
    part->p = DeskPositionNormalize(MakeDeskPosition(p));
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
    pin.wires.Init(desk->deskAllocator);
    return pin;
}

Wire* TryWirePins(Desk* desk, Pin* pin0, Pin* pin1) {
    Wire* result = nullptr;
    Wire* wire = desk->wires.PushBack();
    if (wire) {
        bool assigned = false;
        Wire** p1Wire = pin0->wires.PushBack();
        if (p1Wire) {
            Wire** p2Wire = pin0->wires.PushBack();
            if (p2Wire) {
                assigned = true;
                *p1Wire = wire;
                *p2Wire = wire;
                wire->pin0 = pin0;
                wire->p0 = ComputePinPosition(pin0);
                wire->pin1 = pin1;
                wire->p1 = ComputePinPosition(pin1);
                result = wire;
            }
        } else {
            pin0->wires.PopBack();
        }
        if (!assigned) {
            desk->wires.PopBack();
        }
    }
    return result;
}
