#pragma once

#include "Common.h"

#include "GrowableArray.h"

enum struct DrawCommand {
    Rectangle
};

struct DrawCommandData {
    DrawCommand command;
    v2 min;
    v2 max;
    f32 z;
};

struct DrawList {
    GrowableArray<DrawCommandData> commands;
};

void DrawListInit(DrawList* list, u32 capacity, Allocator allocator);

void DrawListPush(DrawList* list, const DrawCommandData* command);

inline void DrawRect(DrawList* list, v2 min, v2 max, f32 z) {
    DrawCommandData command {};
    command.command = DrawCommand::Rectangle;
    command.min = min;
    command.max = max;
    command.z = z;
    DrawListPush(list, &command);
}
