#include "Desk.h"

u32 HashU32(TileKey& key) {
    // TODO: Reasonable hashing
    u32 hash = key.key.x * 12342 + key.key.y * 23423;
    return hash;
}

bool HashCompareKeys(TileKey& a, TileKey& b) {
    bool result = (a.key.x == b.key.x) && (a.key.y == b.key.y);
    return result;
}

DeskTile* GetDeskTile(Desk* desk, iv2 p, bool create) {
    DeskTile* result = nullptr;
    DeskTile** bucket = desk->tileHashMap.Find(TileKey(p));
    if (bucket) {
        assert(*bucket);
        result = *bucket;
    } else if (create) {
        DeskTile** newBucket = desk->tileHashMap.Add(TileKey(p));
        if (newBucket) {
            DeskTile* newTile = CreateDeskTile(desk, p);
            if (newTile) {
                *newBucket = newTile;
                result = newTile;
            } else {
                bool deleted = desk->tileHashMap.Delete(TileKey(p));
                assert(deleted);
            }
        }
    }
    return result;
}

IRect ComputeAffectedTileRegion(Part* part) {
    auto boxRel = part->boundingBox;
    iv2 deskMin = part->p.Offset(boxRel.min).cell;
    iv2 deskMax = part->p.Offset(boxRel.max).cell;

    iv2 min = TileFromCell(deskMin);
    iv2 max = TileFromCell(deskMax);

    return IRect(min, max);
}

void UnregisterPart(Desk* desk, Part* part) {
    assert(!part->placed);

    auto region = ComputeAffectedTileRegion(part);

    for (i32 y = region.min.y; y <= region.max.y; y++) {
        for (i32 x = region.min.x; x <= region.max.x; x++) {
            DeskTile* tile = GetDeskTile(desk, IV2(x, y), true);
            assert(tile);
            Part** entry = tile->parts.FindFirst([&part](Part** it) { return (*it) == part; });
            assert(entry);
            tile->parts.EraseUnsorted(entry);
        }
    }

    part->placed = false;
}

void RegisterPart(Desk* desk, Part* part) {
    assert(!part->placed);

    auto region = ComputeAffectedTileRegion(part);

    for (i32 y = region.min.y; y <= region.max.y; y++) {
        for (i32 x = region.min.x; x <= region.max.x; x++) {
            DeskTile* tile = GetDeskTile(desk, IV2(x, y), true);
            assert(tile);
            assert(!tile->parts.FindFirst([&part](Part** it) { return (*it) == part; }));
            tile->parts.PushBack(part);
        }
    }

    part->placed = false;
}

void ChangePartLocation(Desk* desk, Part* part, DeskPosition pNew) {
    UnregisterPart(desk, part);
    part->p = pNew;
    RegisterPart(desk, part);
}

bool CheckCollisions(Desk* desk, Part* part, ArrayRef<Part*> ignoreParts) {
    DeskPosition min = part->p;
    DeskPosition max = part->p.Offset(part->boundingBox.max);
    bool result = CheckCollisions(desk, min, max, ignoreParts);
    return result;
}


bool CheckCollisions(Desk* desk, DeskPosition min, DeskPosition max, ArrayRef<Part*> ignoreParts) {
    bool collided = false;

    iv2 tileMin = TileFromCell(min.cell);
    iv2 tileMax = TileFromCell(max.cell);

    Box2D testBox = Box2D(V2(0.0f), max.RelativeTo(min));

    for (i32 y = tileMin.y; y <= tileMax.y; y++) {
        for (i32 x = tileMin.x; x <= tileMax.x; x++) {
            DeskTile* tile = GetDeskTile(desk, IV2(x, y), true);
            assert(tile);
            ForEach(&tile->parts, it) {
                Part* testPart = *it;
                v2 offset = testPart->p.RelativeTo(min);
                if (BoxesIntersect2D(testBox, testPart->boundingBox.Offset(offset))) {
                    if (!ignoreParts.FindFirst([&testPart](Part** it) { return  ((*it) == testPart); })) {
                        collided = true;
                        break;
                    }
                }
            } EndEach;
        }
    }

    return collided;
}

