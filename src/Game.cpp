#include "Game.h"
#undef UNICODE
#undef _UNICODE

#include "Desk.h"
#include "StringBuilder.h"
#include "Serialize.h"


CodepointRange ranges[2];
f32 DefaultFontHeight = 24.0f;

struct StringKey {
    const char* str;
    explicit StringKey(const char* s) : str(s) {};
};

u32 HashU32(StringKey& key) {
    u32 hash = 0;
    auto len = StringSizeZ(key.str);
    for (u32 i = 0; i < len; i++) {
        hash += key.str[i];
    }
    return hash;
}

bool HashCompareKeys(StringKey& a, StringKey& b) {
    return StringsAreEqual(a.str, b.str);
}


void GameInit() {
    auto context = GetContext();
    context->mainAllocator = MakeAllocator(HeapAllocAPI, HeapFreeAPI, context->mainHeap);
    context->menuCanvas = CreateCanvas(&context->mainAllocator);

#if 1
    // String builder test
    StringBuilder builder = StringBuilder(&context->mainAllocator, U"Hll");
    builder.Reserve(10);
    builder.Append("abcdef", sizeof("abcdef"));
    builder.Append(U" sailor");
    builder.Append(-5);
    builder.AppendLiteral(" ");
    builder.Append((i32)38456348563845638);
    builder.AppendLiteral(" ");
    builder.Append(-123.345234234234f);

    builder.Append("sailor", sizeof("sailor"));
    builder.Append("_qwertyuiopasdfghjhlkhklhkh_", sizeof("_qwertyuiopasdfghjhlkhklhkh_"));

    char32* str1 = builder.CopyString();
    char* str3 = builder.CopyStringAsASCII();
    char32* str2 = builder.StealString();

#endif
#if false

    double value = 0.0;
    char buffer[128];
    const char* start = nullptr;
    stbsp__uint32 len = 0;
    stbsp__int32 decimalPos = 0;
    stbsp__uint32 fracDigits = 0;

    //#define STBSP__NUMSZ 512 // big enough for e308 (with commas) or e-307
    //char num[STBSP__NUMSZ];
    // sets decimal pos to STBSP__SPECIAL
    auto a = stbsp__real_to_str(&start, &len, buffer, &decimalPos, value, fracDigits);

    serializer.BeginStruct();
    serializer.WriteField(U"field 1", U"value1");
    serializer.WriteField(U"field 2", U"value2");
    serializer.WriteField(U"field 3", U"value3");
    //serializer.BeginArrayRow();
    serializer.WriteValue(U"1", false);
    serializer.WriteValue(U"2", false);
    serializer.WriteValue(U"3", false);
    serializer.WriteValue(U"4", false);
    serializer.EndArray();
    serializer.WriteField(U"field 4", U"value4");
    serializer.EndStruct();
    serializer.BeginStruct();
    serializer.WriteField(U"field 5", U"value5");
    serializer.EndStruct();
#endif
    SerializedPart part {};
    part.id = 5;
    part.type = 1;
    part.pTile = IV2(100, -2);
    part.pOffset = V2(0.4f, 0.5f);
    part.dim = IV2(3, 5);
    part.active = true;
    part.label = U"Test part";
    part.inputCount = 3;
    part.outputCount = 1;
    part.pinRelPositions = DArray<v2>(&context->mainAllocator);
    part.pinRelPositions.PushBack(V2(0.0f, 0.0f));
    part.pinRelPositions.PushBack(V2(1.0f, 0.0f));
    part.pinRelPositions.PushBack(V2(0.0f, 1.0f));
    part.pinRelPositions.PushBack(V2(1.0f, 1.0f));
    part.pinRelPositions.PushBack(V2(-0.0f, -1.0f));
    part.pinRelPositions.PushBack(V2(1.175494351e-38, 3.402823466e+38));
    //part.pinRelPositions.PushBack(V2(1.0f, HUGE_VAL));

    SerializedWire wires[2];
    wires[0].inputId = 1;
    wires[0].inputId = 55;
    wires[0].nodes = DArray<DeskPosition>(&context->mainAllocator);
    wires[0].nodes.PushBack(DeskPosition(IV2(1, 2)));
    wires[0].nodes.PushBack(DeskPosition(IV2(3, 4)));
    wires[0].nodes.PushBack(DeskPosition(IV2(5, 6), V2(0.5f, 0.25f)));

    wires[1].inputId = 3;
    wires[1].inputId = 4;
    wires[1].nodes = DArray<DeskPosition>(&context->mainAllocator);
    wires[1].nodes.PushBack(DeskPosition(IV2(10, 10)));
    wires[1].nodes.PushBack(DeskPosition(IV2(11, 11)));
    wires[1].nodes.PushBack(DeskPosition(IV2(0, 0), V2(0.0f, 0.23f)));

    JsonSerializer serializer = JsonSerializer(&context->mainAllocator);
    serializer.inlineMode = false;

    serializer.BeginObject();

    serializer.BeginArray(U"Parts");

    serializer.BeginObject();
    SerializeToJson(&serializer, &part);
    serializer.EndObject();

    serializer.EndArray();

    serializer.BeginArray(U"Wires");
    serializer.BeginObject();
    SerializeToJson(&serializer, wires + 0);
    serializer.EndObject();
    serializer.BeginObject();
    SerializeToJson(&serializer, wires + 1);
    serializer.EndObject(false);
    serializer.EndArray();

    serializer.EndObject(false);


    #if false

    serializer.BeginObject(U"One");

    serializer.BeginArray(U"Foo");
    serializer.WriteArrayMember(1);
    serializer.WriteArrayMember(2);
    serializer.WriteArrayMember(3);
    serializer.WriteArrayMember(4);
    serializer.WriteArrayMember(5, false);
    serializer.EndArray(false);

    serializer.EndObject();

    serializer.BeginObject(U"Two");
    serializer.WriteField(U"Vector", V3(0.0f, 1.0f, 2.0f));
    serializer.EndObject();

    serializer.BeginArray(U"Array");

    serializer.BeginObject();
    serializer.WriteField(U"Bar", U"Hello");
    serializer.EndObject();

    serializer.BeginObject();
    serializer.WriteField(U"Bar", U"Hello");
    serializer.EndObject(false);

    serializer.EndArray();


    serializer.BeginArray(U"Wires");
    serializer.BeginObject();
    SerializeToJson(&serializer, wires + 0);
    serializer.EndObject();
    serializer.BeginObject();
    SerializeToJson(&serializer, wires + 1);
    serializer.EndObject(false);
    serializer.EndArray(false);


    serializer.EndObject(false);


#endif
    //serializer.BeginArray();


    //serializer.EndArray();

    auto data = serializer.GenerateStringUtf8();

    Platform.DebugWriteFile("test.json", data.Data(), data.Count() - 1);

    JsonDeserializer deserializer = JsonDeserializer(&context->mainAllocator);
    ParseDeskDescription(&deserializer, data.Data(), data.Count() - 1);

    auto partsArray = DArray<SerializedPart>(&context->mainAllocator);
    auto wiresArray = DArray<SerializedWire>(&context->mainAllocator);

    ForEach(&deserializer.parts, it) {
        JsonPushObject(&deserializer, *it);
        if (DeserializePart(&deserializer))
        {
            partsArray.PushBack(deserializer.scratchPart);
            partsArray.Last()->pinRelPositions = deserializer.scratchPart.pinRelPositions.Clone();
            partsArray.Last()->label = CopyString(deserializer.scratchPart.label, &context->mainAllocator);
        }
        JsonPopObject(&deserializer);
    } EndEach;

    ForEach(&deserializer.wires, it) {
        JsonPushObject(&deserializer, *it);
        if (DeserializeWire(&deserializer))
        {
            wiresArray.PushBack(deserializer.scratchWire);
            wiresArray.Last()->nodes = deserializer.scratchWire.nodes.Clone();
        }
        JsonPopObject(&deserializer);
    } EndEach;

    //data.Data()[491] = 0;

    json_parse_result_s parseResult;
    json_value_s* root = json_parse_ex(data.Data(), data.Count() - 1, json_parse_flags_allow_json5, nullptr, nullptr, &parseResult);

    json_object_s* rootObj = json_value_as_object(root);
    assert(rootObj);

    auto map = HashMap<StringKey, json_value_s*>(&context->mainAllocator);

    json_object_element_s* it = rootObj->start;
    while (it) {
        json_string_s* name = it->name;
        if (StringsAreEqual(name->string, "Parts")) {
            json_array_s* array = json_value_as_array(it->value);
            assert(array);
            json_array_element_s* partIt = array->start;
            while(partIt) {
                json_object_s* part = json_value_as_object(partIt->value);
                assert(part);
                json_object_element_s* partField = part->start;
                while (partField) {
                    json_value_s** v = map.Add(StringKey(partField->name->string));
                    *v = partField->value;
                    partField = partField->next;
                }


                partIt = partIt->next;
            }
        }
        if (StringsAreEqual(name->string, "Wires")) {
            auto array = json_value_as_array(it->value);
            assert(array);
            auto wireIt = array->start;
            while(wireIt) {
                printf("Found wire\n");
                wireIt = wireIt->next;
            }

        }
        it = it->next;
    }

    auto image = LoadImage("../res/alpha_test.png", true, 4, &context->mainAllocator);
    TextureID texture = Renderer.UploadTexture(0, image->width, image->height, TextureFormat::RGBA8, TextureFilter::Bilinear, TextureWrapMode::Repeat, image->bits);
    assert(texture);
    context->testTexture = texture;

    ranges[0].begin = 32;
    ranges[0].end = 126;
    ranges[1].begin = 0x0410;
    ranges[1].end = 0x044f;

    auto f1 = LoadFontTrueType(&context->font, "../res/fonts/Merriweather-Regular.ttf", &context->mainAllocator, 512, DefaultFontHeight, ranges, array_count(ranges));
    auto f2 = LoadFontBM(&context->sdfFont, "../res/fonts/merriweather_sdf.fnt", &context->mainAllocator);
    assert(f1);
    assert(f2);

    context->sdfFont.sdf = true;
    context->sdfFont.sdfParams = V2(1.0f, 0.0f);

    TextureID fontAtlas = Renderer.UploadTexture(0, context->font.bitmapSize, context->font.bitmapSize, TextureFormat::R8, TextureFilter::Bilinear, TextureWrapMode::Repeat, context->font.bitmap);
    context->font.atlas = fontAtlas;

    TextureID sdfFontAtlas = Renderer.UploadTexture(0, context->sdfFont.bitmapSize, context->sdfFont.bitmapSize, TextureFormat::R8, TextureFilter::Bilinear, TextureWrapMode::Repeat, context->sdfFont.bitmap);
    context->sdfFont.atlas = sdfFontAtlas;

    auto platform = GetPlatformMutable();
    //platform->targetFramerate = 60;
    platform->vsync = VSyncMode::Full;
    platform->targetSimStepsPerSecond = 128;

    context->gameState = GameState::Menu;

    PartInfoInit(&context->partInfo);

    for (u32 i = 0; i < (u32)Strings::_Count; i++) {
        context->strings[i] = U"<null>";
    }

    context->language = Language::English;
    InitLanguageEnglish();

    for (int i = 0; i < 10000; i++) {
        CreateDesk();
        DestroyDesk();
    }
}

