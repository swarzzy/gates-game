#pragma once

#include "Position.h"
#include "HashMap.h"
#include "Common.h"
#include "PartInfo.h"

struct Desk;
struct PartID;
struct Wire;

enum struct CellValue : u32 {
    Empty = 0, Part, Pin, Wire
};

struct DeskCell {
    CellValue value;
    union {
        Part* part;
        Pin* pin;
        Wire* wire;
    };
};

struct DeskTile {
    iv2 p;
    Allocator* deskAllocator;
    DeskCell cells[DeskTileSize * DeskTileSize];
};

u32 DeskHash(void* arg);
bool DeskCompare(void* a, void* b);
u32 PartHash(void* arg);
bool PartCompare(void* a, void* b);
u32 NodeHash(void* arg);
bool NodeCompare(void* a, void* b);

struct Node {
    u8 value;
};

struct IRect {
    iv2 min;
    iv2 max;
};

struct Wire {
    u32 index;
    Pin* pin0;
    Pin* pin1;
    DeskPosition p0;
    DeskPosition p1;
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
    // TODO: Just go crazy and ALLOCATE EVERY SINGLE ELEMENT IN THE HEAP.
    // Eventually we will need more appropriate way to store elements
    HashMap<PartID, Part*, PartHash, PartCompare> partsHashMap;
    HashMap<NodeID, Node, NodeHash, NodeCompare> nodeTable;
    GrowableArray<Wire> wires;
};

u32 GetPinUID(Desk* desk) {
    u32 result = desk->pinGeneration++;
    return result;
}

struct AddNodeResult {
    NodeID id;
    Node* node;
};

AddNodeResult AddNode(Desk* desk);
Node* FindNode(Desk* desk, NodeID id);

NodeID GetNodeID(Desk* desk);
void InitDesk(Desk* desk, Canvas* canvas, PartInfo* partInfo, PlatformHeap* deskHeap);
DeskTile* CreateDeskTile(Desk* desk, iv2 p);
Part* CreatePart(Desk* desk, PartInfo* info, iv2 p, PartType type);

bool AddPart(Desk* desk, Part* element);

Part* FindPart(Desk* desk, PartID id);

DeskCell* GetDeskCell(Desk* desk, iv2 p, bool create = false);
DeskCell* GetDeskCell(DeskTile* tile, uv2 cell);
DeskTile* GetDeskTile(Desk* desk, iv2 tileP, bool create = false);

IRect CalcPartBoundingBox(Part* part);
bool TryRegisterPartPlacement(Desk* desk, Part* element);
void UnregisterPartPlcement(Desk* desk, Part* element);
bool CanPlacePart(Desk* desk, IRect box);
bool ExpandDeskFor(Desk* desk, IRect box);

void DrawPart(Desk* desk, Canvas* canvas, Part* element, DeskPosition overridePos, f32 alpha);
void DrawPart(Desk* desk, Canvas* canvas, Part* element, f32 alpha);
void DrawDesk(Desk* desk, Canvas* canvas);

DeskPosition ComputePinPosition(Pin* pin,  DeskPosition partPosition);
DeskPosition ComputePinPosition(Pin* pin);
void PropagateSignals(Desk* desk);