void GetCollisions(Desk* desk, DeskPosition min, DeskPosition max, DArray<Part*>* result, ArrayRef<Part*> ignoreParts) {
    iv2 tileMin = TileFromCell(min.cell);
    iv2 tileMax = TileFromCell(max.cell);

    Box2D testBox = Box2D(V2(0.0f), max.RelativeTo(min));

    for (i32 y = tileMin.y; y <= tileMax.y; y++) {
        for (i32 x = tileMin.x; x <= tileMax.x; x++) {
            DeskTile* tile = GetDeskTile(desk, IV2(x, y), true);
            assert(tile);
            ForEach(&tile->parts, it) {
                Part* testPart = *it;
                v2 offset = testPart->p.RelativeTo(min);
                if (BoxesIntersect2D(testBox, testPart->boundingBox.Offset(offset))) {
                    if (!ignoreParts.FindFirst([&testPart](Part** it) { return  ((*it) == testPart); })) {
                        result->PushBack(testPart);
                    }
                }
            } EndEach;
        }
    }
}

DeskEntity GetAnyAt(Desk* desk, DeskPosition p) {
    DeskEntity result = {};
    DeskTile* tile = GetDeskTile(desk, TileFromCell(p.cell), false);
    if (tile) {
        ForEach(&tile->parts, it) {
            Part* part = *it;
            v2 pRel = p.RelativeTo(part->p);
            if (PointInBox2D(part->boundingBox, pRel)) {
                ForEach(&part->pinBoundingBoxes, box) {
                    if (PointInBox2D(*box, pRel)) {
                        result.type = DeskEntityType::Pin;
                        assert(part->pinBoundingBoxes.IndexFromPtr(box) < part->pins.Count());
                        result.pin = part->pins.Data() + (part->pinBoundingBoxes.IndexFromPtr(box));
                        return result;
                    }
                } EndEach;

                if (PointInBox2D(part->partBoundingBox, pRel)) {
                    result.type = DeskEntityType::Part;
                    result.part = part;
                    return result;
                }
            }
        } EndEach;
    }
    return result;
}

// TODO: Optimize these two below by making them not a wrappers for generic routine
Part* GetPartAt(Desk* desk, DeskPosition p) {
    Part* result = nullptr;
    auto any = GetAnyAt(desk, p);
    if (any.type == DeskEntityType::Part) {
        result = any.part;
    }
    return result;
}

Pin* GetPinAt(Desk* desk, DeskPosition p) {
    Pin* result = nullptr;
    auto any = GetAnyAt(desk, p);
    if (any.type == DeskEntityType::Pin) {
        result = any.pin;
    }
    return result;
}

Part* GetPartMemory(Desk* desk) {
    Part* result = desk->parts.Add();
    return result;
}

u32 RegisterPartID(Desk* desk, Part* part) {
    u32 id = ++desk->partSerialCount;
    auto entry = desk->partsTable.Add(PartID(id));
    assert(entry);
    *entry = part;
    return id;
}

void UnregisterPartID(Desk* desk, u32 id) {
    desk->partsTable.Delete(PartID(id));
}

Part* GetPartByID(Desk* desk, u32 id) {
    Part* result = nullptr;
    if (id) {
        auto entry = desk->partsTable.Find(PartID(id));
        if (entry) {
            result = *entry;
        }
    }
    return result;
}


void ReleasePartMemory(Desk* desk, Part* part) {
    desk->parts.Remove(part);
}

Desk* CreateDesk() {
    auto context = GetContext();
    assert(!context->desk);

    PlatformHeap* heap = Platform.CreateHeap();
    Allocator allocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, heap);

    Desk* desk = allocator.Alloc<Desk>(true);
    desk->deskAllocator = allocator;
    desk->deskHeap = heap;

    desk->canvas = CreateCanvas(&desk->deskAllocator);
    ToolManagerInit(&desk->toolManager, desk);

    desk->tileHashMap = HashMap<TileKey, DeskTile*>(&desk->deskAllocator);
    desk->wires = List<Wire>(&desk->deskAllocator);
    desk->parts = List<Part>(&desk->deskAllocator);
    desk->partInfo = &context->partInfo;
    desk->wireNodeCleanerBuffer = DArray<DeskPosition>(&desk->deskAllocator);

    desk->serializer.Init(&desk->deskAllocator);
    desk->deserializer.Init(&desk->deskAllocator);
    desk->serializerScratchPart.pinRelPositions = DArray<v2>(&desk->deskAllocator);
    desk->serializerScratchWire.nodes = DArray<DeskPosition>(&desk->deskAllocator);

    desk->partsTable = HashMap<PartID, Part*>(&desk->deskAllocator);
    desk->idRemappingTable = HashMap<PartID, u32>(&desk->deskAllocator);

    context->desk = desk;

    return desk;
}

