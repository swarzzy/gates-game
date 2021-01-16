#pragma once

#include "Position.h"
#include "HashMap.h"
#include "Common.h"
#include "PartInfo.h"
#include "List.h"
#include "Part.h"
#include "Tools.h"
#include "Serialize.h"

struct Desk;
struct Wire;
struct Part;
struct Pin;

struct DeskTile {
    iv2 p;
    DArray<Part*> parts;
};

struct TileKey {
    iv2 key;
    explicit TileKey(iv2 v) : key(v) {}
};

struct PartID {
    u32 value;
    PartID() = default;
    PartID(u32 v) : value(v) {}
};

u32 HashU32(PartID& key) {
    // TODO: Reasonable hashing
    u32 hash = key.value;
    return hash;
}

bool HashCompareKeys(PartID& a, PartID& b) {
    bool result = a.value == b.value;
    return result;
}

struct Desk {
    u32 partSerialCount;
    DeskPosition origin;
    PlatformHeap* deskHeap;
    Allocator deskAllocator;
    Canvas canvas;
    PartInfo* partInfo;
    HashMap<TileKey, DeskTile*> tileHashMap;
    List<Part> parts;
    List<Wire> wires;
    DArray<DeskPosition> wireNodeCleanerBuffer;
    ToolManager toolManager;

    SerializedPart serializerScratchPart;
    SerializedWire serializerScratchWire;
    JsonSerializer serializer;
    JsonDeserializer deserializer;

    HashMap<PartID, Part*> partsTable;
    HashMap<PartID, u32> idRemappingTable;
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
u32 RegisterPartID(Desk* desk, Part* part);
void UnregisterPartID(Desk* desk, u32 id);

Part* GetPartByID(Desk* desk, u32 id);

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
