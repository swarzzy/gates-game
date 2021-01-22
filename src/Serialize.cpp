#include "Serialize.h"

void JsonSerializer::Init(Allocator* allocator) {
    builder = StringBuilder(allocator);
    outputBuffer = DArray<char>(allocator);
    builder.Reserve(DefaultBufferSize / sizeof(char32));
    indentLevel = 0;
    inlineMode = false;
}

void JsonSerializer::Clear() {
    builder.Clear();
    outputBuffer.Clear();
    indentLevel = 0;
}

void JsonSerializer::_Indent() {
    if (!inlineMode) {
        for (i32 i = 0; i < indentLevel; i++) {
            builder.Append(U"    ");
        }
    }
}

void JsonSerializer::BeginField(const char32* name) {
    _Indent();
    if (name) {
        builder.Append(U"\"");
        builder.Append(name);
        builder.Append(U"\": ");
    }
}

void JsonSerializer::EndField(bool comma) {
    if (inlineMode) {
        if (comma) builder.Append(U", ");
    } else {
        if (comma) builder.Append(U",\n");
        else builder.Append(U"\n");
    }
}


void JsonSerializer::BeginArray(const char32* name) {
    BeginField(name);
    if (inlineMode) builder.Append(U"[ ");
    else builder.Append(U"[\n");
    indentLevel++;
}

void JsonSerializer::EndArray(bool comma) {
    assert(indentLevel > 0);
    indentLevel--;
    _Indent();
    if (inlineMode) builder.Append(U" ]");
    else builder.Append(U"]");
    EndField(comma);
}

void JsonSerializer::BeginObject(const char32* name) {
    BeginField(name);
    if (inlineMode) builder.Append(U"{ ");
    else builder.Append(U"{\n");
    indentLevel++;
}

void JsonSerializer::EndObject(bool comma) {
    assert(indentLevel > 0);
    indentLevel--;
    _Indent();
    if (inlineMode) builder.Append(U" }");
    else builder.Append(U"}");
    EndField(comma);
}

template <typename T>
void JsonSerializer::WriteArrayMember(T value, bool comma) {
    WriteField(nullptr, value, comma);
}

template <typename T>
void JsonSerializer::WriteField(const char32* name, T value, bool comma) {
    BeginField(name);
    WriteValue(value);
    EndField(comma);
}

template <typename T, u32 Size>
void JsonSerializer::WriteField(const char32* name, Vector<T, Size> value, bool comma) {
    BeginArray(name);
    WriteValue(value);
    EndArray(comma);
}

template <typename T>
void JsonSerializer::WriteField(const char32* name, ArrayRef<T> value, bool comma) {
    BeginArray(name);
    WriteValue(value);
    EndArray(comma);
}

void JsonSerializer::WriteField(const char32* name, DeskPosition value, bool comma) {
    BeginObject(name);
    WriteField(U"cell", value.cell);
    WriteField(U"offset", value.offset, false);
    EndObject(comma);
}

template <typename T>
void JsonSerializer::_WriteValue(T value, bool quoted) {
    if (quoted) builder.Append("\"");
    builder.Append(value);
    if (quoted) builder.Append("\"");
}

void JsonSerializer::WriteValue(u32 value) {
    _WriteValue(value, false);
}

void JsonSerializer::WriteValue(i32 value) {
    _WriteValue(value, false);
}

void JsonSerializer::WriteValue(f32 value) {
    _WriteValue(value, false);
}

void JsonSerializer::WriteValue(bool value) {
    _WriteValue(value, false);
}

void JsonSerializer::WriteValue(const char32* value) {
    _WriteValue(value, true);
}

template <typename T, u32 Size>
void JsonSerializer::WriteValue(Vector<T, Size> value) {
    for (u32 i = 0; i < Size; i++) {
        WriteArrayMember(value.data[i], i != (Size - 1));
    }
}

template <typename T>
void JsonSerializer::WriteValue(ArrayRef<T> value) {
    for (u32 i = 0; i < value.Count(); i++) {
        WriteArrayMember(value.Data()[i], i != (value.Count() - 1));
    }
}

ArrayRef<char> JsonSerializer::GenerateStringUtf8() {
    outputBuffer.Clear();
    outputBuffer.Reserve(builder.CapacityZ());
    char tmp[4];
    for (const char32* at = builder.Buffer(); *at != 0; at++) {
        void* back = utf8catcodepoint(tmp, *at, 4);
        usize written = (usize)((uptr)back - (uptr)tmp);
        char* mem = outputBuffer.PushBackN((u32)written);
        memcpy(mem, tmp, written);
    }

    outputBuffer.PushBack('\0');

    return outputBuffer.AsRef();
}

