#pragma once

struct Part;
struct Wire;
struct Desk;
struct PartInfo;

enum struct PartType : u32 {
    Unknown = 0, And, Or, Not, Led, Source, _Count
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
};

struct Wire {
    Pin* input;
    Pin* output;

    //DeskPosition pInput;
    //DeskPosition pOutput;

    // Nodes are stored in sorted order
    // 0 - output
    // 1 = input
    Array<DeskPosition> nodes;
};

struct WireRecord {
    Wire* wire;
    Pin* pin;
};

struct Part {
    PartType type;

    DeskPosition p;
    iv2 dim;

    b32 active;

    b32 selected;

    u32 inputCount;
    u32 outputCount;

    Array<Pin> pins;
    Array<WireRecord> wires;

    Box2D boundingBox;
    Box2D partBoundingBox;
    Array<Box2D> pinBoundingBoxes;

    v4 activeColor;
    v4 inactiveColor;

    const char16* label;

    b32 placed;
};

struct IRect {
    iv2 min;
    iv2 max;

    IRect() = default;
    IRect(iv2 Min, iv2 Max) : min(Min), max(Max) {}
};

// There are three stage of part creation for now:
// 1. Get the memory
// 2. Initialize part in that memory
// 3. Place the part onto the desk

// Initialize part memory
void InitPart(PartInfo* info, Desk* desk, Part* part, iv2 p, PartType type);
void DeinitPart(Desk* desk, Part* part);

void UnwirePart(Desk* desk, Part* part);

// Allocate part memory, initialize it and place the part to the desk
Part* CreatePart(Desk* desk, PartInfo* info, iv2 p, PartType type);
void DestroyPart(Desk* desk, Part* part);

void DestroyWire(Desk* desk, Wire* wire);

Pin CreatePin(Part* part, i32 xRel, i32 yRel, PinType type);

void PartProcessSignals(PartInfo* info, Part* part);

Pin* GetInput(Part* part, u32 index);
Pin* GetOutput(Part* part, u32 index);
u32 PinCount(Part* part);

bool ArePinsWired(Pin* input, Pin* output);
Wire* TryWirePins(Desk* desk, Pin* input, Pin* output);
bool UnwirePin(Pin* pin, Wire* wire);

IRect CalcPartBoundingBox(Part* part);
IRect CalcPartBoundingBox(Part* part, iv2 overridePos);

void UpdateCachedWirePositions(Part* part);
void WireCleanupNodes(Wire* wire, Array<DeskPosition>* buffer);

void DrawPart(Desk* desk, Canvas* canvas, Part* element, DeskPosition overridePos, v3 overrideColor, f32 overrideColorFactor, f32 alpha);
void DrawPart(Desk* desk, Canvas* canvas, Part* element, v3 overrideColor, f32 overrideColorFactor, f32 alpha);
void DrawPartBoundingBoxes(Desk* desk, Canvas* canvas, Part* part);

DeskPosition ComputePinPosition(Pin* pin,  DeskPosition partPosition);
DeskPosition ComputePinPosition(Pin* pin);

v4 GetPartColor(Part* element);
