#pragma once

#include "Position.h"
#include "HashMap.h"
#include "Common.h"
#include "PartInfo.h"
#include "List.h"
#include "Part.h"

struct Desk;
struct Wire;
struct Part;
struct Pin;

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
    Array<Part*> parts;
};

u32 DeskHash(void* arg);
bool DeskCompare(void* a, void* b);

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
    Array<DeskPosition> wireNodeCleanerBuffer;
};

struct GetWireAtResult {
    Wire* wire;
    u32 nodeIndex;
};

enum struct DeskEntityType {
    None = 0, Part, Pin
};

struct DeskEntity {
    DeskEntityType type;
    union {
        Part* part;
        Pin* pin;
    };
};

void InitDesk(Desk* desk, Canvas* canvas, PartInfo* partInfo, PlatformHeap* deskHeap);

Part* GetPartMemory(Desk* desk);
bool AddPartToDesk(Desk* desk, Part* part);

// Desk spatial representation API
DeskCell* GetDeskCell(Desk* desk, iv2 p, bool create = false);
DeskCell* GetDeskCell(DeskTile* tile, uv2 cell);
bool TryChangePartLocation(Desk* desk, Part* part, iv2 newP);
bool CanPlacePart(Desk* desk, IRect box, Part* self = nullptr);

// New API
DeskEntity GetAnyAt(Desk* desk, DeskPosition p);
Part* GetPartAt(Desk* desk, DeskPosition p);
Pin* GetPinAt(Desk* desk, DeskPosition p);

bool CheckCollisions(Desk* desk, Part* part);
void RegisterPart(Desk* desk, Part* part);
void UnregisterPart(Desk* desk, Part* part);


DeskTile* GetDeskTile(Desk* desk, iv2 tileP, bool create = false);

DeskTile* CreateDeskTile(Desk* desk, iv2 p);

void ExpandDeskFor(Desk* desk, IRect box);

void DrawDesk(Desk* desk, Canvas* canvas);

void PropagateSignals(Desk* desk);

Wire* AddWire(Desk* desk);
void RemoveWire(Desk* desk, Wire* wire);

// Position is desk-relative
GetWireAtResult GetWireAt(Desk* desk, v2 p);
