#pragma once

#include "Common.h"

const f32 DeskCellSize = 0.5f; // Centimeters
const f32 DeskCellHalfSize = DeskCellSize / 2.0f;

const u32 DeskTileBitShift = 4;
const u32 DeskTileBitMask = (1 << DeskTileBitShift) - 1;
const u32 DeskTileSize = 1 << DeskTileBitShift;

struct DeskPosition {
    iv2 cell;
    v2 offset;
};

struct TilePosition {
    iv2 tile;
    uv2 cell;
};


DeskPosition MakeDeskPosition(iv2 cell, v2 offset) {
    // TODO: Normalize?
    DeskPosition result;
    result.cell = cell;
    result.offset = offset;
    return result;
}

DeskPosition DeskPositionNormalize(DeskPosition p) {
    DeskPosition result;

    // NOTE: We are not checking against integer overflowing
    i32 cellX = (i32)Floor((p.offset.x + DeskCellHalfSize) / DeskCellSize);
    i32 cellY = (i32)Floor((p.offset.y + DeskCellHalfSize) / DeskCellSize);

    result.offset.x = p.offset.x - cellX * DeskCellSize;
    result.offset.y = p.offset.y - cellY * DeskCellSize;

    result.cell.x = p.cell.x + cellX;
    result.cell.y = p.cell.y + cellY;

    return result;
}

DeskPosition DeskPositionAdd(DeskPosition a, DeskPosition b) {
    DeskPosition result = a;
    result.cell += b.cell;
    result.offset += b.offset;
    result = DeskPositionNormalize(result);
    return result;
}

DeskPosition MakeDeskPosition(v2 offset) {
    DeskPosition result = DeskPositionNormalize(MakeDeskPosition({}, offset));
    return result;
}

DeskPosition MakeDeskPosition(iv2 cell) {
    DeskPosition result;
    result.cell = cell;
    result.offset = V2(0.0f);
    return result;
}

DeskPosition DeskPositionOffset(DeskPosition p, v2 offset) {
    DeskPosition result = p;
    result.offset += offset;
    result = DeskPositionNormalize(result);
    return result;
}

v2 DeskPositionDifference(DeskPosition a, DeskPosition b) {
    v2 result;
    iv2 cellDiff = a.cell - b.cell;
    v2 offsetDiff = a.offset - b.offset;
    result = V2(cellDiff) * DeskCellSize + offsetDiff;
    return result;
}

v2 DeskPositionRelative(DeskPosition origin, DeskPosition target) {
    v2 result = DeskPositionDifference(target, origin);
    return result;
}

TilePosition MakeTilePosition(iv2 tile, uv2 cell) {
    TilePosition result;
    result.tile = tile;
    result.cell = cell;
    return result;
}

iv2 TileFromCell(iv2 cell) {
    iv2 result;
    result.x = cell.x >> DeskTileBitShift;
    result.y = cell.y >> DeskTileBitShift;
    return result;
}

uv2 CellInTile(iv2 cell) {
    uv2 result;
    result.x = cell.x & DeskTileBitMask;
    result.y = cell.y & DeskTileBitMask;
    return result;
}

DeskPosition TilePositionToDesk(TilePosition p) {
    DeskPosition result;
    result.offset = {};
    result.cell.x = (p.tile.x << DeskTileBitShift) | p.cell.x;
    result.cell.y = (p.tile.y << DeskTileBitShift) | p.cell.y;
    return result;
}

TilePosition DeskPositionToTile(iv2 cell) {
    TilePosition result;
    result.tile = TileFromCell(cell);
    result.cell = CellInTile(cell);
    return result;
}
