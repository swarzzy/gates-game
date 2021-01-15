#pragma once

#include "Part.h"

struct JsonSerializer {
    const usize DefaultBufferSize = Megabytes(2);
    StringBuilder builder = StringBuilder();
    DArray<char> outputBuffer = DArray<char>();
    i32 indentLevel = 0;
    bool inlineMode;

    JsonSerializer() = default;
    JsonSerializer(Allocator* allocator);

    void BeginField(const char32* name);
    void EndField(bool comma = true);

    void BeginArray(const char32* name = nullptr);
    void EndArray(bool comma = true);

    void BeginObject(const char32* name = nullptr);
    void EndObject(bool comma = true);

    template <typename T>
    void WriteArrayMember(T value, bool comma = true);

    template <typename T>
    void WriteField(const char32* name, T value, bool comma = true);

    template <typename T, u32 Size>
    void WriteField(const char32* name, Vector<T, Size> value, bool comma = true);

    template <typename T>
    void WriteField(const char32* name, ArrayRef<T> value, bool comma = true);

    void WriteField(const char32* name, DeskPosition value, bool comma = true);

    void WriteValue(const char32* value);
    void WriteValue(u32 value);
    void WriteValue(i32 value);
    void WriteValue(f32 value);
    void WriteValue(bool value);
    void WriteValue(DeskPosition value);

    template <typename T, u32 Size>
    void WriteValue(Vector<T, Size> value);

    template <typename T>
    void WriteValue(ArrayRef<T> value);

    ArrayRef<char> GenerateStringUtf8();

    template <typename T>
    void _WriteValue(T value, bool quoted);

    void _Indent();
};

struct SerializedPart {
    u32 version;
    u32 id;
    u32 type;
    iv2 pTile;
    v2 pOffset;
    iv2 dim;
    bool active;
    u32 clockDiv;
    const char32* label;

    u32 inputCount;
    u32 outputCount;

    DArray<v2> pinRelPositions;
};

struct SerializedWire {
    u32 version;
    u32 inputId;
    u32 outputId;
    DArray<DeskPosition> nodes;
};

void SerializeToJson(JsonSerializer* serializer, SerializedWire* wire) {
    serializer->WriteField(U"version", wire->version);
    serializer->WriteField(U"inputId", wire->inputId);
    serializer->WriteField(U"outputId", wire->outputId);
    serializer->WriteField(U"nodes", wire->nodes.AsRef(), false);
}

void SerializeToJson(JsonSerializer* serializer, SerializedPart* serialized) {
    serializer->WriteField(U"version", serialized->version);
    serializer->WriteField(U"id", serialized->id);
    serializer->WriteField(U"type", serialized->type);
    serializer->WriteField(U"pTile", serialized->pTile);
    serializer->WriteField(U"pOffset", serialized->pOffset);
    serializer->WriteField(U"dim", serialized->dim);
    serializer->WriteField(U"active", serialized->active);
    serializer->WriteField(U"clockDiv", serialized->clockDiv);
    serializer->WriteField(U"label", serialized->label);
    serializer->WriteField(U"inputCount", serialized->inputCount);
    serializer->WriteField(U"outputCount", serialized->outputCount);
    serializer->WriteField(U"pinRelPositions", serialized->pinRelPositions.AsRef(), false);
}

struct PrototypeKey {
    const char* str;

    PrototypeKey() = default;
    explicit PrototypeKey(const char* s) : str(s) {};
};

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

struct JsonDeserializer {
    u32 prototypeAt = 0;
    HashMap<PrototypeKey, json_value_s*> prototypes[4];
    DArray<json_object_s*> parts;
    DArray<json_object_s*> wires;
    Allocator* destinationAllocator;

    SerializedPart scratchPart = {};
    SerializedWire scratchWire = {};

    JsonDeserializer() = default;
    JsonDeserializer(Allocator* allocator);

    void PreparePrototype(json_object_s* object);
};

bool ParseDeskDescription(JsonDeserializer* deserializer, const char* json, u32 lenZ);

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

char32* JsonTryGetString(JsonDeserializer* deserializer, json_value_s* value) {
    char32* result = nullptr;
    json_string_s* str = json_value_as_string(value);
    if (str) {
        // nocheckin
        // TODO: ensure str->string_size counts null
        auto builder = StringBuilder(deserializer->destinationAllocator, str->string, (usize)str->string_size, 0);
        result = builder.StealString();
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

char32* JsonTryGetString(JsonDeserializer* deserializer, const char* fieldName) {
    char32* result = nullptr;
    assert(deserializer->prototypeAt);
    json_value_s** entry = deserializer->prototypes[deserializer->prototypeAt - 1].Find(PrototypeKey(fieldName));
    if (entry) {
        result = JsonTryGetString(deserializer, *entry);
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
    deserializer->scratchPart = {};
    deserializer->scratchPart.pinRelPositions = tmp;

    auto version = JsonTryGetU32(deserializer, "version"); if (!version.hasValue) goto notOk;
    auto id = JsonTryGetU32(deserializer, "id"); if (!id.hasValue) goto notOk;
    auto type = JsonTryGetU32(deserializer, "type"); if (!type.hasValue) goto notOk;
    auto pTile = JsonTryGetVector<i32, 2>(deserializer, "pTile"); if (!pTile.hasValue) goto notOk;
    auto pOffset = JsonTryGetVector<f32, 2>(deserializer, "pOffset"); if (!pOffset.hasValue) goto notOk;
    auto dim = JsonTryGetVector<i32, 2>(deserializer, "dim"); if (!dim.hasValue) goto notOk;
    auto active = JsonTryGetBool(deserializer, "active"); if (!active.hasValue) goto notOk;
    auto clockDiv = JsonTryGetU32(deserializer, "clockDiv"); if (!clockDiv.hasValue) goto notOk;
    auto label = JsonTryGetString(deserializer, "label"); if (!label) goto notOk;
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
    deserializer->scratchPart.label = label;
    deserializer->scratchPart.inputCount = inputCount.value;
    deserializer->scratchPart.outputCount = outputCount.value;

    return true;

notOk:

    if (label) deserializer->destinationAllocator->Dealloc(label);
    deserializer->scratchPart.pinRelPositions.Clear();

    return false;
}

bool DeserializeWire(JsonDeserializer* deserializer) {
    auto tmp = deserializer->scratchWire.nodes;
    deserializer->scratchWire = {};
    deserializer->scratchWire.nodes = tmp;

    auto version = JsonTryGetU32(deserializer, "version"); if (!version.hasValue) goto notOk;
    auto inputId = JsonTryGetU32(deserializer, "inputId"); if (!inputId.hasValue) goto notOk;
    auto outputId = JsonTryGetU32(deserializer, "outputId"); if (!outputId.hasValue) goto notOk;

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
    deserializer->scratchWire.outputId = outputId.value;

    return true;

notOk:
    deserializer->scratchWire.nodes.Clear();

    return false;
}
