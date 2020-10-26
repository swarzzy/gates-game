#pragma once

struct Desk;

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

struct Pin {
    iv2 pRelative;
    NodeID nodeId;
    u8 value;
};

struct Part {
    PartID id;
    PartType type;

    DeskPosition p;
    iv2 dim;

    b32 active;

    u32 inputCount;
    u32 outputCount;

    Pin inputs[8];
    Pin outputs[8];

    v4 activeColor;
    v4 inactiveColor;
};

typedef void(PartFunctionFn)(Part* part);

struct PartInfo {
    u32 partSerialCount;
    Part partBlanks[PartType::_Count];
    PartFunctionFn* partFunctions[PartType::_Count];
};

void PartInfoInit(PartInfo* info);

void InitPart(PartInfo* info, Part* element, iv2 p, PartType type);

Pin CreatePin(i32 xRel, i32 yRel);

inline v4 GetPartColor(Part* element) { return element->active ? element->activeColor : element->inactiveColor; }

void PartGatherSignals(Desk* desk, Part* part);
void PartProcessSignals(PartInfo* info, Part* part);
void PartPropagateSignals(Desk* desk, Part* part);
