#pragma once

#include "Common.h"

const f32 DeskCellSize = 0.5f; // Centimeters
const f32 DeskCellHalfSize = DeskCellSize / 2.0f;

const u32 DeskTileBitShift = 4;
const u32 DeskTileBitMask = (1 << DeskTileBitShift) - 1;
const u32 DeskTileSize = 1 << DeskTileBitShift;

const i32 DeskCoordInvalid = I32::Max;

struct TilePosition;

struct DeskPosition {
    iv2 cell = {};
    v2 offset = {};

    explicit DeskPosition() = default;
    explicit DeskPosition(iv2 cellValue, v2 offsetValue);
    explicit DeskPosition(iv2 cellValue);
    explicit DeskPosition(v2 offsetValue);
    explicit DeskPosition(TilePosition p);

    DeskPosition Normalized() const;
    DeskPosition Add(DeskPosition other) const;
    DeskPosition Offset(iv2 offset) const;
    DeskPosition Offset(v2 offset) const;
    v2 Sub(DeskPosition other) const;
    v2 RelativeTo(DeskPosition origin) const;
    v2 ToOffset() const;

    TilePosition ToTilePos() const;

    bool IsValid() const;
};

struct TilePosition {
    iv2 tile = {};
    uv2 cell = {};

    explicit TilePosition() = default;
    explicit TilePosition(DeskPosition p);
    explicit TilePosition(iv2 cell);

    DeskPosition ToDeskPos() const;
};

DeskPosition InvalidDeskPosition();

iv2 TileFromCell(iv2 cell);
uv2 CellInTile(iv2 cell);
TilePosition CellToTilePos(iv2 cell);
