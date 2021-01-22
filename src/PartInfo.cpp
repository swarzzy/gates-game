#include "PartInfo.h"

void FuncPartAnd(Part* part) {
    u8 value = 1;
    for (u32 i = 0; i < part->inputCount; i++) {
        value = value && GetInput(part, i)->value;
    }
    GetOutput(part, 0)->value = value;
}

void FuncPartOr(Part* part) {
    u8 value = 0;
    for (u32 i = 0; i < part->inputCount; i++) {
        value = value || GetInput(part, i)->value;
    }
    GetOutput(part, 0)->value = value;
}

void FuncPartNot(Part* part) {
    u8 value = !GetInput(part, 0)->value;
    GetOutput(part, 0)->value = value;
}

void FuncPartSource(Part* part) {
    GetOutput(part, 0)->value = (u8)part->active;
    if (part->active) {
        part->label = U"I";
    } else {
        part->label = U"0";
    }
}

void FuncPartLed(Part* part) {
    part->active = GetInput(part, 0)->value;
}

void FuncPartClock(Part* part) {
    auto platform = GetPlatform();
    if (part->clockDiv == 1) {
        part->active = !part->active;
    } else {
        part->active = (platform->simStepCount % part->clockDiv) == 1 ? !part->active : part->active;
    }
    GetOutput(part, 0)->value = part->active;
}

void AllocatePins(Desk* desk, Part* part, u32 inputCount, u32 outputCount) {
    u32 count = inputCount + outputCount;
    part->pins = DArray<Pin>(&desk->deskAllocator, count);
    part->inputCount = inputCount;
    part->outputCount = outputCount;
}

void DeallocatePins(Desk* desk, Part* part) {
    part->pins.FreeBuffers();
}

void InitPartAnd(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::And;
    part->dim = IV2(3, 5);

    part->activeColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->inactiveColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->label = U"&";

    AllocatePins(desk, part, 2, 1);
    *GetInput(part, 0) = CreatePin(part, V2(-1, 1) / 2.0f, PinType::Input);
    *GetInput(part, 1) = CreatePin(part, V2(-1, 3) / 2.0f, PinType::Input);
    *GetOutput(part, 0) = CreatePin(part, V2(3, 3) / 2.0f, PinType::Output);
}

void InitPartOr(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Or;
    part->dim = IV2(3, 5);

    part->activeColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->inactiveColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->label = U"1";

    AllocatePins(desk, part, 2, 1);
    *GetInput(part, 0) = CreatePin(part, V2(-1, 1) / 2.0f, PinType::Input);
    *GetInput(part, 1) = CreatePin(part, V2(-1, 3) / 2.0f, PinType::Input);
    *GetOutput(part, 0) = CreatePin(part, V2(3, 3) / 2.0f, PinType::Output);
}

void InitPartNot(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Not;
    part->dim = IV2(3, 3);

    part->activeColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->inactiveColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    //part->label = u"0";

    AllocatePins(desk, part, 1, 1);
    *GetInput(part, 0) = CreatePin(part, V2(-1, 1) / 2.0f, PinType::Input);
    *GetOutput(part, 0) = CreatePin(part, V2(3, 1) / 2.0f, PinType::Output);
}

void InitPartLed(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Led;
    part->dim = IV2(3, 3);

    part->activeColor = V4(0.9f, 0.0f, 0.0f, 1.0f);
    part->inactiveColor = V4(0.3f, 0.3f, 0.3f, 1.0f);

    AllocatePins(desk, part, 1, 0);
    *GetInput(part, 0) = CreatePin(part, V2(-1, 1) / 2.0f, PinType::Input);
}

void InitPartSource(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Source;
    part->dim = IV2(3, 3);

    part->label = U"0";

    part->activeColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->inactiveColor = V4(1.0f, 1.0f, 1.0f, 1.0f);

    AllocatePins(desk, part, 0, 1);
    *GetOutput(part, 0) = CreatePin(part, V2(3, 1) / 2.0f, PinType::Output);
}

void InitPartClock(Desk* desk, Part* part) {
    memset(part, 0, sizeof(Part));
    part->type = PartType::Clock;
    part->dim = IV2(3, 3);

    part->label = U"CL";

    part->clockDiv = 1;

    part->activeColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    part->inactiveColor = V4(1.0f, 1.0f, 1.0f, 1.0f);

    AllocatePins(desk, part, 0, 1);
    *GetOutput(part, 0) = CreatePin(part, V2(3, 1) / 2.0f, PinType::Output);
}

void ClickPartSource(Desk* desk, Part* part) {
    part->active = !part->active;
}

void ClickPartClock(Desk* desk, Part* part) {
    part->clockDiv = (part->clockDiv << 1) > 1024 ? 1 : (part->clockDiv << 1);
    log_print("Clock divider: %lu\n", (unsigned long)part->clockDiv);
}

void PartInfoInit(PartInfo* info) {
    // TODO: Ensure memory are zero
    memset(info, 0, sizeof(PartInfo));

    info->partInitializers[(u32)PartType::And] = InitPartAnd;
    info->partInitializers[(u32)PartType::Or] = InitPartOr;
    info->partInitializers[(u32)PartType::Not] = InitPartNot;
    info->partInitializers[(u32)PartType::Source] = InitPartSource;
    info->partInitializers[(u32)PartType::Led] = InitPartLed;
    info->partInitializers[(u32)PartType::Clock] = InitPartClock;


    info->partFunctions[(u32)PartType::And] = FuncPartAnd;
    info->partFunctions[(u32)PartType::Or] = FuncPartOr;
    info->partFunctions[(u32)PartType::Not] = FuncPartNot;
    info->partFunctions[(u32)PartType::Source] = FuncPartSource;
    info->partFunctions[(u32)PartType::Led] = FuncPartLed;
    info->partFunctions[(u32)PartType::Clock] = FuncPartClock;

    info->partClickHandlers[(u32)PartType::Source] = ClickPartSource;
    info->partClickHandlers[(u32)PartType::Clock] = ClickPartClock;
}

void HandlePartClick(PartInfo* info, Desk* desk, Part* part) {
    assert((u32)part->type < (u32)PartType::_Count);
    auto fn = info->partClickHandlers[(u32)part->type];
    if (fn) {
        fn(desk, part);
    }
}