void JsonDeserializer::Init(Allocator* allocator) {
    destinationAllocator = allocator;

    for (u32 i = 0; i < array_count(prototypes); i++) {
        prototypes[i] = HashMap<PrototypeKey, json_value_s*>(allocator);
    }

    parts = DArray<json_object_s*>(allocator);
    wires = DArray<json_object_s*>(allocator);

    scratchPart.pinRelPositions = DArray<v2>(allocator);
    scratchWire.nodes = DArray<DeskPosition>(allocator);
}

void FreeDeskDescription(JsonDeserializer* deserializer) {
    assert(deserializer->root);
    deserializer->destinationAllocator->Dealloc(deserializer->root);
    deserializer->root = nullptr;
}

bool ParseDeskDescription(JsonDeserializer* deserializer, const char* json, u32 lenZ) {
    bool result = true;

    deserializer->parts.Clear();
    deserializer->wires.Clear();

    auto allocator = [](void* data, size_t size) {
        auto a = (Allocator*)data;
        return a->Alloc(size, false);
    };

    json_parse_result_s parseResult;
    json_value_s* root = json_parse_ex(json, lenZ - 1, json_parse_flags_allow_json5, allocator, deserializer->destinationAllocator, &parseResult);
    if (!root) {
        result = false;
        return result;
    }

    deserializer->root = root;

    defer {
        if (result == false) {
            if (deserializer->root) {
                deserializer->destinationAllocator->Dealloc(deserializer->root);
                deserializer->root = nullptr;
            }

            deserializer->parts.Clear();
            deserializer->wires.Clear();
        }

        if (result == true && deserializer->parts.Count() == 0) {
            result = false;
        }
    };


    json_object_s* rootObj = json_value_as_object(root);
    if (!rootObj) {
        result = false;
        return result;
    }

    json_object_element_s* it = rootObj->start;
    while (it) {
        json_string_s* name = it->name;
        if (StringsAreEqual(name->string, "Parts")) {
            json_array_s* array = json_value_as_array(it->value);
            if (!array) {
                result = false;
                return result;
            }

            json_array_element_s* partIt = array->start;
            while(partIt) {
                json_object_s* part = json_value_as_object(partIt->value);
                if (!part) {
                    result = false;
                    return result;
                }

                deserializer->parts.PushBack(part);

                partIt = partIt->next;
            }
        } else if (StringsAreEqual(name->string, "Wires")) {
            auto array = json_value_as_array(it->value);
            if (!array) {
                result = false;
                return result;
            }

            auto wireIt = array->start;
            while(wireIt) {
                json_object_s* wire = json_value_as_object(wireIt->value);
                if (!wire) {
                    result = false;
                    return result;
                }

                deserializer->wires.PushBack(wire);

                wireIt = wireIt->next;
            }
        } else {
            result = false;
            return result;
        }

        it = it->next;
    }

    return result;
}

u32 HashU32(PrototypeKey& key) {
    u32 hash = 0;
    auto len = StringSizeZ(key.str);
    for (u32 i = 0; i < len; i++) {
        hash += key.str[i];
    }
    return hash;
}

bool HashCompareKeys(PrototypeKey& a, PrototypeKey& b) {
    return StringsAreEqual(a.str, b.str);
}

HashMap<PrototypeKey, json_value_s*>* JsonPushObject(JsonDeserializer* deserializer, json_object_s* obj) {
    assert(deserializer->prototypeAt < array_count(deserializer->prototypes));
    auto p = deserializer->prototypes + deserializer->prototypeAt;
    deserializer->prototypeAt++;
    p->Clear();
    json_object_element_s* field = obj->start;
    while (field) {
        json_value_s** v = p->Add(PrototypeKey(field->name->string));
        *v = field->value;
        field = field->next;
    }
    return p;
}

void JsonPopObject(JsonDeserializer* deserializer) {
    assert(deserializer->prototypeAt > 0);
    deserializer->prototypeAt--;
}

Option<u32> JsonTryGetU32(JsonDeserializer* deserializer, json_value_s* value) {
    Option<u32> result {};
    json_number_s* num = json_value_as_number(value);
    if (num) {
        auto parseResult = StringToU32(num->number);
        if (parseResult.hasValue) {
            result = Option(parseResult.value);
        }
    }
    return result;
}