void DestroyDesk() {
    auto context = GetContext();
    Platform.DestroyHeap(context->desk->deskHeap);
    context->desk = nullptr;
}

DeskTile* CreateDeskTile(Desk* desk, iv2 p) {
    DeskTile* result = (DeskTile*)desk->deskAllocator.Alloc(sizeof(DeskTile), true);
    if (result) {
        result->p = p;
        result->parts = DArray<Part*>(&desk->deskAllocator);
    }
    return result;
}

void DrawDesk(Desk* desk, Canvas* canvas) {
    ListForEach(&desk->parts, part) {
        DrawPart(desk, canvas, part, {}, 0.0f, 1.0f);
    } ListEndEach(part);

    DrawListBeginBatch(&canvas->drawList, TextureMode::Color, 0);
    ListForEach(&desk->parts, part) {
        DrawPartBoundingBoxes(desk, canvas, part);
    } ListEndEach(part);
    DrawListEndBatch(&canvas->drawList);
}

void PropagateSignals(Desk* desk) {
    ListForEach(&desk->wires, wire) {
        Pin* input = wire->input;
        Pin* output = wire->output;
        input->value = output->value;
    } ListEndEach(wire);
}

Wire* AddWire(Desk* desk) {
    Wire* wire = desk->wires.Add();
    wire->nodes = DArray<DeskPosition>(&desk->deskAllocator);
    return wire;
}

void RemoveWire(Desk* desk, Wire* wire) {
    wire->nodes.FreeBuffers();
    desk->wires.Remove(wire);
}

bool CheckWireSegmentHit(v2 mouseP, v2 begin, v2 end, f32 thickness) {
    bool result = false;
    v2 vec = Normalize(end - begin);
    v2 perp = Perp(vec);
    v2 offset = perp * thickness * 0.5f;

    auto rect = MinMax(begin, end);
    rect.min -= offset;
    rect.max += offset;

    if (PointInRectangle2D(mouseP, rect.min, rect.max)) {
        result = true;
    }
    return result;
}

// TODO: Optimize this. Narrow down the traversal subset. We cound for instance
// compute line bounding box and check only inside it.

// Position is desk-relative
GetWireAtResult GetWireAt(Desk* desk, v2 p) {
    GetWireAtResult result {};

    ListForEach(&desk->wires, wire) {
        assert(wire->nodes.Count() >= 2);
        for (u32 i = 1; i < wire->nodes.Count(); i++) {
            DeskPosition* prev = wire->nodes.Data() + (i - 1);
            DeskPosition* curr = wire->nodes.Data() + i;

            v2 begin = prev->RelativeTo(desk->origin);
            v2 end = curr->RelativeTo(desk->origin);

            // TODO: Wire thickness
            f32 thickness = 0.1f;
            if (CheckWireSegmentHit(p, begin, end, thickness)) {
                result.wire = wire;
                result.nodeIndex = wire->nodes.IndexFromPtr(prev);
                break;
            }
        }
    } ListEndEach(wire);

    return result;
}

bool LoadDeskFromFile(JsonDeserializer* deserializer, Desk* desk, const char* filename) {
    bool result = false;
    u32 saveFileSize = Platform.DebugGetFileSize(filename);
    if (saveFileSize) {
        void* saveFileData = desk->deskAllocator.Alloc(saveFileSize + 1, false);
        assert(saveFileData);
        defer { desk->deskAllocator.Dealloc(saveFileData); };

        u32 readSize = Platform.DebugReadTextFile(saveFileData, saveFileSize + 1, filename);
        if (readSize == saveFileSize + 1) {
            char* deskJson = (char*)saveFileData;
            auto deserialized = DeserializeDeskFromJson(deserializer, deskJson, readSize, desk);
            if (deserialized) {
                result = true;
            }
        }
    }
    return result;
}
