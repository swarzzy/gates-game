#pragma once

struct Desk;
struct Wire;
struct Part;

#define InvalidPartID (PartID {0})

struct PartID {
    // TODO: 64 bit or freelist?
    u32 id;
};

enum struct PartType : u32 {
    Unknown = 0, And, Or, Not, Led, Source, _Count
};

struct NodeID {
    u32 id;
};

enum struct PinType {
    Input, Output
};

struct Pin {
    PinType type;
    // TODO: Do we need pointer to the part here?
    Part* part;
    iv2 pRelative;
    u8 value;
    Wire* firstWire;
};

struct Part {
    u32 id;
    PartType type;

    DeskPosition p;
    iv2 dim;

    b32 active;

    u32 inputCount;
    u32 outputCount;

    Pin* pins;

    v4 activeColor;
    v4 inactiveColor;
};

typedef void(PartFunctionFn)(Part* part);
typedef void(PartInitializerFn)(Desk* desk, Part* part);

struct PartInfo {
    u32 partSerialCount;
    PartFunctionFn* partFunctions[PartType::_Count];
    PartInitializerFn* partInitializers[PartType::_Count];
};

Pin* GetInput(Part* part, u32 index);
Pin* GetOutput(Part* part, u32 index);
u32 PinCount(Part* part);

Wire* TryWirePins(Desk* desk, Pin* input, Pin* output);

void PartInfoInit(PartInfo* info);

void InitPart(PartInfo* info, Desk* desk, Part* part, iv2 p, PartType type);

Pin CreatePin(Desk* desk, Part* part, i32 xRel, i32 yRel, PinType type);

inline v4 GetPartColor(Part* element) { return element->active ? element->activeColor : element->inactiveColor; }

void PartGatherSignals(Desk* desk, Part* part);
void PartProcessSignals(PartInfo* info, Part* part);
void PartPropagateSignals(Desk* desk, Part* part);
