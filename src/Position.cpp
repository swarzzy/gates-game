#include "Position.h"

DeskPosition::DeskPosition(iv2 cellValue, v2 offsetValue) : cell(cellValue), offset(offsetValue) {
    *this = Normalized();
}

DeskPosition::DeskPosition(iv2 cellValue) : cell(cellValue), offset({}) {}

DeskPosition::DeskPosition(v2 offsetValue) : cell({}), offset(offsetValue) {
    *this = Normalized();
}

DeskPosition::DeskPosition(TilePosition p) { unreachable(); }


DeskPosition DeskPosition::Normalized() const {
    DeskPosition result;

    // NOTE: We are not checking against integer overflowing
    i32 cellX = (i32)Floor((offset.x + DeskCellHalfSize) / DeskCellSize);
    i32 cellY = (i32)Floor((offset.y + DeskCellHalfSize) / DeskCellSize);

    result.offset.x = offset.x - cellX * DeskCellSize;
    result.offset.y = offset.y - cellY * DeskCellSize;

    result.cell.x = cell.x + cellX;
    result.cell.y = cell.y + cellY;

    return result;
}


DeskPosition DeskPosition::Add(DeskPosition other) const {
    DeskPosition result = *this;
    result.cell += other.cell;
    result.offset += other.offset;
    result = result.Normalized();
    return result;
}

DeskPosition DeskPosition::Offset(v2 offset) const {
    DeskPosition result = *this;
    result.offset += offset;
    result = result.Normalized();
    return result;
}

v2 DeskPosition::Sub(DeskPosition other) const {
    v2 result;
    iv2 cellDiff = cell - other.cell;
    v2 offsetDiff = offset - other.offset;
    result = V2(cellDiff) * DeskCellSize + offsetDiff;
    return result;
}

v2 DeskPosition::RelativeTo(DeskPosition origin) const {
    v2 result = Sub(origin);
    return result;
}

TilePosition DeskPosition::ToTilePos() const {
    TilePosition result;
    result.tile = TileFromCell(cell);
    result.cell = CellInTile(cell);
    return result;
}

TilePosition::TilePosition(DeskPosition p) {
    *this = p.ToTilePos();
}

TilePosition::TilePosition(iv2 cell) {
    *this = DeskPosition(cell).ToTilePos();
}

DeskPosition TilePosition::ToDeskPos() const {
    DeskPosition result;
    result.offset = {};
    result.cell.x = (tile.x << DeskTileBitShift) | cell.x;
    result.cell.y = (tile.y << DeskTileBitShift) | cell.y;
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

TilePosition CellToTilePos(iv2 cell) {
    TilePosition result;
    result.tile = TileFromCell(cell);
    result.cell = CellInTile(cell);
    return result;
}