Option<i32> JsonTryGetI32(JsonDeserializer* deserializer, json_value_s* value) {
    Option<i32> result {};
    json_number_s* num = json_value_as_number(value);
    if (num) {
        auto parseResult = StringToI32(num->number);
        if (parseResult.hasValue) {
            result = Option(parseResult.value);
        }
    }
    return result;
}

Option<f32> JsonTryGetF32(JsonDeserializer* deserializer, json_value_s* value) {
    Option<f32> result {};
    json_number_s* num = json_value_as_number(value);
    if (num) {
        auto parseResult = StringToF32(num->number);
        if (parseResult.hasValue) {
            result = Option(parseResult.value);
        }
    }
    return result;
}

Option<bool> JsonTryGetBool(JsonDeserializer* deserializer, json_value_s* value) {
    Option<bool> result {};
    if (json_value_is_true(value)) result = Option(true);
    else if (json_value_is_false(value)) result = Option(false);
    return result;
}

Option<char32*> JsonTryGetString(JsonDeserializer* deserializer, json_value_s* value) {
    Option<char32*> result {};
    json_string_s* str = json_value_as_string(value);
    if (str) {
        auto builder = StringBuilder(deserializer->destinationAllocator, str->string, (usize)str->string_size + 1, 0);
        result = Option(builder.StealString());
    }
    return result;
}

json_array_s* JsonTryGetArray(JsonDeserializer* deserializer, json_value_s* value) {
    json_array_s* result = json_value_as_array(value);
    return result;
}

json_object_s* JsonTryGetObject(JsonDeserializer* deserializer, json_value_s* value) {
    json_object_s* result = json_value_as_object(value);
    return result;
}

template <typename T, u32 Size>
Option<Vector<T, Size>> JsonTryGetVector(JsonDeserializer* deserializer, json_value_s* value) {
    Option<Vector<T, Size>> result {};
    json_array_s* arr = json_value_as_array(value);
    if (arr) {
        if (arr->length == Size) {
            json_array_element_s* it = arr->start;
            Vector<T, Size> vec {};
            u32 index = 0;
            bool ok = true;
            while (it) {
                Option<T> value {};
                compile_if (EqualTypes<T, i32>) {
                    value = JsonTryGetI32(deserializer, it->value);
                } else compile_if (EqualTypes<T, u32>) {
                    value = JsonTryGetI32(deserializer, it->value);
                } else compile_if (EqualTypes<T, f32>) {
                    value = JsonTryGetF32(deserializer, it->value);
                } else {
                    static_assert(false);
                }

                if (!value.hasValue) {
                    ok = false;
                    break;
                }

                vec.data[index] = value.value;
                index++;

                it = it->next;
            }

            if (ok) {
                result = Option(vec);
            }
        }
    }

    return result;
}

Option<i32> JsonTryGetI32(JsonDeserializer* deserializer, const char* fieldName) {
    Option<i32> result {};
    assert(deserializer->prototypeAt);
    json_value_s** entry = deserializer->prototypes[deserializer->prototypeAt - 1].Find(PrototypeKey(fieldName));
    if (entry) {
        result = JsonTryGetI32(deserializer, *entry);
    }
    return result;
}

Option<u32> JsonTryGetU32(JsonDeserializer* deserializer, const char* fieldName) {
    Option<u32> result {};
    assert(deserializer->prototypeAt);
    json_value_s** entry = deserializer->prototypes[deserializer->prototypeAt - 1].Find(PrototypeKey(fieldName));
    if (entry) {
        result = JsonTryGetU32(deserializer, *entry);
    }
    return result;
}

Option<f32> JsonTryGetF32(JsonDeserializer* deserializer, const char* fieldName) {
    Option<f32> result {};
    assert(deserializer->prototypeAt);
    json_value_s** entry = deserializer->prototypes[deserializer->prototypeAt - 1].Find(PrototypeKey(fieldName));
    if (entry) {
        result = JsonTryGetF32(deserializer, *entry);
    }
    return result;
}

Option<bool> JsonTryGetBool(JsonDeserializer* deserializer, const char* fieldName) {
    Option<bool> result {};
    assert(deserializer->prototypeAt);
    json_value_s** entry = deserializer->prototypes[deserializer->prototypeAt - 1].Find(PrototypeKey(fieldName));
    if (entry) {
        result = JsonTryGetBool(deserializer, *entry);
    }
    return result;
}

