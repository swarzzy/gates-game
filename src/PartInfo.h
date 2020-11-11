#pragma once

#include "Array.h"
#include "Part.h"

struct Desk;

typedef void(PartFunctionFn)(Part* part);
typedef void(PartInitializerFn)(Desk* desk, Part* part);

struct PartInfo {
    u32 partSerialCount;
    PartFunctionFn* partFunctions[PartType::_Count];
    PartInitializerFn* partInitializers[PartType::_Count];
};

bool ArePinsWired(Pin* input, Pin* output);
Wire* TryWirePins(Desk* desk, Pin* input, Pin* output);
bool UnwirePin(Pin* pin, Wire* wire);

void PartInfoInit(PartInfo* info);

Pin CreatePin(Desk* desk, Part* part, i32 xRel, i32 yRel, PinType type);

void DeallocatePins(Desk* desk, Part* part);
