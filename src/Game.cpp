#include "Game.h"
#undef UNICODE
#undef _UNICODE

#include "Desk.h"

CodepointRange ranges[2];
f32 DefaultFontHeight = 24.0f;

void GameInit() {
    auto context = GetContext();
    context->mainAllocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap);
    context->deskCanvas = CreateCanvas(&context->mainAllocator);
    //DrawListInit(&context->drawList, 256, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));

    auto image = ResourceLoaderLoadImage("../res/alpha_test.png", true, 4, MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap));
    TextureID texture = Renderer.UploadTexture(0, image->width, image->height, TextureFormat::RGBA8, TextureFilter::Bilinear, TextureWrapMode::Repeat, image->bits);
    assert(texture);
    context->testTexture = texture;

    ranges[0].begin = 32;
    ranges[0].end = 126;
    ranges[1].begin = 0x0410;
    ranges[1].end = 0x044f;

    auto f1 = ResourceLoader.BakeFont(&context->font, "../res/fonts/Merriweather-Regular.ttf", &context->mainAllocator, 512, DefaultFontHeight, ranges, array_count(ranges));
    auto f2 = ResourceLoader.LoadFontBM(&context->sdfFont, "../res/fonts/merriweather_sdf.fnt", &context->mainAllocator);
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

    InitDesk(desk, &context->deskCanvas, partInfo, context->mainHeap);
    PartInfoInit(&context->partInfo);

    ToolManagerInit(&context->toolManager, desk);

    TryCreatePart(desk, partInfo, IV2(2, 2), PartType::And);
    TryCreatePart(desk, partInfo, IV2(6, 10), PartType::Or);
    TryCreatePart(desk, partInfo, IV2(-6, -10), PartType::Not);

    Part* source = TryCreatePart(desk, partInfo, IV2(10, 10), PartType::Source);
    Part* led = TryCreatePart(desk, partInfo, IV2(20, 10), PartType::Led);

    Wire* wire = TryWirePins(desk, GetInput(led, 0), GetOutput(source, 0));
    assert(wire);
}

void GameReload() {
}

void GameUpdate() {

}

