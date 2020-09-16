#include "Draw.h"

void DrawListInit(DrawList* list, u32 capacity, Allocator allocator) {
    assert(!list->commands.Capacity());
    list->commands = GrowableArray<DrawCommandData>::Make(allocator);
}

void DrawListPush(DrawList* list, const DrawCommandData* command) {
    list->commands.PushBack(*command);
}
