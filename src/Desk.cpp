#include "Desk.h"

u32 PartHash(void* arg) {
    PartID* id = (PartID*)arg;
    // TODO: Actual hashing
    return id->id;
}

bool PartCompare(void* a, void* b) {
    PartID* key1 = (PartID*)a;
    PartID* key2 = (PartID*)b;
    bool result = key1->id == key2->id;
    return result;
}

u32 NodeHash(void* arg) {
    PartID* id = (PartID*)arg;
    // TODO: Actual hashing
    return id->id;
}

bool NodeCompare(void* a, void* b) {
    NodeID* key1 = (NodeID*)a;
    NodeID* key2 = (NodeID*)b;
    bool result = key1->id == key2->id;
    return result;
}

NodeID GetNodeID(Desk* desk) {
    NodeID id = { ++desk->nodeSerialCount };
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

AddNodeResult AddNode(Desk* desk) {
    AddNodeResult result {};
    NodeID id = GetNodeID(desk);
    Node* entry = HashMapAdd(&desk->nodeTable, &id);
    if (entry) {
        memset(entry, 0, sizeof(Node));
        result.id = id;
        result.node = entry;
    }
    return result;
}

Node* FindNode(Desk* desk, NodeID id) {
    Node* result = HashMapGet(&desk->nodeTable, &id);
    return result;
}

bool CanPlacePart(Desk* desk, IRect box) {
    for (i32 y = box.min.y; y < box.max.y; y++) {
        for (i32 x = box.min.x; x < box.max.x; x++) {
            // TODO: Optimize. Here is a lot of unnecessary hash lookups!
            iv2 testP = IV2(x, y);
            DeskCell* cell = GetDeskCell(desk, testP, false);
            if (cell) {
                if (cell->value != CellValue::Empty) {
                    return false;
                }
            }
        }
    }
    return true;
}

IRect CalcPartBoundingBox(Part* part) {
    IRect rect {};
    rect.min = part->p.cell;
    rect.max = part->p.cell + part->dim;
    for (u32 i = 0; i < part->inputCount; i++) {
        Pin* pin = part->inputs + i;
        iv2 pinP = part->p.cell + pin->pRelative;
        rect.min = IV2(Min(rect.min.x, pinP.x), Min(rect.min.y, pinP.y));
        rect.max = IV2(Max(rect.max.x, pinP.x), Max(rect.max.y, pinP.y));
    }
    for (u32 i = 0; i < part->outputCount; i++) {
        Pin* pin = part->outputs + i;
        iv2 pinP = part->p.cell + pin->pRelative;
        rect.min = IV2(Min(rect.min.x, pinP.x), Min(rect.min.y, pinP.y));
        rect.max = IV2(Max(rect.max.x, pinP.x), Max(rect.max.y, pinP.y));
    }
    return rect;
}


bool ExpandDeskFor(Desk* desk, IRect box) {
    bool result = true;

    iv2 points[] = {
        box.min,
        IV2(box.max.x, box.min.y),
        box.max,
        IV2(box.min.x, box.max.y)
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

bool TryRegisterPartPlacement(Desk* desk, Part* part) {
    bool result = false;
    IRect boundingBox = CalcPartBoundingBox(part);
    if (CanPlacePart(desk, boundingBox)) {
        if (ExpandDeskFor(desk, boundingBox)) {
            // Register part body
            for (i32 y = 0; y < part->dim.y; y++) {
                for (i32 x = 0; x < part->dim.x; x++) {
                    // TODO: Optimize. Here is a lot of unnecessary hash lookups!
                    iv2 testP = IV2(part->p.cell.x + x, part->p.cell.y + y);
                    DeskCell* cell = GetDeskCell(desk, testP, false);
                    assert(cell);
                    assert(cell->value == CellValue::Empty);
                    cell->value = CellValue::Part;
                    cell->part = part;
                }
            }
            // Register part inputs
            for (u32 i = 0; i < part->inputCount; i++) {
                Pin* pin = part->inputs + i;
                iv2 pinP = part->p.cell + pin->pRelative;
                DeskCell* cell = GetDeskCell(desk, pinP, false);
                assert(cell);
                assert(cell->value == CellValue::Empty);
                cell->value = CellValue::Pin;
                cell->pin = pin;
            }
            // Register part outputs
            for (u32 i = 0; i < part->outputCount; i++) {
                Pin* pin = part->outputs + i;
                iv2 pinP = part->p.cell + pin->pRelative;
                DeskCell* cell = GetDeskCell(desk, pinP, false);
                assert(cell);
                assert(cell->value == CellValue::Empty);
                cell->value = CellValue::Pin;
                cell->pin = pin;
            }
            result = true;
        }
    }
    return result;
}

void UnregisterPartPlcement(Desk* desk, Part* part) {
    // Register part body
    for (i32 y = 0; y < part->dim.y; y++) {
        for (i32 x = 0; x < part->dim.x; x++) {
            // TODO: Optimize. Here is a lot of unnecessary hash lookups!
            iv2 testP = IV2(part->p.cell.x + x, part->p.cell.y + y);
            DeskCell* cell = GetDeskCell(desk, testP, false);
            assert(cell);
            assert(cell->value == CellValue::Part && cell->part == part);
            cell->value = CellValue::Empty;
            cell->part = nullptr;
        }
    }
    // Register part inputs
    for (u32 i = 0; i < part->inputCount; i++) {
        Pin* pin = part->inputs + i;
        iv2 pinP = part->p.cell + pin->pRelative;
        DeskCell* cell = GetDeskCell(desk, pinP, false);
        assert(cell);
        assert(cell->value == CellValue::Pin && cell->pin == pin);
        cell->value = CellValue::Empty;
        cell->pin = nullptr;
    }
    // Register part outputs
    for (u32 i = 0; i < part->outputCount; i++) {
        Pin* pin = part->outputs + i;
        iv2 pinP = part->p.cell + pin->pRelative;
        DeskCell* cell = GetDeskCell(desk, pinP, false);
        assert(cell);
        assert(cell->value == CellValue::Pin && cell->pin == pin);
        cell->value = CellValue::Empty;
        cell->pin = nullptr;
    }
}


bool AddPart(Desk* desk, Part* element) {
    bool result = false;
    IRect boundingBox = CalcPartBoundingBox(element);
    if (CanPlacePart(desk, boundingBox)) {
        if (ExpandDeskFor(desk, boundingBox)) {
            Part** entry = HashMapAdd(&desk->partsHashMap, &element->id);
            if (entry) {
                if (TryRegisterPartPlacement(desk, element)) {
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

Part* CreatePart(Desk* desk, PartInfo* info, iv2 p, PartType type) {
    Part* result = nullptr;
    Part* element = (Part*)desk->deskAllocator.Alloc(sizeof(Part), true);
    if (element) {
        InitPart(info, element, p, type);
        if (AddPart(desk, element)) {
            result = element;
        }
    }

    return result;
}

void InitDesk(Desk* desk, Canvas* canvas, PartInfo* partInfo, PlatformHeap* deskHeap) {
    desk->deskHeap = deskHeap;
    desk->deskAllocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, deskHeap);
    desk->tileHashMap = HashMap<iv2, DeskTile*, DeskHash, DeskCompare>::Make(desk->deskAllocator);
    desk->partsHashMap = HashMap<PartID, Part*, PartHash, PartCompare>::Make(desk->deskAllocator);
    desk->nodeTable = HashMap<NodeID, Node, NodeHash, NodeCompare>::Make(desk->deskAllocator);
    desk->wires.Init(desk->deskAllocator);
    desk->canvas = canvas;
    desk->partInfo = partInfo;
}

Part* FindPart(Desk* desk, PartID id) {
    Part* result = nullptr;
    Part** bucket = HashMapGet(&desk->partsHashMap, &id);
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
    ForEach(&desk->partsHashMap, [&](auto it) {
        DrawPart(desk, canvas, *it, 1.0f);
    });
}

DeskPosition ComputePinPosition(Pin* pin,  DeskPosition partPosition) {
    DeskPosition result {};
    iv2 cell = partPosition.cell + pin->pRelative;
    switch (pin->type) {
    case PinType::Input: { result = MakeDeskPosition(cell, V2(DeskCellHalfSize, 0.0f)); } break;
    case PinType::Output: { result = MakeDeskPosition(cell, V2(-DeskCellHalfSize, 0.0f)); } break;
        invalid_default();
    }
    return result;
}

DeskPosition ComputePinPosition(Pin* pin) {
    return ComputePinPosition(pin, pin->part->p);
}

void DrawPart(Desk* desk, Canvas* canvas, Part* element, DeskPosition overridePos, f32 alpha) {
    DeskPosition maxP = MakeDeskPosition(overridePos.cell + element->dim);
    v2 min = DeskPositionRelative(desk->origin, MakeDeskPosition(overridePos.cell)) - DeskCellHalfSize;
    v2 max = DeskPositionRelative(desk->origin, maxP) - DeskCellHalfSize;
    v4 color = V4(GetPartColor(element).xyz, alpha);
    DrawListPushRect(&canvas->drawList, min, max, 0.0f, color);
    for (u32 pinIndex = 0; pinIndex < element->inputCount; pinIndex++) {
        Pin* pin = element->inputs + pinIndex;
        v4 color = V4(0.0f, 0.9f, 0.0f, 1.0f);
        DeskPosition pinPos = ComputePinPosition(pin, overridePos);
        v2 relPos = DeskPositionRelative(desk->origin, pinPos);
        v2 pinMin = relPos - V2(0.1);
        v2 pinMax = relPos + V2(0.1);
        DrawListPushRect(&canvas->drawList, pinMin, pinMax, 0.0f, color);
    }
    for (u32 pinIndex = 0; pinIndex < element->outputCount; pinIndex++) {
        Pin* pin = element->outputs + pinIndex;
        v4 color = V4(0.9f, 0.9f, 0.0f, 1.0f);
        DeskPosition pinPos = ComputePinPosition(pin, overridePos);
        v2 relPos = DeskPositionRelative(desk->origin, pinPos);
        v2 pinMin = relPos - V2(0.1);
        v2 pinMax = relPos + V2(0.1);
        DrawListPushRect(&canvas->drawList, pinMin, pinMax, 0.0f, color);
    }
}

void DrawPart(Desk* desk, Canvas* canvas, Part* element, f32 alpha) {
    DrawPart(desk, canvas, element, element->p, alpha);
}

void PropagateSignals(Desk* desk) {
    for (u32 i = 0; i < desk->wires.Count(); i++) {
        Wire* wire = desk->wires.Begin() + i;
        Pin* p0 = wire->pin0;
        Pin* p1 = wire->pin1;
        if (p0->type == PinType::Input && p1->type == PinType::Output) {
            p0->value = p1->value;
        } else if (p1->type == PinType::Input && p0->type == PinType::Output) {
            p1->value = p0->value;
        }
    }
}
