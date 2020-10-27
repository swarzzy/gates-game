#include "Game.h"
#undef UNICODE
#undef _UNICODE

#include "Desk.h"

CodepointRange ranges[2];
f32 DefaultFontHeight = 24.0f;

void GameInit() {
    auto context = GetContext();
    auto allocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap);
    context->deskCanvas = CreateCanvas(allocator);
    //DrawListInit(&context->drawList, 256, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));

    auto image = ResourceLoaderLoadImage("../res/alpha_test.png", true, 4, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));
    TextureID texture = Renderer.UploadTexture(0, image->width, image->height, TextureFormat::RGBA8, TextureFilter::Bilinear, TextureWrapMode::Repeat, image->bits);
    assert(texture);
    context->testTexture = texture;

    ranges[0].begin = 32;
    ranges[0].end = 126;
    ranges[1].begin = 0x0410;
    ranges[1].end = 0x044f;

    auto f1 = ResourceLoader.BakeFont(&context->font, "../res/fonts/Merriweather-Regular.ttf", &allocator, 512, DefaultFontHeight, ranges, array_count(ranges));
    auto f2 = ResourceLoader.LoadFontBM(&context->sdfFont, "../res/fonts/merriweather_sdf.fnt", &allocator);
    assert(f1);
    assert(f2);

    context->sdfFont.sdf = true;
    context->sdfFont.sdfParams = V2(1.0f, 0.0f);

    TextureID fontAtlas = Renderer.UploadTexture(0, context->font.bitmapSize, context->font.bitmapSize, TextureFormat::R8, TextureFilter::Bilinear, TextureWrapMode::Repeat, context->font.bitmap);
    context->font.atlas = fontAtlas;

    TextureID sdfFontAtlas = Renderer.UploadTexture(0, context->sdfFont.bitmapSize, context->sdfFont.bitmapSize, TextureFormat::R8, TextureFilter::Bilinear, TextureWrapMode::Repeat, context->sdfFont.bitmap);
    context->sdfFont.atlas = sdfFontAtlas;

    auto desk = &context->desk;
    auto partInfo = &context->partInfo;

    InitDesk(desk, context->mainHeap);
    PartInfoInit(&context->partInfo);

    CreatePart(desk, partInfo, IV2(2, 2), PartType::And);
    CreatePart(desk, partInfo, IV2(6, 10), PartType::Or);
    CreatePart(desk, partInfo, IV2(-6, -10), PartType::Not);

    Part* source = CreatePart(desk, partInfo, IV2(10, 10), PartType::Source);
    Part* led = CreatePart(desk, partInfo, IV2(20, 10), PartType::Led);

    auto node = AddNode(desk);
    assert(node.id.id);

    source->outputs[0].nodeId = node.id;
    led->inputs[0].nodeId = node.id;

    Wire* wire = WirePins(desk, source->outputs, led->inputs);
    assert(wire);
}

void GameReload() {
}

void GameUpdate() {

}

void DebugDrawDesk(Desk* desk, Canvas* canvas) {
    DeskPosition begin = desk->origin;
    DeskPosition end = DeskPositionOffset(desk->origin, canvas->sizeCm);

    for (i32 y = begin.cell.y - 1; y != (end.cell.y + 1); y++) {
        for (i32 x = begin.cell.x - 1; x != (end.cell.x + 1); x++) {
            iv2 p = IV2(x, y);
            DeskCell* cell = GetDeskCell(desk, p, false);
            if (cell) {
                if (cell->value == CellValue::Part) {
                    Part* part = cell->part;
                    assert(part);
                    v2 min = DeskPositionRelative(desk->origin, MakeDeskPosition(p)) - DeskCellHalfSize;
                    v2 max = DeskPositionRelative(desk->origin, MakeDeskPosition(p + 1)) - DeskCellHalfSize;
                    DrawListPushRect(&canvas->drawList, min, max, 0.0f, V4(GetPartColor(part).xyz, 1.0f));
                } else if (cell->value == CellValue::Pin) {
                    Pin* pin = cell->pin;
                    assert(pin);
                    v4 color = V4(0.0f, 0.0f, 0.0f, 1.0f);

                    switch (pin->type) {
                    case PinType::Input: { color = V4(0.0f, 0.9f, 0.0f, 1.0f); } break;
                    case PinType::Output: { color = V4(0.9f, 0.9f, 0.0f, 1.0f); } break;
                    invalid_default();
                    }

                    v2 min = DeskPositionRelative(desk->origin, MakeDeskPosition(p)) - DeskCellHalfSize;
                    v2 max = DeskPositionRelative(desk->origin, MakeDeskPosition(p + 1)) - DeskCellHalfSize;
                    DrawListPushRect(&canvas->drawList, min, max, 0.0f, color);
                }
            }
        }
    }
}