void GameReload() {
}

void GameUpdate() {
    auto context = GetContext();

    switch (context->gameState) {
    case GameState::Menu: { GameUpdateMenu(); } break;
    case GameState::Desk: { GameUpdateDesk(); } break;
        invalid_default();
    }
}

void GameSim() {
    auto context = GetContext();

    switch (context->gameState) {
    case GameState::Menu: { } break;
    case GameState::Desk: { GameSimDesk(); } break;
        invalid_default();
    }


    if(KeyPressed(Key::Tilde)) {
        context->consoleEnabled = !context->consoleEnabled;
    }
}

void GameRender() {
    auto context = GetContext();

    switch (context->gameState) {
    case GameState::Menu: { GameRenderMenu(); } break;
    case GameState::Desk: { GameRenderDesk(); } break;
        invalid_default();
    }

    DrawDebugPerformanceCounters();

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }
}

i32 updateSleepMs = 0;
i32 simSleepMs = 0;
i32 renderSleepMs = 0;

void GameUpdateMenu() {
    auto context = GetContext();
    auto input = GetInput();
    auto canvas = &context->menuCanvas;

    UpdateCanvas(canvas);


}

void GameRenderMenu() {
    auto context = GetContext();
    auto input = GetInput();

    auto canvas = &context->menuCanvas;

    // TODO: Now we checking for mouse input in render stage. It's actually totally ok.
    // But if we will cache text geometry then we will be able to move mouse handling code
    // to update which is more appropriate place for that code
    //
    // OR!!!!!
    //
    // Just make update and render be the same thing. It is mitght be a good idea since a probably don't care too much
    // about input polling rate in this game
    // And again, now update and render are exequted at same frequency
    //

    if (MouseButtonPressed(MouseButton::Left)) {
        if (context->hitNewGame) {
            CreateDesk();
            context->gameState = GameState::Desk;
            return;
        }

        if (context->hitExit) {
            KillProcess();
        }

        if (context->hitLanguage) {
            if (context->language == Language::English) {
                InitLanguageRussian();
                context->language = Language::Russian;
            } else if (context->language == Language::Russian) {
                InitLanguageEnglish();
                context->language =Language::English;
            } else {
                unreachable();
            }
        }
    }

    v2 mousePosition = Hadamard(V2(input->mouseX, input->mouseY), canvas->sizeCm);

    BeginCanvas(canvas);

    v3 pTitle = V3(CanvasPositionFromNormalized(canvas, V2(0.5f, 0.75f)), 0.0f);
    v3 pNewGame = V3(CanvasPositionFromNormalized(canvas, V2(0.5f, 0.5f)), 0.0f);
    v3 pExit = V3(CanvasPositionFromNormalized(canvas, V2(0.5f, 0.4f)), 0.0f);
    v3 pLang = V3(CanvasPositionFromNormalized(canvas, V2(0.5f, 0.3f)), 0.0f);

    // TODO: sRGB to Linear
    v4 color = V4(0.0f, 0.0f, 0.0f, 1.0f);
    // TODO: Better text drawing API. Also maybe think about geometry caching?
    DrawText(&canvas->drawList, &context->sdfFont, GetString(Strings::GatesGame), pTitle, color, V2(canvas->cmPerPixel), V2(0.5f), F32::Infinity, TextAlign::Center, 2.0f);

    v4 newGameColor = context->hitNewGame ? V4(1.0f, 0.0f, 0.0f, 1.0f) : V4(0.0f, 0.0f, 0.0f, 1.0f);
    v4 exitColor = context->hitExit ? V4(1.0f, 0.0f, 0.0f, 1.0f) : V4(0.0f, 0.0f, 0.0f, 1.0f);
    v4 langColor = context->hitLanguage ? V4(1.0f, 0.0f, 0.0f, 1.0f) : V4(0.0f, 0.0f, 0.0f, 1.0f);

    Box2D newGameBox = DrawText(&canvas->drawList, &context->sdfFont, GetString(Strings::NewGame), pNewGame, newGameColor, V2(canvas->cmPerPixel), V2(0.5f), F32::Infinity, TextAlign::Center, 1.0f);
    Box2D exitBox = DrawText(&canvas->drawList, &context->sdfFont, GetString(Strings::Exit), pExit, exitColor, V2(canvas->cmPerPixel), V2(0.5f), F32::Infinity, TextAlign::Center, 1.0f);
    Box2D langBox = DrawText(&canvas->drawList, &context->sdfFont, GetString(Strings::Language), pLang - V3(0.5f, 0.0f, 0.0f), langColor, V2(canvas->cmPerPixel), V2(1.0f, 0.5f), F32::Infinity, TextAlign::Center, 1.0f);
    DrawText(&canvas->drawList, &context->sdfFont, GetString(Strings::CurrentLanguage), pLang, langColor, V2(canvas->cmPerPixel), V2(0.0f, 0.5f), F32::Infinity, TextAlign::Center, 1.0f);

    v2 newGameRelMouse = mousePosition - pNewGame.xy;
    v2 exitRelMouse = mousePosition - pExit.xy;
    v2 langRelMouse = mousePosition - pLang.xy;

    DEBUG_OVERLAY_TRACE(newGameRelMouse);
    DEBUG_OVERLAY_TRACE(newGameBox.min);
    DEBUG_OVERLAY_TRACE(newGameBox.max);

    // TODO: Delay for one frame for hit text highlight ib not a good idea
    context->hitNewGame = PointInBox2D(newGameBox, newGameRelMouse);
    context->hitExit = PointInBox2D(exitBox, exitRelMouse);
    context->hitLanguage = PointInBox2D(langBox, langRelMouse);

    EndCanvas(canvas);
}