void GameRender() {
    auto context = GetContext();
    auto input = GetInput();

    auto deskCanvas = &context->deskCanvas;
    auto desk = &context->desk;
    auto partInfo = &context->partInfo;
    auto toolManager = &context->toolManager;

    f32 scaleSpeed = 0.1f;

    DEBUG_OVERLAY_SLIDER(deskCanvas->scale, 0.1f, 3.0f);

    v2 mousePBeforeScale = Hadamard(V2(input->mouseX, input->mouseY), deskCanvas->sizeCm);
    deskCanvas->scale = Max(0.1f, deskCanvas->scale + input->scrollFrameOffset *  (scaleSpeed * deskCanvas->scale));

    BeginCanvas(deskCanvas);

    v2 mousePosition = Hadamard(V2(input->mouseX, input->mouseY), deskCanvas->sizeCm);

    v2 scaleOffset = mousePosition - mousePBeforeScale;
    desk->origin = desk->origin.Offset(-scaleOffset);

    PartInitializerFn* partInitializer = nullptr;

    if (KeyPressed(Key::_1)) {
        partInitializer = partInfo->partInitializers[(u32)PartType::And];
    } else if (KeyPressed(Key::_2)) {
        partInitializer = partInfo->partInitializers[(u32)PartType::Or];
    } else if (KeyPressed(Key::_3)) {
        partInitializer = partInfo->partInitializers[(u32)PartType::Not];
    } else if (KeyPressed(Key::_4)) {
        partInitializer = partInfo->partInitializers[(u32)PartType::Led];
    } else if (KeyPressed(Key::_5)) {
        partInitializer = partInfo->partInitializers[(u32)PartType::Source];
    }

    if (partInitializer) {
        ToolPartEnable(toolManager, desk, partInitializer);
    }

    if (MouseButtonPressed(MouseButton::Right)) {
        ToolManagerRightMouseDown(toolManager);
    }

    if (MouseButtonPressed(MouseButton::Left)) {
        ToolManagerLeftMouseDown(toolManager);
    }

    if (MouseButtonReleased(MouseButton::Left)) {
        ToolManagerLeftMouseUp(toolManager);
    }

    DEBUG_OVERLAY_TRACE(toolManager->dragStarted);
    DEBUG_OVERLAY_TRACE(toolManager->dragAttempt);

    ToolManagerUpdate(toolManager);

    DEBUG_OVERLAY_TRACE(deskCanvas->sizeCm.x);
    DEBUG_OVERLAY_TRACE(deskCanvas->sizeCm.y);
    DEBUG_OVERLAY_TRACE(deskCanvas->sizePx.x);
    DEBUG_OVERLAY_TRACE(deskCanvas->sizePx.y);
    DEBUG_OVERLAY_TRACE(input->mouseFrameOffsetX);
    DEBUG_OVERLAY_TRACE(input->mouseFrameOffsetY);
    DEBUG_OVERLAY_TRACE(input->scrollFrameOffset);

    if (MouseButtonDown(MouseButton::Middle)) {
        v2 offset = Hadamard(V2(input->mouseFrameOffsetX, input->mouseFrameOffsetY), deskCanvas->sizeCm);
        desk->origin = desk->origin.Offset(-offset);
        //context->deskPosition += Hadamard(V2(input->mouseFrameOffsetX, input->mouseFrameOffsetY), deskCanvas->sizeCm);
    }

    //DebugDrawDeskCell(desk, deskCanvas, toolManager->mouseDeskPos, V4(1.0f, 0.0f, 0.0f, 1.0f));

    DeskPosition begin = desk->origin;
    DeskPosition end = desk->origin.Offset(deskCanvas->sizeCm);

    DrawListBeginBatch(&deskCanvas->drawList, TextureMode::Color);
    u32 vertLines = 0;
    u32 horzLines = 0;

    DEBUG_OVERLAY_TRACE(begin.cell.x);
    DEBUG_OVERLAY_TRACE(end.cell.x);

    for (i32 x = begin.cell.x - 1; x != (end.cell.x + 1); x++) {
        f32 thickness = 1.0f * deskCanvas->cmPerPixel;
        v2 min = V2(DeskPosition(IV2(x, 0)).RelativeTo(desk->origin).x - thickness * 0.5f, 0.0f) - DeskCellHalfSize;
        v2 max = V2(DeskPosition(IV2(x, 0)).RelativeTo(desk->origin).x + thickness * 0.5f, deskCanvas->sizeCm.y) - DeskCellHalfSize;
        DrawListPushRectBatch(&deskCanvas->drawList, min, max, 0.0f, V2(0.0f), V2(0.0f), V4(0.6f, 0.6f, 0.6f, 1.0f), 0.0f);
        vertLines++;
    }

    for (i32 y = begin.cell.y - 1; y != (end.cell.y + 1); y++) {
        f32 thickness = 1.0f * deskCanvas->cmPerPixel;
        v2 min = V2(0.0f, DeskPosition(IV2(0, y)).RelativeTo(desk->origin).y - thickness * 0.5f) - DeskCellHalfSize;
        v2 max = V2(deskCanvas->sizeCm.x, DeskPosition(IV2(0, y)).RelativeTo(desk->origin).y + thickness * 0.5f) - DeskCellHalfSize;
        DrawListPushRectBatch(&deskCanvas->drawList, min, max, 0.0f, V2(0.0f), V2(0.0f), V4(0.6f, 0.6f, 0.6f, 1.0f), 0.0f);
        horzLines++;
    }

    DEBUG_OVERLAY_TRACE(vertLines);
    DEBUG_OVERLAY_TRACE(horzLines);

    DrawListEndBatch(&deskCanvas->drawList);

    DEBUG_OVERLAY_TRACE(desk->origin.cell.x);
    DEBUG_OVERLAY_TRACE(desk->origin.cell.y);

    PropagateSignals(desk);

    ListForEach(&desk->parts, part) {
        PartProcessSignals(partInfo, part);
    } ListEndEach(part);

    switch (context->drawMode) {
    case DrawMode::Normal: { DrawDesk(desk, deskCanvas); } break;
        //case DrawMode::DeskDebug: { DebugDrawDesk(desk, deskCanvas); } break;
    invalid_default();
    }

    // Wire thickness
    // TODO: Factor this out
    f32 thickness = 0.1f;
#if false
    if (MouseButtonPressed(MouseButton::Left)) {
        // TODO: Optimize this. Narrow down traversal subset. We cound for instance
        // compute line bounding box and check only inside it.
        Wire* hitWire = nullptr;
        WireNode* hitNode = nullptr;
        ListForEach(&desk->wires, wire) {
            if (wire->nodes.Count() == 0) {
            } else {
                assert(wire->nodes.Count() >= 2);
                for (u32 i = 1; i < wire->nodes.Count(); i++) {
                    WireNode* prev = wire->nodes.Data() + (i - 1);
                    WireNode* curr = wire->nodes.Data() + i;

                    v2 begin = prev->p.RelativeTo(desk->origin);
                    v2 end = curr->p.RelativeTo(desk->origin);

                    if (CheckWireSegmentHit(toolManager->mouseCanvasPos, begin, end, thickness)) {
                        log_print("Intersecting wire at tick %d!\n", (int)GetPlatform()->tickCount);
                        hitWire = wire;
                        hitNode = prev;
                        break;
                    }
                }
            }
        } ListEndEach;

        if (hitWire) {
            assert(hitNode);
            u32 index = hitWire->nodes.IndexFromPtr(hitNode);
            // TODO: toolManager->mouseDeskPos
            DeskPosition p = DeskPosition(DeskPosition(desk->origin.cell, toolManager->mouseCanvasPos).cell);

            if (TryRegisterWireNodePlacement(desk, hitWire, p.cell)) {
                WireNode* newNode = hitWire->nodes.Insert(index + 1);
                newNode->p = p;
            }
        }
    }

    if (MouseButtonPressed(MouseButton::Right)) {
        DeskCell* cell = GetDeskCell(desk, toolManager->mouseDeskPos.cell);
        if (cell && cell->value == CellValue::WireNode) {
            Wire* wire = cell->wire;
            iv2 p = toolManager->mouseDeskPos.cell;
            WireNode* node = nullptr;
            ForEach(&wire->nodes, it) {
                if (it->p.cell == p) {
                    node = it;
                    break;
                }
            } EndEach;

            if (node) {
                UnregisterWireNodePlacement(desk, wire, p);
                wire->nodes.Erase(node);
            }
        }
    }
#endif
    ToolManagerRender(toolManager);

    // TODO: Culling
    DrawListBeginBatch(&deskCanvas->drawList, TextureMode::Color);
    ListForEach(&desk->wires, wire) {
        assert(wire->nodes.Count() >= 2);
        for (u32 i = 1; i < wire->nodes.Count(); i++) {
            DeskPosition* prev = wire->nodes.Data() + (i - 1);
            DeskPosition* curr = wire->nodes.Data() + i;

            v2 begin = prev->RelativeTo(desk->origin);
            v2 end = curr->RelativeTo(desk->origin);

            DrawSimpleLineBatch(&deskCanvas->drawList, begin, end, 0.0f, thickness, V4(0.2f, 0.2f, 0.2f, 1.0f));
        }
#if 0
        ForEach(&wire->nodes, node) {
            if (index == 0 || (index == wire->nodes.Count() - 1)) continue;
            v2 pRel = node->RelativeTo(desk->origin);
            v2 min = pRel - DeskCellHalfSize;
            v2 max = pRel + DeskCellHalfSize;
            DrawListPushRectBatch(&deskCanvas->drawList, min, max, 0.0f, {}, {}, V4(0.2f, 0.2f, 0.2f, 1.0f), 0.0f);
        } EndEach;
#endif
    } ListEndEach(wire);

    DrawListEndBatch(&deskCanvas->drawList);
    //const char16* string = u"&";
    //v3 textP = V3(DeskPositionRelative(desk->origin, MakeDeskPosition(IV2(5, 5))), 0.0f);
    //DrawText(&deskCanvas->drawList, &context->sdfFont, string, textP, V4(0.0f, 0.0f, 0.0f, 1.0f), V2(deskCanvas->cmPerPixel), V2(0.5f), F32::Infinity, TextAlign::Center, deskCanvas->scale);

    EndCanvas(deskCanvas);
#if 0
    const char16* string = u"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.";

    v2 pixelSize = V2(deskCanvas->cmPerPixel);
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


    DrawText(&deskCanvas->drawList, &context->font, string, V3(10.0f, 10.0f, 0.0f), V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f), width, TextAlign::Center, 1.0f);
    i32 enable = false;
    DEBUG_OVERLAY_SLIDER(enable, 0, 1);
    if (enable) {
        DrawText(&deskCanvas->drawList, &context->sdfFont, string, V3(20.0f, 10.0f, 0.0f), V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.5f), width, TextAlign::Center, scaleSDF);
    }

    //string[strOffset] = 0;

    //DrawListPushQuad(list, V2(100.0f, 200.0f), V2(100.0f + textDim.x, 200.0f), V2(100.0f + textDim.x, 200.0f + textDim.y), V2(100.0f, 200.0f + textDim.y), 0.0f, V4(1.0f, 0.0f, 0.0f, 0.3f));
    //DrawText(list, &context->font, string, V2(100.0f, 200.0f), 0.0f, V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f));
    //DrawTextLine(list, &context->font, string, V2(100.0f, 200.0f), 0.0f, V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f), 220.0f, textDim);
    //DrawText(list, &context->font, string, V3(100.0f, 200.0f, 0.0f), V4(0.0f, 0.0f, 0.0f, 1.0f), pixelSize, V2(0.0f), width, TextAlign::Left, scale);
    //DrawListPushQuad(list, V2(100.0f, 200.0f), V2(105.0f, 200.0f), V2(104.0f, 205.0f), V2(100.0f, 205.0f), 0.0f, V4(0.0f, 0.0f, 1.0f, 1.0f));
    //Renderer.RenderDrawList(list);
    //DrawListClear(list);
#endif
    if(KeyPressed(Key::Tilde)) {
        context->consoleEnabled = !context->consoleEnabled;
    }

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }
}
