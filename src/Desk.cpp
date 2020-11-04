#include "Desk.h"

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

IRect CalcPartBoundingBox(Part* part, iv2 overridePos) {
    IRect rect {};
    rect.min = overridePos;
    rect.max = overridePos + part->dim;
    for (u32 i = 0; i < PinCount(part); i++) {
        Pin* pin = part->pins + i;
        iv2 pinP = overridePos + pin->pRelative;
        rect.min = IV2(Min(rect.min.x, pinP.x), Min(rect.min.y, pinP.y));
        rect.max = IV2(Max(rect.max.x, pinP.x), Max(rect.max.y, pinP.y));
    }
    return rect;
}

IRect CalcPartBoundingBox(Part* part) {
    return CalcPartBoundingBox(part, part->p.cell);
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
            for (u32 i = 0; i < PinCount(part); i++) {
                Pin* pin = part->pins + i;
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

void UnregisterPartPlacement(Desk* desk, Part* part) {
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
    for (u32 i = 0; i < PinCount(part); i++) {
        Pin* pin = part->pins + i;
        iv2 pinP = part->p.cell + pin->pRelative;
        DeskCell* cell = GetDeskCell(desk, pinP, false);
        assert(cell);
        assert(cell->value == CellValue::Pin && cell->pin == pin);
        cell->value = CellValue::Empty;
        cell->pin = nullptr;
    }
}

void UpdateCachedWirePositions(Part* part) {
    for (u32 i = 0; i < part->wires.Count(); i++) {
        auto record = part->wires.Begin() + i;
        Wire* wire = record->wire;
        Pin* pin = record->pin;
        if (pin->type == PinType::Input) {
            wire->pInput = ComputePinPosition(pin);
            wire = wire->inputNext;
        } else {
            wire->pOutput = ComputePinPosition(pin);
            wire = wire->outputNext;
        }
    }
}

bool TryChangePartLocation(Desk* desk, Part* part, iv2 newP) {
    bool result = false;
    IRect bbox = CalcPartBoundingBox(part, newP);
    if (CanPlacePart(desk, bbox)) {
        if (ExpandDeskFor(desk, bbox)) {
            UnregisterPartPlacement(desk, part);
            part->p = MakeDeskPosition(newP);
            // TODO: TryRegisterPartPlacement also performs checks and desk expansion. Not efficient!
            bool registered = TryRegisterPartPlacement(desk, part);
            assert(registered);
            UpdateCachedWirePositions(part);
            result = true;
        }
    }
    return result;
}

Part* GetPartMemory(Desk* desk) {
    Part* result = ListAdd(&desk->parts);
    return result;
}

void ReleasePartMemory(Desk* desk, Part* part) {
    ListRemove(&desk->parts, part);
}

bool AddPartToDesk(Desk* desk, Part* part) {
    bool result = false;
    IRect boundingBox = CalcPartBoundingBox(part);
    if (TryRegisterPartPlacement(desk, part)) {
        result = true;
    }

    return result;
}

Part* CreatePart(Desk* desk, PartInfo* info, iv2 p, PartType type) {
    Part* result = nullptr;
    // TODO: Check if we can place the part before allocate it!!
    Part* part = GetPartMemory(desk);
    if (part) {
        InitPart(info, desk, part, p, type);
        if (AddPartToDesk(desk, part)) {
            result = part;
        } else {
            ReleasePartMemory(desk, part);
        }
    }

    return result;
}

void UnwirePart(Desk* desk, Part* part) {
    for (u32 i = 0; i < part->wires.Count(); i++) {
        auto record = part->wires.Begin() + i;
        Pin* pin = record->pin;
        Wire* wire = record->wire;
        if (pin->type == PinType::Input) {
            bool unwired = UnwirePin(wire->output, wire);
            assert(unwired);
        } else if (pin->type == PinType::Output) {
            bool unwired = UnwirePin(wire->input, wire);
            assert(unwired);
        } else {
            unreachable();
        }
        ListRemove(&desk->wires, wire);
    }
}

void DestroyPart(Desk* desk, Part* part) {
    UnwirePart(desk, part);
    UnregisterPartPlacement(desk, part);
    DeinitPart(desk, part);
    ReleasePartMemory(desk, part);
}

void InitDesk(Desk* desk, Canvas* canvas, PartInfo* partInfo, PlatformHeap* deskHeap) {
    desk->deskHeap = deskHeap;
    desk->deskAllocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, deskHeap);
    desk->tileHashMap = HashMap<iv2, DeskTile*, DeskHash, DeskCompare>::Make(desk->deskAllocator);
    desk->wires = CreateList<Wire>(desk->deskAllocator);
    desk->parts = CreateList<Part>(desk->deskAllocator);
    desk->canvas = canvas;
    desk->partInfo = partInfo;
}

DeskTile* CreateDeskTile(Desk* desk, iv2 p) {
    DeskTile* result = (DeskTile*)desk->deskAllocator.Alloc(sizeof(DeskTile), true);
    if (result) {
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
    ListForEach(&desk->parts, part) {
        DrawPart(desk, canvas, part, {}, 0.0f, 1.0f);
    } ListEndEach
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

void DrawPart(Desk* desk, Canvas* canvas, Part* element, DeskPosition overridePos, v3 overrideColor, f32 overrideColorFactor, f32 alpha) {
    DeskPosition maxP = MakeDeskPosition(overridePos.cell + element->dim);
    v2 min = DeskPositionRelative(desk->origin, MakeDeskPosition(overridePos.cell)) - DeskCellHalfSize;
    v2 max = DeskPositionRelative(desk->origin, maxP) - DeskCellHalfSize;
    v3 partColor = GetPartColor(element).xyz;
    v4 color = V4(Lerp(partColor, overrideColor, overrideColorFactor), alpha);
    DrawListPushRect(&canvas->drawList, min, max, 0.0f, color);
    for (u32 pinIndex = 0; pinIndex < element->inputCount; pinIndex++) {
        Pin* pin = GetInput(element, pinIndex);
        v4 color = V4(0.0f, 0.9f, 0.0f, 1.0f);
        DeskPosition pinPos = ComputePinPosition(pin, overridePos);
        v2 relPos = DeskPositionRelative(desk->origin, pinPos);
        v2 pinMin = relPos - V2(0.1);
        v2 pinMax = relPos + V2(0.1);
        DrawListPushRect(&canvas->drawList, pinMin, pinMax, 0.0f, color);
    }
    for (u32 pinIndex = 0; pinIndex < element->outputCount; pinIndex++) {
        Pin* pin = GetOutput(element, pinIndex);
        v4 color = V4(0.9f, 0.9f, 0.0f, 1.0f);
        DeskPosition pinPos = ComputePinPosition(pin, overridePos);
        v2 relPos = DeskPositionRelative(desk->origin, pinPos);
        v2 pinMin = relPos - V2(0.1);
        v2 pinMax = relPos + V2(0.1);
        DrawListPushRect(&canvas->drawList, pinMin, pinMax, 0.0f, color);
    }
}

void DrawPart(Desk* desk, Canvas* canvas, Part* element, v3 overrideColor, f32 overrideColorFactor, f32 alpha) {
    DrawPart(desk, canvas, element, element->p, overrideColor, overrideColorFactor, alpha);
}

void PropagateSignals(Desk* desk) {
    ListForEach(&desk->wires, wire) {
        Pin* input = wire->input;
        Pin* output = wire->output;
        input->value = output->value;
    } ListEndEach
}
