#pragma once

#include "Position.h"
#include "HashMap.h"
#include "Common.h"
#include "PartInfo.h"
#include "List.h"

struct Desk;
struct Wire;

enum struct CellValue : u32 {
    Empty = 0, Part, Pin
};

struct DeskCell {
    CellValue value;
    union {
        Part* part;
        Pin* pin;
    };
};

struct DeskTile {
    iv2 p;
    DeskCell cells[DeskTileSize * DeskTileSize];
};

u32 DeskHash(void* arg);
bool DeskCompare(void* a, void* b);

struct IRect {
    iv2 min;
    iv2 max;
};

struct Wire {
    Pin* input;
    Pin* output;
    DeskPosition pInput;
    DeskPosition pOutput;
    Wire* inputNext;
    Wire* outputNext;
};

struct Desk {
    DeskPosition origin;
    u32 nodeSerialCount;
    u32 pinGeneration;
    PlatformHeap* deskHeap;
    Allocator deskAllocator;
    Canvas* canvas;
    PartInfo* partInfo;
    HashMap<iv2, DeskTile*, DeskHash, DeskCompare> tileHashMap;
    List<Part> parts;
    List<Wire> wires;
};

void InitDesk(Desk* desk, Canvas* canvas, PartInfo* partInfo, PlatformHeap* deskHeap);

Part* GetPartMemory(Desk* desk);
bool AddPartToDesk(Desk* desk, Part* part);
Part* CreatePart(Desk* desk, PartInfo* info, iv2 p, PartType type);
void DestroyPart(Desk* desk, Part* part);

DeskTile* CreateDeskTile(Desk* desk, iv2 p);
DeskCell* GetDeskCell(Desk* desk, iv2 p, bool create = false);
DeskCell* GetDeskCell(DeskTile* tile, uv2 cell);
DeskTile* GetDeskTile(Desk* desk, iv2 tileP, bool create = false);

IRect CalcPartBoundingBox(Part* part);
IRect CalcPartBoundingBox(Part* part, iv2 overridePos);

void UpdateCachedWirePositions(Part* part);
bool TryChangePartLocation(Desk* desk, Part* part, iv2 newP);

bool TryRegisterPartPlacement(Desk* desk, Part* element);
void UnregisterPartPlcement(Desk* desk, Part* element);
bool CanPlacePart(Desk* desk, IRect box, Part* self = nullptr);
bool ExpandDeskFor(Desk* desk, IRect box);

void DrawPart(Desk* desk, Canvas* canvas, Part* element, DeskPosition overridePos, v3 overrideColor, f32 overrideColorFactor, f32 alpha);
void DrawPart(Desk* desk, Canvas* canvas, Part* element, v3 overrideColor, f32 overrideColorFactor, f32 alpha);
void DrawDesk(Desk* desk, Canvas* canvas);

DeskPosition ComputePinPosition(Pin* pin,  DeskPosition partPosition);
DeskPosition ComputePinPosition(Pin* pin);
void PropagateSignals(Desk* desk);