Option<char32*> JsonTryGetString(JsonDeserializer* deserializer, const char* fieldName) {
    Option<char32*> result {};
    assert(deserializer->prototypeAt);
    json_value_s** entry = deserializer->prototypes[deserializer->prototypeAt - 1].Find(PrototypeKey(fieldName));
    if (entry) {
        result = Option(JsonTryGetString(deserializer, *entry));
    }
    return result;
}

json_array_s* JsonTryGetArray(JsonDeserializer* deserializer, const char* fieldName) {
    json_array_s* result = nullptr;
    assert(deserializer->prototypeAt);
    json_value_s** entry = deserializer->prototypes[deserializer->prototypeAt - 1].Find(PrototypeKey(fieldName));
    if (entry) {
        result = JsonTryGetArray(deserializer, *entry);
    }
    return result;
}

template <typename T, u32 Size>
Option<Vector<T, Size>> JsonTryGetVector(JsonDeserializer* deserializer, const char* fieldName) {
    Option<Vector<T, Size>> result {};
    assert(deserializer->prototypeAt);
    json_value_s** entry = deserializer->prototypes[deserializer->prototypeAt - 1].Find(PrototypeKey(fieldName));
    if (entry) {
        result = JsonTryGetVector<T, Size>(deserializer, *entry);
    }
    return result;
}

bool DeserializePart(JsonDeserializer* deserializer) {
    auto tmp = deserializer->scratchPart.pinRelPositions;
    assert(!deserializer->scratchPart.label); // Ensure ownership over label is taken away
    deserializer->scratchPart = {};
    deserializer->scratchPart.pinRelPositions = tmp;
    deserializer->scratchPart.pinRelPositions.Clear();

    auto version = JsonTryGetU32(deserializer, "version"); if (!version.hasValue) goto notOk;
    auto id = JsonTryGetU32(deserializer, "id"); if (!id.hasValue) goto notOk;
    auto type = JsonTryGetU32(deserializer, "type"); if (!type.hasValue) goto notOk;
    auto pTile = JsonTryGetVector<i32, 2>(deserializer, "pTile"); if (!pTile.hasValue) goto notOk;
    auto pOffset = JsonTryGetVector<f32, 2>(deserializer, "pOffset"); if (!pOffset.hasValue) goto notOk;
    auto dim = JsonTryGetVector<i32, 2>(deserializer, "dim"); if (!dim.hasValue) goto notOk;
    auto active = JsonTryGetBool(deserializer, "active"); if (!active.hasValue) goto notOk;
    auto clockDiv = JsonTryGetU32(deserializer, "clockDiv"); if (!clockDiv.hasValue) goto notOk;
    auto label = JsonTryGetString(deserializer, "label"); if (!label.hasValue) goto notOk;
    auto inputCount = JsonTryGetU32(deserializer, "inputCount"); if (!inputCount.hasValue) goto notOk;
    auto outputCount = JsonTryGetU32(deserializer, "outputCount"); if (!outputCount.hasValue) goto notOk;

    auto pinRelPositions = JsonTryGetArray(deserializer, "pinRelPositions"); if (!pinRelPositions) goto notOk;
    auto it = pinRelPositions->start;
    while (it) {
        auto p = JsonTryGetVector<f32, 2>(deserializer, it->value); if (!p.hasValue) goto notOk;
        deserializer->scratchPart.pinRelPositions.PushBack(p.value);
        it = it->next;
    }

    deserializer->scratchPart.version = version.value;
    deserializer->scratchPart.id = id.value;
    deserializer->scratchPart.type = type.value;
    deserializer->scratchPart.pTile = pTile.value;
    deserializer->scratchPart.pOffset = pOffset.value;
    deserializer->scratchPart.dim = dim.value;
    deserializer->scratchPart.active = active.value;
    deserializer->scratchPart.clockDiv = clockDiv.value;
    deserializer->scratchPart.label = label.value;
    deserializer->scratchPart.inputCount = inputCount.value;
    deserializer->scratchPart.outputCount = outputCount.value;

    return true;

notOk:

    if (label.hasValue && label.value) deserializer->destinationAllocator->Dealloc(label.value);
    deserializer->scratchPart.pinRelPositions.Clear();

    return false;
}

