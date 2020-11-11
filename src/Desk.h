#pragma once

#include "Position.h"
#include "HashMap.h"
#include "Common.h"
#include "PartInfo.h"
#include "List.h"
#include "Part.h"

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

DeskTile* CreateDeskTile(Desk* desk, iv2 p);
DeskCell* GetDeskCell(Desk* desk, iv2 p, bool create = false);
DeskCell* GetDeskCell(DeskTile* tile, uv2 cell);
DeskTile* GetDeskTile(Desk* desk, iv2 tileP, bool create = false);

bool TryChangePartLocation(Desk* desk, Part* part, iv2 newP);

bool TryRegisterPartPlacement(Desk* desk, Part* element);
void UnregisterPartPlcement(Desk* desk, Part* element);
bool CanPlacePart(Desk* desk, IRect box, Part* self = nullptr);
bool ExpandDeskFor(Desk* desk, IRect box);

void DrawDesk(Desk* desk, Canvas* canvas);

void PropagateSignals(Desk* desk);
