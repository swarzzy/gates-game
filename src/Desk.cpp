#include "Desk.h"

u32 ElementHash(void* arg) {
    ElementID* id = (ElementID*)arg;
    // TODO: Actual hashing
    return id->id;
}

bool ElementCompare(void* a, void* b) {
    ElementID* key1 = (ElementID*)a;
    ElementID* key2 = (ElementID*)b;
    bool result = key1->id == key2->id;
    return result;
}

ElementID GetElementID(Desk* desk) {
    ElementID id = { ++desk->elementSerialCount };
    return id;
}

u32 DeskHash(void* arg) {
    iv2* key = (iv2*)arg;
    // TODO: Reasonable hashing
    u32 hash = key->x * 12342 + key->y * 23423;
    return hash;
}

bool DeskCompare(void* a, void* b) {
    iv2* key1 = (iv2*)a;
    iv2* key2 = (iv2*)b;
    bool result = (key1->x == key2->x) && (key1->y == key2->y);
    return result;
}

ElementPin CreateElementPin(Desk* desk, iv2 relP, PinType type) {
    ElementPin pin {};
    pin.type = type;
    pin.relativeP = relP;
    pin.uid = GetPinUID(desk);
    return pin;
}

bool CanPlaceElement(Desk* desk, iv2 p, iv2 dim) {
    for (i32 y = 0; y < dim.y; y++) {
        for (i32 x = 0; x < dim.x; x++) {
            // TODO: Optimize. Here is a lot of unnecessary hash lookups!
            iv2 testP = IV2(p.x + x, p.y + y);
            DeskCell* cell = GetDeskCell(desk, testP, false);
            if (cell) {
                if (cell->element.id) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool ExpandDeskFor(Desk* desk, iv2 p, iv2 dim) {
    bool result = true;

    iv2 points[] = {
        p,
        IV2(p.x + dim.x, p.y),
        p + dim,
        IV2(p.x, p.y + dim.y)
    };

    for (i32 i = 0; i < array_count(points); i++) {
        TilePosition tileP = DeskPositionToTile(points[i]);
        DeskTile* tile = GetDeskTile(desk, tileP.tile, true);
        if (!tile) {
            result = false;
            break;
        }
    }
    return result;
}

bool TryRegisterElementPlacement(Desk* desk, Element* element) {
    bool result = false;
    if (CanPlaceElement(desk, element->p.cell, element->dim)) {
        if (ExpandDeskFor(desk, element->p.cell, element->dim)) {
            for (i32 y = 0; y < element->dim.y; y++) {
                for (i32 x = 0; x < element->dim.x; x++) {
                    // TODO: Optimize. Here is a lot of unnecessary hash lookups!
                    iv2 testP = IV2(element->p.cell.x + x, element->p.cell.y + y);
                    DeskCell* cell = GetDeskCell(desk, testP, false);
                    assert(cell);
                    assert(!cell->element.id);
                    cell->element = element->id;
                }
            }
            result = true;
        }
    }
    return result;
}

void UnregisterElementPlcement(Desk* desk, Element* element) {
    for (i32 y = 0; y < element->dim.y; y++) {
        for (i32 x = 0; x < element->dim.x; x++) {
            // TODO: Optimize. Here is a lot of unnecessary hash lookups!
            iv2 testP = IV2(element->p.cell.x + x, element->p.cell.y + y);
            DeskCell* cell = GetDeskCell(desk, testP, false);
            assert(cell->element.id == element->id.id);
            cell->element = InvalidElementID;
        }
    }
}

void InitElement(Desk* desk, Element* element, iv2 p, ElementType type) {
    //memset(element, 0, sizeof(Element));
    ElementID id = GetElementID(desk);
    element->id = id;

    element->p = MakeDeskPosition(p);
    element->type = type;

    switch (element->type) {
    case ElementType::And: {
        element->color = V4(0.4f, 0.6f, 0.0f, 1.0f);
        element->dim = IV2(2, 3);
        element->inputs[0] = CreateElementPin(desk, IV2(0, 1), PinType::Input);
        element->inputs[1] = CreateElementPin(desk, IV2(0, 2), PinType::Input);
        element->outputs[0] = CreateElementPin(desk, IV2(2, 2), PinType::Output);
        element->inputCount = 2;
        element->outputCount = 1;
    } break;
    case ElementType::Or: {
        element->color = V4(0.0f, 0.0f, 0.6f, 1.0f);
        element->dim = IV2(2, 3);
        element->inputs[0] = CreateElementPin(desk, IV2(0, 1), PinType::Input);
        element->inputs[1] = CreateElementPin(desk, IV2(0, 2), PinType::Input);
        element->outputs[0] = CreateElementPin(desk, IV2(2, 2), PinType::Output);
        element->inputCount = 2;
        element->outputCount = 1;
    } break;
    case ElementType::Not: {
        element->color = V4(0.6f, 0.0f, 0.0f, 1.0f);
        element->dim = IV2(2, 2);
        element->inputs[0] = CreateElementPin(desk, IV2(0, 1), PinType::Input);
        element->outputs[0] = CreateElementPin(desk, IV2(2, 1), PinType::Output);
        element->inputCount = 1;
        element->outputCount = 1;
    } break;
        invalid_default();
    }
}

bool AddElement(Desk* desk, Element* element) {
    bool result = false;
    if (CanPlaceElement(desk, element->p.cell, element->dim)) {
        if (ExpandDeskFor(desk, element->p.cell, element->dim)) {
            Element** entry = HashMapAdd(&desk->elementsHashMap, &element->id);
            if (entry) {
                if (TryRegisterElementPlacement(desk, element)) {
                    *entry = element;
                    result = true;
                } else {
                    unreachable();
                }
            }
        }
    }

    return result;
}

Element* CreateElement(Desk* desk, iv2 p, ElementType type) {
    Element* result = nullptr;
    Element* element = (Element*)desk->deskAllocator.Alloc(sizeof(Element), true);
    if (element) {
        InitElement(desk, element, p, type);
        if (AddElement(desk, element)) {
            result = element;
        }
    }

    return result;
}

void InitDesk(Desk* desk, PlatformHeap* deskHeap) {
    desk->deskHeap = deskHeap;
    desk->deskAllocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, deskHeap);
    desk->tileHashMap = HashMap<iv2, DeskTile*, DeskHash, DeskCompare>::Make(desk->deskAllocator);
    desk->elementsHashMap = HashMap<ElementID, Element*, ElementHash, ElementCompare>::Make(desk->deskAllocator);
}

Element* FindElement(Desk* desk, ElementID id) {
    Element* result = nullptr;
    Element** bucket = HashMapGet(&desk->elementsHashMap, &id);
    if (bucket) {
        result = *bucket;
    }
    return result;
}

DeskTile* CreateDeskTile(Desk* desk, iv2 p) {
    DeskTile* result = (DeskTile*)desk->deskAllocator.Alloc(sizeof(DeskTile), true);
    if (result) {
        result->deskAllocator = &desk->deskAllocator;
        result->p = p;
    }
    return result;
}

DeskTile* GetDeskTile(Desk* desk, iv2 tileP, bool create) {
    DeskTile* result = nullptr;
    DeskTile** bucket = HashMapGet(&desk->tileHashMap, &tileP);
    if (bucket) {
        assert(*bucket);
        result = *bucket;
    } else if (create) {
        DeskTile** newBucket = HashMapAdd(&desk->tileHashMap, &tileP);
        if (newBucket) {
            DeskTile* newTile = CreateDeskTile(desk, tileP);
            if (newTile) {
                *newBucket = newTile;
                result = newTile;
            } else {
                bool deleted = HashMapDelete(&desk->tileHashMap, &tileP);
                assert(deleted);
            }
        }
    }
    return result;
}

DeskCell* GetDeskCell(DeskTile* tile, uv2 cell) {
    assert(cell.x < DeskTileSize && cell.y < DeskTileSize);
    DeskCell* result = tile->cells + (cell.y * DeskTileSize + cell.x);
    return result;
}

DeskCell* GetDeskCell(Desk* desk, iv2 p, bool create) {
    DeskCell* result = nullptr;
    TilePosition tileP = DeskPositionToTile(p);
    DeskTile* tile = GetDeskTile(desk, tileP.tile, create);
    if (tile) {
        result = GetDeskCell(tile, tileP.cell);
    }
    return result;
}

void DrawDesk(Desk* desk, Canvas* canvas) {
    ForEach(&desk->elementsHashMap, [&](auto it) {
        DrawElement(desk, canvas, *it, 1.0f);
    });
}

void DrawElement(Desk* desk, Canvas* canvas, Element* element, f32 alpha) {
    DeskPosition maxP = MakeDeskPosition(element->p.cell + element->dim);
    v2 min = DeskPositionRelative(desk->origin, MakeDeskPosition(element->p.cell));
    v2 max = DeskPositionRelative(desk->origin, maxP);
    DrawListPushRect(&canvas->drawList, min, max, 0.0f, V4(element->color.xyz, alpha));
    for (u32 pinIndex = 0; pinIndex < element->inputCount; pinIndex++) {
        ElementPin* pin = element->inputs + pinIndex;
        v4 color = GetPinColor(pin->type);
        v2 pinMin = V2(pin->relativeP) * DeskCellSize + min - V2(0.1);
        v2 pinMax = V2(pin->relativeP) * DeskCellSize + min + V2(0.1);
        DrawListPushRect(&canvas->drawList, pinMin, pinMax, 0.0f, color);
    }
    for (u32 pinIndex = 0; pinIndex < element->outputCount; pinIndex++) {
        ElementPin* pin = element->outputs + pinIndex;
        v4 color = V4(GetPinColor(pin->type).xyz, alpha);
        v2 pinMin = V2(pin->relativeP) * DeskCellSize + min - V2(0.1);
        v2 pinMax = V2(pin->relativeP) * DeskCellSize + min + V2(0.1);
        DrawListPushRect(&canvas->drawList, pinMin, pinMax, 0.0f, color);
    }
}
