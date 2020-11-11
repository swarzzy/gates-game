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

bool CanPlacePart(Desk* desk, IRect box, Part* self) {
    for (i32 y = box.min.y; y < box.max.y; y++) {
        for (i32 x = box.min.x; x < box.max.x; x++) {
            // TODO: Optimize. Here is a lot of unnecessary hash lookups!
            iv2 testP = IV2(x, y);
            DeskCell* cell = GetDeskCell(desk, testP, false);
            if (cell) {
                if (cell->value != CellValue::Empty) {
                    if (self && cell->value == CellValue::Part && cell->part == self) continue;
                    if (self && cell->value == CellValue::Pin && cell->pin->part == self) continue;
                    return false;
                }
            }
        }
    }
    return true;
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
        TilePosition tileP = CellToTilePos(points[i]);
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
            ForEach(&part->pins, pin) {
                iv2 pinP = part->p.cell + pin->pRelative;
                DeskCell* cell = GetDeskCell(desk, pinP, false);
                assert(cell);
                assert(cell->value == CellValue::Empty);
                cell->value = CellValue::Pin;
                cell->pin = pin;
            } EndEach;
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
    ForEach(&part->pins, pin) {
        iv2 pinP = part->p.cell + pin->pRelative;
        DeskCell* cell = GetDeskCell(desk, pinP, false);
        assert(cell);
        assert(cell->value == CellValue::Pin && cell->pin == pin);
        cell->value = CellValue::Empty;
        cell->pin = nullptr;
    } EndEach;
}

bool TryChangePartLocation(Desk* desk, Part* part, iv2 newP) {
    bool result = false;
    IRect bbox = CalcPartBoundingBox(part, newP);
    if (CanPlacePart(desk, bbox, part)) {
        if (ExpandDeskFor(desk, bbox)) {
            UnregisterPartPlacement(desk, part);
            part->p = DeskPosition(newP);
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
    Part* result = desk->parts.Add();
    return result;
}

void ReleasePartMemory(Desk* desk, Part* part) {
    desk->parts.Remove(part);
}

bool AddPartToDesk(Desk* desk, Part* part) {
    bool result = false;
    IRect boundingBox = CalcPartBoundingBox(part);
    if (TryRegisterPartPlacement(desk, part)) {
        result = true;
    }

    return result;
}

void InitDesk(Desk* desk, Canvas* canvas, PartInfo* partInfo, PlatformHeap* deskHeap) {
    desk->deskHeap = deskHeap;
    desk->deskAllocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, deskHeap);
    desk->tileHashMap = HashMap<iv2, DeskTile*, DeskHash, DeskCompare>::Make(desk->deskAllocator);
    desk->wires = List<Wire>(&desk->deskAllocator);
    desk->parts = List<Part>(&desk->deskAllocator);
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
    TilePosition tileP = CellToTilePos(p);
    DeskTile* tile = GetDeskTile(desk, tileP.tile, create);
    if (tile) {
        result = GetDeskCell(tile, tileP.cell);
    }
    return result;
}

void DrawDesk(Desk* desk, Canvas* canvas) {
    ListForEach(&desk->parts, part) {
        DrawPart(desk, canvas, part, {}, 0.0f, 1.0f);
    } ListEndEach;
}

void PropagateSignals(Desk* desk) {
    ListForEach(&desk->wires, wire) {
        Pin* input = wire->input;
        Pin* output = wire->output;
        input->value = output->value;
    } ListEndEach;
}
