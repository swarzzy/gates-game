#pragma once

#include "Position.h"
#include "HashMap.h"
#include "Common.h"
#include "PartInfo.h"
#include "List.h"
#include "Part.h"
#include "Tools.h"

struct Desk;
struct Wire;
struct Part;
struct Pin;

struct DeskTile {
    iv2 p;
    DArray<Part*> parts;
};

u32 DeskHash(void* arg);
bool DeskCompare(void* a, void* b);

struct Desk {
    DeskPosition origin;
    u32 nodeSerialCount;
    u32 pinGeneration;
    PlatformHeap* deskHeap;
    Allocator deskAllocator;
    Canvas canvas;
    PartInfo* partInfo;
    HashMap<iv2, DeskTile*, DeskHash, DeskCompare> tileHashMap;
    List<Part> parts;
    List<Wire> wires;
    DArray<DeskPosition> wireNodeCleanerBuffer;
    ToolManager toolManager;
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

Desk* CreateDesk();
void DestroyDesk();

Part* GetPartMemory(Desk* desk);

// Desk spatial representation API
//DeskCell* GetDeskCell(Desk* desk, iv2 p, bool create = false);
//DeskCell* GetDeskCell(DeskTile* tile, uv2 cell);
//bool TryChangePartLocation(Desk* desk, Part* part, iv2 newP);
//bool CanPlacePart(Desk* desk, IRect box, Part* self = nullptr);

// New API
// TODO: Maybe put search for wires hwre too
DeskEntity GetAnyAt(Desk* desk, DeskPosition p);
Part* GetPartAt(Desk* desk, DeskPosition p);
Pin* GetPinAt(Desk* desk, DeskPosition p);

bool CheckCollisions(Desk* desk, DeskPosition min, DeskPosition max, ArrayRef<Part*> ignoreParts = ArrayRef<Part*>::Empty());
bool CheckCollisions(Desk* desk, Part* part, ArrayRef<Part*> ignoreParts = ArrayRef<Part*>::Empty());

// TODO: Add a way to specify whether test rect should fully enclose colliders or just touch
void GetCollisions(Desk* desk, DeskPosition min, DeskPosition max, DArray<Part*>* result, ArrayRef<Part*> ignoreParts = ArrayRef<Part*>::Empty());

//bool CheckCollisions(Desk* desk, Part* part, bool selfCollide = true);
void RegisterPart(Desk* desk, Part* part);
void UnregisterPart(Desk* desk, Part* part);
void ChangePartLocation(Desk* desk, Part* part, DeskPosition pNew);

DeskTile* GetDeskTile(Desk* desk, iv2 tile, bool create = false);

DeskTile* CreateDeskTile(Desk* desk, iv2 p);

void ExpandDeskFor(Desk* desk, IRect box);

void DrawDesk(Desk* desk, Canvas* canvas);

void PropagateSignals(Desk* desk);

Wire* AddWire(Desk* desk);
void RemoveWire(Desk* desk, Wire* wire);

// Position is desk-relative
GetWireAtResult GetWireAt(Desk* desk, v2 p);
