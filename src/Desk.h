#pragma once

const f32 DeskCellSize = 0.5f; // Centimeters
const f32 DeskCellHalfSize = DeskCellSize / 2.0f;


struct DeskPosition {
    iv2 cell;
    v2 offset;
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

struct Desk {
    DeskPosition origin;
};