bool DeserializeWire(JsonDeserializer* deserializer) {
    auto tmp = deserializer->scratchWire.nodes;
    deserializer->scratchWire = {};
    deserializer->scratchWire.nodes = tmp;
    deserializer->scratchWire.nodes.Clear();

    auto version = JsonTryGetU32(deserializer, "version"); if (!version.hasValue) goto notOk;
    auto inputId = JsonTryGetU32(deserializer, "inputId"); if (!inputId.hasValue) goto notOk;
    auto inputPinIndex = JsonTryGetU32(deserializer, "inputPinIndex"); if (!inputPinIndex.hasValue) goto notOk;
    auto outputId = JsonTryGetU32(deserializer, "outputId"); if (!outputId.hasValue) goto notOk;
    auto outputPinIndex = JsonTryGetU32(deserializer, "outputPinIndex"); if (!outputPinIndex.hasValue) goto notOk;

    auto nodes = JsonTryGetArray(deserializer, "nodes"); if (!nodes) goto notOk;
    auto it = nodes->start;
    while (it) {
        auto obj = JsonTryGetObject(deserializer, it->value);
        if (!obj) goto notOk;

        JsonPushObject(deserializer, obj);
        auto cell = JsonTryGetVector<i32, 2>(deserializer, "cell");
        auto offset = JsonTryGetVector<f32, 2>(deserializer, "offset");
        JsonPopObject(deserializer);
        if ((!cell.hasValue) || (!offset.hasValue)) {
            goto notOk;
        }
        deserializer->scratchWire.nodes.PushBack(DeskPosition(cell.value, offset.value));
        it = it->next;
    }

    deserializer->scratchWire.version = version.value;
    deserializer->scratchWire.inputId = inputId.value;
    deserializer->scratchWire.inputPinIndex = inputPinIndex.value;
    deserializer->scratchWire.outputId = outputId.value;
    deserializer->scratchWire.outputPinIndex = outputPinIndex.value;

    return true;

notOk:
    deserializer->scratchWire.nodes.Clear();

    return false;
}

ArrayRef<char> SerializeDeskToJson(Desk* desk, JsonSerializer* serializer) {
    serializer->Clear();
    serializer->BeginObject();

    serializer->BeginArray(U"Parts");

    ListForEach(&desk->parts, part) {
        serializer->BeginObject();
        desk->serializerScratchPart.pinRelPositions.Clear();
        SerializePart(part, &desk->serializerScratchPart);
        SerializeToJson(serializer, &desk->serializerScratchPart);
        serializer->EndObject();
    } ListEndEach(part);

    serializer->EndArray();

    serializer->BeginArray(U"Wires");

    ListForEach(&desk->wires, wire) {
        serializer->BeginObject();
        desk->serializerScratchWire.nodes.Clear();
        SerializeWire(wire, &desk->serializerScratchWire);
        SerializeToJson(serializer, &desk->serializerScratchWire);
        serializer->EndObject();
    } ListEndEach(wire);

    serializer->EndArray();

    serializer->EndObject(false);

    auto fileData = serializer->GenerateStringUtf8();

    return fileData;
}

bool DeserializeDeskFromJson(JsonDeserializer* deserializer, const char* json, usize jsonLenZ, Desk* desk) {
    bool result = true;
    bool parsed = ParseDeskDescription(deserializer, json, jsonLenZ);
    if (parsed) {
        defer { FreeDeskDescription(deserializer); };
        desk->idRemappingTable.Clear();

        ForEach(&deserializer->parts, it) {
            JsonPushObject(deserializer, *it);
            if (DeserializePart(deserializer)) {
                Part* part = TryCreatePartFromSerialized(desk, desk->partInfo, &deserializer->scratchPart);
                if (!part) {
                    result = false;
                    break;
                }
                if (deserializer->scratchPart.label) {
                    // If nobody owned the label, deallocate it
                    deserializer->destinationAllocator->Dealloc(deserializer->scratchPart.label);
                    deserializer->scratchPart.label = nullptr;
                }
            }
            JsonPopObject(deserializer);
        } EndEach;

        ForEach(&deserializer->wires, it) {
            JsonPushObject(deserializer, *it);
            if (DeserializeWire(deserializer)) {
                Wire* wire = TryCreateWireFromSerialized(desk, &deserializer->scratchWire);
                if (!wire) {
                    result = false;
                    break;
                }
            }
            JsonPopObject(deserializer);
        } EndEach;
    } else {
        result = false;
    }
    return result;
}