void GameUpdateDesk() {
    ThreadSleep(updateSleepMs);
    auto context = GetContext();
    auto input = GetInput();

    auto desk = GetDesk();
    auto deskCanvas = &desk->canvas;
    auto partInfo = &context->partInfo;
    auto toolManager = &desk->toolManager;

    if (KeyPressed(Key::F5)) {
        ListForEach(&desk->parts, part) {
            //SerializedPart serialized {};
            //SerializePart(desk, part, &serialized);
        } ListEndEach(part);
    }

    f32 scaleSpeed = 0.1f;

    DEBUG_OVERLAY_SLIDER(deskCanvas->scale, 0.1f, 3.0f);

    v2 mousePBeforeScale = Hadamard(V2(input->mouseX, input->mouseY), deskCanvas->sizeCm);
    deskCanvas->scale = Max(0.1f, deskCanvas->scale + input->scrollFrameOffset *  (scaleSpeed * deskCanvas->scale));

    UpdateCanvas(deskCanvas);

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
    } else if (KeyPressed(Key::_6)) {
        partInitializer = partInfo->partInitializers[(u32)PartType::Clock];
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

    ToolManagerUpdate(toolManager);

    if (MouseButtonDown(MouseButton::Middle)) {
        v2 offset = Hadamard(V2(input->mouseFrameOffsetX, input->mouseFrameOffsetY), deskCanvas->sizeCm);
        desk->origin = desk->origin.Offset(-offset);
    }
}

