#include "PartInfo.h"

void PartAndFunc(Part* part) {
    u8 value = 1;
    for (u32 i = 0; i < part->inputCount; i++) {
        value = value && part->inputs[i].value;
    }
    part->outputs[0].value = value;
}

void PartOrFunc(Part* part) {
    u8 value = 0;
    for (u32 i = 0; i < part->inputCount; i++) {
        value = value || part->inputs[i].value;
    }
    part->outputs[0].value = value;
}

void PartNotFunc(Part* part) {
    u8 value = !part->inputs[0].value;
    part->outputs[0].value = value;
}

void PartSourceFunc(Part* part) {
    part->outputs[0].value = (u8)part->active;
}

void PartLedFunc(Part* part) {
    part->active = part->inputs[0].value;
}


void PartInfoInit(PartInfo* info) {
    // TODO: Ensure memory are zero
    memset(info, 0, sizeof(PartInfo));
    auto andBlank = info->partBlanks + (u32)PartType::And;
    andBlank->type = PartType::And;
    andBlank->dim = IV2(3, 5);
    andBlank->inputCount = 2;
    andBlank->outputCount = 1;
    andBlank->inputs[0] = CreatePin(andBlank, -1, 1, PinType::Input);
    andBlank->inputs[1] = CreatePin(andBlank, -1, 3, PinType::Input);
    andBlank->outputs[0] = CreatePin(andBlank, 3, 3, PinType::Output);
    andBlank->activeColor = V4(0.4f, 0.6f, 0.0f, 1.0f);
    andBlank->inactiveColor = V4(0.4f, 0.6f, 0.0f, 1.0f);

    auto orBlank = info->partBlanks + (u32)PartType::Or;
    orBlank->type = PartType::Or;
    orBlank->dim = IV2(3, 5);
    orBlank->inputCount = 2;
    orBlank->outputCount = 1;
    orBlank->inputs[0] = CreatePin(orBlank, -1, 1, PinType::Input);
    orBlank->inputs[1] = CreatePin(orBlank, -1, 3, PinType::Input);
    orBlank->outputs[0] = CreatePin(orBlank, 3, 3, PinType::Output);
    orBlank->activeColor = V4(0.0f, 0.0f, 0.6f, 1.0f);
    orBlank->inactiveColor = V4(0.0f, 0.0f, 0.6f, 1.0f);

    auto notBlank = info->partBlanks + (u32)PartType::Not;
    notBlank->type = PartType::Not;
    notBlank->dim = IV2(3, 3);
    notBlank->inputCount = 1;
    notBlank->outputCount = 1;
    notBlank->inputs[0] = CreatePin(notBlank, -1, 1, PinType::Input);
    notBlank->outputs[0] = CreatePin(notBlank, 3, 1, PinType::Output);
    notBlank->activeColor = V4(0.6f, 0.0f, 0.0f, 1.0f);
    notBlank->inactiveColor = V4(0.6f, 0.0f, 0.0f, 1.0f);

    auto ledBlank = info->partBlanks + (u32)PartType::Led;
    ledBlank->type = PartType::Led;
    ledBlank->dim = IV2(3, 3);
    ledBlank->inputCount = 1;
    ledBlank->inputs[0] = CreatePin(ledBlank, -1, 1, PinType::Input);
    ledBlank->activeColor = V4(0.9f, 0.0f, 0.0f, 1.0f);
    ledBlank->inactiveColor = V4(0.1f, 0.1f, 0.1f, 1.0f);

    auto sourceBlank = info->partBlanks + (u32)PartType::Source;
    sourceBlank->type = PartType::Source;
    sourceBlank->dim = IV2(3, 3);
    sourceBlank->outputCount = 1;
    sourceBlank->outputs[0] = CreatePin(sourceBlank, 3, 1, PinType::Output);
    sourceBlank->activeColor = V4(0.7f, 0.7f, 0.0f, 1.0f);
    sourceBlank->inactiveColor = V4(0.0f, 0.0f, 0.5f, 1.0f);

    info->partFunctions[(u32)PartType::And] = PartAndFunc;
    info->partFunctions[(u32)PartType::Or] = PartOrFunc;
    info->partFunctions[(u32)PartType::Not] = PartNotFunc;
    info->partFunctions[(u32)PartType::Source] = PartSourceFunc;
    info->partFunctions[(u32)PartType::Led] = PartLedFunc;
}

PartID GetPartID(PartInfo* info) {
    PartID id = { ++info->partSerialCount };
    return id;
}

void InitPart(PartInfo* info, Part* part, iv2 p, PartType type) {
    *part = info->partBlanks[(u32)type];
    part->id = GetPartID(info);
    part->p = DeskPositionNormalize(MakeDeskPosition(p));
    for (u32 i = 0; i < (array_count(part->inputs) + array_count(part->outputs)); i++) {
        Pin* pin = part->pins + i;
        pin->part = part;
    }
}

void PartProcessSignals(PartInfo* info, Part* part) {
    assert((u32)part->type < (u32)PartType::_Count);
    auto partFn = info->partFunctions[(u32)part->type];
    assert(partFn);
    partFn(part);
}

Pin CreatePin(Part* part, i32 xRel, i32 yRel, PinType type) {
    Pin pin {};
    pin.type = type;
    pin.part = part;
    pin.pRelative = IV2(xRel, yRel);
    return pin;
}

Wire* WirePins(Desk* desk, Pin* pin0, Pin* pin1) {
    Wire* result = nullptr;
    Wire* wire = desk->wires.PushBack();
    if (wire) {
        pin0->wire = wire;
        pin1->wire = wire;
        wire->pin0 = pin0;
        wire->p0 = ComputePinPosition(pin0);
        wire->pin1 = pin1;
        wire->p1 = ComputePinPosition(pin1);
        result = wire;
    }
    return result;
}

Part* GetPrefabPart(PartInfo* info, PartType type) {
    assert((u32)type < (u32)PartType::_Count);
    Part* result = info->partBlanks + (u32)type;
    return result;
}
