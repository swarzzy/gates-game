#pragma once

#include "Part.h"

struct SerializedPart {
    u32 id;
    PartType type;
    DeskPosition p;
    iv2 dim;
    b32 active;
    u32 clockDiv;
    const char32* label;

    u32 inputCount;
    u32 outputCount;

    Array<v2> pinRelPositions;
};

char* SerializePart(Part* part, StringBuilder* builder) {
    StringBuilderClear(builder);
    StringBuilderAppend(builder, "[\n");
    StringBuilderAppend(builder, "    {\n");
    StringBuilderAppend(builder, "    {\n");
    StringBuilderAppend(builder, "    },\n");
    StringBuilderAppend(builder, "]\n");

    serialized->id = part->id;
    serialized->type = part->type;
    serialized->p = part->p;
    serialized->dim = part->dim;
    serialized->active = part->active;
    serialized->clockDiv = part->clockDiv;
}