void GameSimDesk() {
    auto context = GetContext();
    if (context->gameState == GameState::Desk) {
        ThreadSleep(simSleepMs);
        auto partInfo = &context->partInfo;
        auto desk = GetDesk();

        PropagateSignals(desk);

        ListForEach(&desk->parts, part) {
            PartProcessSignals(partInfo, part);
        } ListEndEach(part);
    }
}

void GameRenderDesk() {
    ThreadSleep(renderSleepMs);

    auto platform = GetPlatformMutable();

    DEBUG_OVERLAY_SLIDER(updateSleepMs, 0, 100);
    DEBUG_OVERLAY_SLIDER(simSleepMs, 0, 100);
    DEBUG_OVERLAY_SLIDER(renderSleepMs, 0, 100);
    DEBUG_OVERLAY_SLIDER(platform->targetSimStepsPerSecond, 1, 10000);

    auto context = GetContext();
    auto input = GetInput();

    auto desk = GetDesk();
    auto deskCanvas = &desk->canvas;
    auto partInfo = &context->partInfo;
    auto toolManager = &desk->toolManager;

    BeginCanvas(deskCanvas);

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

    DrawListEndBatch(&deskCanvas->drawList);

    switch (context->drawMode) {
    case DrawMode::Normal: { DrawDesk(desk, deskCanvas); } break;
        //case DrawMode::DeskDebug: { DebugDrawDesk(desk, deskCanvas); } break;
    invalid_default();
    }

    ToolManagerRender(toolManager);

    f32 thickness = 0.1f;

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

    // TODO: Relative to upper left
    v3 pExit = V3(CanvasPositionFromNormalized(deskCanvas, V2(0.95f, 0.01f)), 0.0f);
    v4 exitColor = context->hitExitDesk ? V4(1.0f, 0.0f, 0.0f, 1.0f) : V4(0.0f, 0.0f, 0.0f, 1.0f);
    Box2D exitBox = DrawText(&deskCanvas->drawList, &context->sdfFont, GetString(Strings::Exit), pExit, exitColor, V2(deskCanvas->cmPerPixel), V2(0.5f, 0.0f), F32::Infinity, TextAlign::Center, 1.0f);

    v2 mousePosition = Hadamard(V2(input->mouseX, input->mouseY), deskCanvas->sizeCm);
    v2 exitRelMouse = mousePosition - pExit.xy;

    context->hitExitDesk = PointInBox2D(exitBox, exitRelMouse);

    EndCanvas(deskCanvas);

    if (MouseButtonPressed(MouseButton::Left) && context->hitExitDesk) {
        context->hitExitDesk = false;
        DestroyDesk();
        context->gameState = GameState::Menu;
    }

    if (context->consoleEnabled) {
        DrawConsole(&context->console);
    }
}

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
    #if false
    auto desk = &context->desk;
    auto partInfo = &context->partInfo;

    InitDesk(desk, &context->deskCanvas, partInfo, context->mainHeap);

    ToolManagerInit(&context->toolManager, desk);

    TryCreatePart(desk, partInfo, IV2(2, 2), PartType::And);
    TryCreatePart(desk, partInfo, IV2(6, 10), PartType::Or);
    TryCreatePart(desk, partInfo, IV2(-6, -10), PartType::Not);

    Part* source = TryCreatePart(desk, partInfo, IV2(10, 10), PartType::Source);
    Part* led = TryCreatePart(desk, partInfo, IV2(20, 10), PartType::Led);

    Wire* wire = TryWirePins(desk, GetInput(led, 0), GetOutput(source, 0));
    assert(wire);
#endif
