#pragma once

#include "Common.h"

const f32 DeskCellSize = 0.5f; // Centimeters
const f32 DeskCellHalfSize = DeskCellSize / 2.0f;

const u32 DeskTileBitShift = 4;
const u32 DeskTileBitMask = (1 << DeskTileBitShift) - 1;
const u32 DeskTileSize = 1 << DeskTileBitShift;

struct TilePosition;

struct DeskPosition {
    iv2 cell = {};
    v2 offset = {};

    DeskPosition() = default;
    DeskPosition(iv2 cellValue, v2 offsetValue);
    DeskPosition(iv2 cellValue);
    DeskPosition(v2 offsetValue);
    DeskPosition(TilePosition p);

    DeskPosition Normalized() const;
    DeskPosition Add(DeskPosition other) const;
    DeskPosition Offset(v2 offset) const;
    v2 Sub(DeskPosition other) const;
    v2 RelativeTo(DeskPosition origin) const;

    TilePosition ToTilePos() const;
};

struct TilePosition {
    iv2 tile = {};
    uv2 cell = {};

    TilePosition() = default;
    TilePosition(DeskPosition p);
    TilePosition(iv2 cell);

    DeskPosition ToDeskPos() const;
};

iv2 TileFromCell(iv2 cell);
uv2 CellInTile(iv2 cell);
TilePosition CellToTilePos(iv2 cell);
