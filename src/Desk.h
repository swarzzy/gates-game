#pragma once

#include "Position.h"
#include "HashMap.h"
#include "Common.h"

struct Desk;

enum struct ElementType {
    And, Or, Not
};

enum struct PinType {
    Input, Output
};

struct ElementPin {
    PinType type;
    u32 uid;
    iv2 relativeP;
};

struct Element {
    DeskPosition p;
    iv2 dim;
    v4 color;
    ElementType type;
    u32 inputCount;
    u32 outputCount;
    ElementPin inputs[8];
    ElementPin outputs[8];
};

struct DeskCell {
    Element* element;
};

struct DeskTile {
    iv2 p;
    Allocator* deskAllocator;
    DeskCell cells[DeskTileSize * DeskTileSize];
};

u32 DeskHash(void* arg);
bool DeskCompare(void* a, void* b);

struct Desk {
    DeskPosition origin;
    u32 pinGeneration;
    PlatformHeap* deskHeap;
    Allocator deskAllocator;
    HashMap<iv2, DeskTile*, DeskHash, DeskCompare> tileHashMap;
    GrowableArray<Element> elements;
};

u32 GetPinUID(Desk* desk) {
    u32 result = desk->pinGeneration++;
    return result;
}

void InitDesk(Desk* desk, PlatformHeap* deskHeap);
DeskTile* CreateDeskTile(Desk* desk, iv2 p);
void InitElement(Desk* desk, Element* element, iv2 p, ElementType type);
Element* CreateElement(Desk* desk, iv2 p, ElementType type);
ElementPin CreateElementPin(Desk* desk, iv2 relP, PinType type);

bool AddElement(Desk* desk, Element* element);

DeskCell* GetDeskCell(Desk* desk, iv2 p, bool create = false);
DeskCell* GetDeskCell(DeskTile* tile, uv2 cell);
DeskTile* GetDeskTile(Desk* desk, iv2 tileP, bool create = false);

bool TryRegisterElementPlacement(Desk* desk, Element* element);
void UnregisterElementPlcement(Desk* desk, Element* element);
bool CanPlaceElement(Desk* desk, iv2 p, iv2 dim);
bool ExpandDeskFor(Desk* desk, iv2 p, iv2 dim);

void DrawElement(Desk* desk, Canvas* canvas, Element* element, f32 alpha);
void DrawDesk(Desk* desk, Canvas* canvas);