void DebugDrawDeskCell(Desk* desk, Canvas* canvas, DeskPosition p, v4 color) {
    v2 min = DeskPositionRelative(desk->origin, MakeDeskPosition(p.cell)) - DeskCellHalfSize;
    v2 max = DeskPositionRelative(desk->origin, MakeDeskPosition(p.cell + 1)) - DeskCellHalfSize;
    DrawListPushRect(&canvas->drawList, min, max, 0.0f, color);
}

void PinProcessClick(Pin* pin, DeskPosition mouseP, MouseButton button) {
    auto context = GetContext();
    if (button == MouseButton::Left) {
        if (!context->pendingWire) {
            if (!pin->wire) {
                context->pendingWire = true;
                context->pendingWireBeginPin = pin;
            }
        } else {
            if (!pin->wire && pin != context->pendingWireBeginPin) {
                context->pendingWire = false;
                WirePins(&context->desk, context->pendingWireBeginPin, pin);
            }
        }
    }
}

void PartProcessClick(Part* part, DeskPosition mouseP, MouseButton button) {
    switch (part->type) {
    case PartType::Source: {
        part->active = !part->active;
    } break;
    default: {} break;
    }
}


void GameRender() {
    auto context = GetContext();
    auto input = GetInput();

    auto deskCanvas = &context->deskCanvas;
    auto desk = &context->desk;
    auto partInfo = &context->partInfo;

    f32 scaleSpeed = 0.1f;

    DEBUG_OVERLAY_SLIDER(deskCanvas->scale, 0.1f, 3.0f);

    v2 mousePBeforeScale = Hadamard(V2(input->mouseX, input->mouseY), deskCanvas->sizeCm);
    deskCanvas->scale = Max(0.1f, deskCanvas->scale + input->scrollFrameOffset *  (scaleSpeed * deskCanvas->scale));

    BeginCanvas(deskCanvas);

    v2 mousePosition = Hadamard(V2(input->mouseX, input->mouseY), deskCanvas->sizeCm);

    v2 scaleOffset = mousePosition - mousePBeforeScale;
    desk->origin = DeskPositionOffset(desk->origin, -scaleOffset);

    if (MouseButtonPressed(MouseButton::Right) && context->ghostPartEnabled) {
        context->ghostPartEnabled = false;
    }

    // Path stuff

    iv2 origin = IV2(1, 1);
    iv2 dest = IV2(20, 30);

    if (KeyPressed(Key::_1)) {
        memset(&context->ghostPart, 0, sizeof(Part));
        InitPart(partInfo, &context->ghostPart, IV2(0), PartType::And);
        context->ghostPartEnabled = true;
    } else if (KeyPressed(Key::_2)) {
        memset(&context->ghostPart, 0, sizeof(Part));
        InitPart(partInfo, &context->ghostPart, IV2(0), PartType::Or);
        context->ghostPartEnabled = true;
    } else if (KeyPressed(Key::_3)) {
        memset(&context->ghostPart, 0, sizeof(Part));
        InitPart(partInfo, &context->ghostPart, IV2(0), PartType::Not);
        context->ghostPartEnabled = true;
    } else if (KeyPressed(Key::_4)) {
        memset(&context->ghostPart, 0, sizeof(Part));
        InitPart(partInfo, &context->ghostPart, IV2(0), PartType::Led);
        context->ghostPartEnabled = true;
    } else if (KeyPressed(Key::_5)) {
        memset(&context->ghostPart, 0, sizeof(Part));
        InitPart(partInfo, &context->ghostPart, IV2(0), PartType::Source);
        context->ghostPartEnabled = true;
    }

    if (MouseButtonPressed(MouseButton::Right)) {
        context->ghostPartEnabled = false;
        context->pendingWire = false;
    }

    if (context->ghostPartEnabled) {
        DeskPosition mouseDeskP = DeskPositionOffset(desk->origin, mousePosition);
        DeskPosition offP = DeskPositionOffset(mouseDeskP, -V2(context->ghostPart.dim) * 0.5f * DeskCellSize);
        context->ghostPart.p = offP;

        if (MouseButtonPressed(MouseButton::Left)) {
            Part* clone = (Part*)desk->deskAllocator.Alloc(sizeof(Part), false);
            memcpy(clone, &context->ghostPart, sizeof(Part));
            InitPart(partInfo, clone, context->ghostPart.p.cell, context->ghostPart.type);
            AddPart(desk, clone);
        }
    }

    // raycast
    v2 mouseScreen = V2(input->mouseX, input->mouseY);
    v2 mouseCanvas = CanvasProjectScreenPos(deskCanvas, mouseScreen);
    DeskPosition mouseDesk = DeskPositionOffset(desk->origin, mouseCanvas);
    if (MouseButtonPressed(MouseButton::Left) || MouseButtonPressed(MouseButton::Right)) {
        MouseButton button = MouseButtonPressed(MouseButton::Left) ? MouseButton::Left : MouseButton::Right;
        DeskCell* mouseCell = GetDeskCell(desk, mouseDesk.cell, false);
        if (mouseCell) {
            if (mouseCell->value != CellValue::Empty) {
                switch (mouseCell->value) {
                case CellValue::Part: { assert(mouseCell->part); PartProcessClick(mouseCell->part, mouseDesk, button); } break;
                case CellValue::Pin: { assert(mouseCell->pin); PinProcessClick(mouseCell->pin, mouseDesk, button); } break;
                default: {} break;
                }
            }
        }
    }


    DEBUG_OVERLAY_TRACE(deskCanvas->sizeCm.x);
    DEBUG_OVERLAY_TRACE(deskCanvas->sizeCm.y);
    DEBUG_OVERLAY_TRACE(deskCanvas->sizePx.x);
    DEBUG_OVERLAY_TRACE(deskCanvas->sizePx.y);
    DEBUG_OVERLAY_TRACE(input->mouseFrameOffsetX);
    DEBUG_OVERLAY_TRACE(input->mouseFrameOffsetY);
    DEBUG_OVERLAY_TRACE(input->scrollFrameOffset);

    if (MouseButtonDown(MouseButton::Middle)) {
        v2 offset = Hadamard(V2(input->mouseFrameOffsetX, input->mouseFrameOffsetY), deskCanvas->sizeCm);
        desk->origin = DeskPositionOffset(desk->origin, -offset);
        //context->deskPosition += Hadamard(V2(input->mouseFrameOffsetX, input->mouseFrameOffsetY), deskCanvas->sizeCm);
    }

    DebugDrawDeskCell(desk, deskCanvas, mouseDesk, V4(1.0f, 0.0f, 0.0f, 1.0f));

    DeskPosition begin = desk->origin;
    DeskPosition end = DeskPositionOffset(desk->origin, deskCanvas->sizeCm);

    DrawListBeginBatch(&deskCanvas->drawList, TextureMode::Color);
    u32 vertLines = 0;
    u32 horzLines = 0;

    DEBUG_OVERLAY_TRACE(begin.cell.x);
    DEBUG_OVERLAY_TRACE(end.cell.x);

    for (i32 x = begin.cell.x - 1; x != (end.cell.x + 1); x++) {
        f32 thickness = 1.0f * deskCanvas->cmPerPixel;
        v2 min = V2(DeskPositionRelative(desk->origin, MakeDeskPosition(IV2(x, 0))).x - thickness * 0.5f, 0.0f) - DeskCellHalfSize;
        v2 max = V2(DeskPositionRelative(desk->origin, MakeDeskPosition(IV2(x, 0))).x + thickness * 0.5f, deskCanvas->sizeCm.y) - DeskCellHalfSize;
        DrawListPushRectBatch(&deskCanvas->drawList, min, max, 0.0f, V2(0.0f), V2(0.0f), V4(0.6f, 0.6f, 0.6f, 1.0f), 0.0f);
        vertLines++;
    }

    for (i32 y = begin.cell.y - 1; y != (end.cell.y + 1); y++) {
        f32 thickness = 1.0f * deskCanvas->cmPerPixel;
        v2 min = V2(0.0f, DeskPositionRelative(desk->origin, MakeDeskPosition(IV2(0, y))).y - thickness * 0.5f) - DeskCellHalfSize;
        v2 max = V2(deskCanvas->sizeCm.x, DeskPositionRelative(desk->origin, MakeDeskPosition(IV2(0, y))).y + thickness * 0.5f) - DeskCellHalfSize;
        DrawListPushRectBatch(&deskCanvas->drawList, min, max, 0.0f, V2(0.0f), V2(0.0f), V4(0.6f, 0.6f, 0.6f, 1.0f), 0.0f);
        horzLines++;
    }

    DEBUG_OVERLAY_TRACE(vertLines);
    DEBUG_OVERLAY_TRACE(horzLines);

    DrawListEndBatch(&deskCanvas->drawList);

    DEBUG_OVERLAY_TRACE(desk->origin.cell.x);
    DEBUG_OVERLAY_TRACE(desk->origin.cell.y);

    PropagateSignals(desk);
    ForEach(&desk->partsHashMap, [&](Part** it) {
        PartProcessSignals(partInfo, *it);
    });

    switch (context->drawMode) {
    case DrawMode::Normal: { DrawDesk(desk, deskCanvas); } break;
    case DrawMode::DeskDebug: { DebugDrawDesk(desk, deskCanvas); } break;
    invalid_default();
    }

    if (context->ghostPartEnabled) {
        DrawPart(desk, deskCanvas, &context->ghostPart, 0.5f);
    }

    // TODO: Culling
    DrawListBeginBatch(&deskCanvas->drawList, TextureMode::Color);
    f32 thickness = 0.1f;
    for (u32 i = 0; i < desk->wires.Count(); i++) {
        Wire* wire = desk->wires.Begin() + i;
        v2 begin = DeskPositionRelative(desk->origin, wire->p0);
        v2 end = DeskPositionRelative(desk->origin, wire->p1);
        DrawSimpleLineBatch(&deskCanvas->drawList, begin, end, 0.0f, thickness, V4(0.2f, 0.2f, 0.2f, 1.0f));
    }

    if (context->pendingWire) {
        DeskPosition p1 = ComputePinPosition(context->pendingWireBeginPin);
        v2 lineBeg = DeskPositionRelative(desk->origin, p1);
        v2 lineEnd = mouseCanvas;
        DrawSimpleLineBatch(&deskCanvas->drawList, lineBeg, lineEnd, 0.0f, thickness, V4(0.5f, 0.2f, 0.0f, 1.0f));
    }

    DrawListEndBatch(&deskCanvas->drawList);

    EndCanvas(deskCanvas);



#if 0
    const char16* string = u"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.";

    f32 width = 300.0f;
    static f32 scale = 1.0f;
    static f32 scaleSDF = 1.0f;

    auto textDim = CalcTextSizeUnscaled(&context->font, string, width / scale);
    textDim.x *= pixelSize.x * scale;
    textDim.y *= pixelSize.y * scale;

    //DEBUG_OVERLAY_SLIDER(context->sdfFont.sdfParams.x, 0.0f, 3.0f);
    //DEBUG_OVERLAY_SLIDER(context->sdfFont.sdfParams.y, 0.0f, 3.0f);
    auto prevScale = scale;
    DEBUG_OVERLAY_SLIDER(scale, 0.3f, 3.0f);
    DEBUG_OVERLAY_SLIDER(scaleSDF, 0.3f, 3.0f);


    if (scale != prevScale) {
        f32 newHeight = Round(DefaultFontHeight * scale);
        auto allocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap);
        FontFreeResources(&context->font, allocator);
        memset(context->font.glyphIndexTable, 0, sizeof(u16) * U16::Max);
        auto f1 = ResourceLoader.BakeFont(&context->font, "../res/fonts/Merriweather-Regular.ttf", &allocator, 8192, newHeight, ranges, array_count(ranges));
        assert(f1);
        TextureID fontAtlas = Renderer.UploadTexture(context->font.atlas, context->font.bitmapSize, context->font.bitmapSize, TextureFormat::R8, TextureFilter::Bilinear, TextureWrapMode::Repeat, context->font.bitmap);
        context->font.atlas = fontAtlas;
    }


    DrawText(list, &context->font, string, V3(200.0f, 350.0f, 0.0f), V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.5f), width, TextAlign::Center, 1.0f);
    i32 enable = false;
    DEBUG_OVERLAY_SLIDER(enable, 0, 1);
    if (enable) {
        DrawText(list, &context->sdfFont, string, V3(700.0f, 350.0f, 0.0f), V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.5f), width, TextAlign::Center, scaleSDF);
    }

    //string[strOffset] = 0;

    //DrawListPushQuad(list, V2(100.0f, 200.0f), V2(100.0f + textDim.x, 200.0f), V2(100.0f + textDim.x, 200.0f + textDim.y), V2(100.0f, 200.0f + textDim.y), 0.0f, V4(1.0f, 0.0f, 0.0f, 0.3f));
    //DrawText(list, &context->font, string, V2(100.0f, 200.0f), 0.0f, V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f));
    //DrawTextLine(list, &context->font, string, V2(100.0f, 200.0f), 0.0f, V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f), 220.0f, textDim);
    //DrawText(list, &context->font, string, V3(100.0f, 200.0f, 0.0f), V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f), width, TextAlign::Left, scale);
    //DrawListPushQuad(list, V2(100.0f, 200.0f), V2(105.0f, 200.0f), V2(104.0f, 205.0f), V2(100.0f, 205.0f), 0.0f, V4(0.0f, 0.0f, 1.0f, 1.0f));
#endif
    //Renderer.RenderDrawList(list);
    //DrawListClear(list);

    if(KeyPressed(Key::Tilde)) {
        context->consoleEnabled = !context->consoleEnabled;
    }

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }
}
