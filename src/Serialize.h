#pragma once

#include "Part.h"
#include "StringBuilder.h"

struct JsonSerializer {
    static const usize DefaultBufferSize = Megabytes(2);
    StringBuilder builder;
    DArray<char> outputBuffer;
    i32 indentLevel;
    bool inlineMode;

    void Init(Allocator* allocator);

    void Clear();

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
    u32 inputPinIndex;
    u32 outputId;
    u32 outputPinIndex;
    DArray<DeskPosition> nodes;
};

void SerializePart(Part* part, SerializedPart* serialized) {
    serialized->version = 0;
    serialized->id = part->id;
    serialized->type = (u32)part->type;
    serialized->pTile = part->p.cell;
    serialized->pOffset = part->p.offset;
    serialized->dim = part->dim;
    serialized->active = part->active;
    serialized->clockDiv = part->clockDiv;
    serialized->label = part->label;
    serialized->inputCount = part->inputCount;
    serialized->outputCount = part->outputCount;
    ForEach(&part->pins, pin) {
        serialized->pinRelPositions.PushBack(pin->pRelative);
    } EndEach;
}

void SerializeWire(Wire* wire, SerializedWire* serialized) {
    serialized->version = 0;
    serialized->inputId = wire->input->part->id;
    serialized->inputPinIndex = wire->input->part->pins.IndexFromPtr(wire->input);
    serialized->outputId = wire->output->part->id;
    serialized->outputPinIndex = wire->output->part->pins.IndexFromPtr(wire->output) - wire->output->part->inputCount;
    ForEach(&wire->nodes, node) {
        serialized->nodes.PushBack(*node);
    } EndEach;
}

void SerializeToJson(JsonSerializer* serializer, SerializedWire* wire) {
    serializer->WriteField(U"version", wire->version);
    serializer->WriteField(U"inputId", wire->inputId);
    serializer->WriteField(U"inputPinIndex", wire->inputPinIndex);
    serializer->WriteField(U"outputId", wire->outputId);
    serializer->WriteField(U"outputPinIndex", wire->outputPinIndex);
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

u32 HashU32(PrototypeKey& key);
bool HashCompareKeys(PrototypeKey& a, PrototypeKey& b);

struct JsonDeserializer {
    u32 prototypeAt = 0;
    HashMap<PrototypeKey, json_value_s*> prototypes[4];
    DArray<json_object_s*> parts;
    DArray<json_object_s*> wires;
    Allocator* destinationAllocator;

    SerializedPart scratchPart;
    SerializedWire scratchWire;

    void Init(Allocator* allocator);
};

bool ParseDeskDescription(JsonDeserializer* deserializer, const char* json, u32 lenZ);

HashMap<PrototypeKey, json_value_s*>* JsonPushObject(JsonDeserializer* deserializer, json_object_s* obj);
void JsonPopObject(JsonDeserializer* deserializer);

Option<u32> JsonTryGetU32(JsonDeserializer* deserializer, json_value_s* value);
Option<i32> JsonTryGetI32(JsonDeserializer* deserializer, json_value_s* value);
Option<f32> JsonTryGetF32(JsonDeserializer* deserializer, json_value_s* value);
Option<bool> JsonTryGetBool(JsonDeserializer* deserializer, json_value_s* value);
Option<char32*> JsonTryGetString(JsonDeserializer* deserializer, json_value_s* value);
json_array_s* JsonTryGetArray(JsonDeserializer* deserializer, json_value_s* value);
json_object_s* JsonTryGetObject(JsonDeserializer* deserializer, json_value_s* value);
template <typename T, u32 Size>
Option<Vector<T, Size>> JsonTryGetVector(JsonDeserializer* deserializer, json_value_s* value);

Option<i32> JsonTryGetI32(JsonDeserializer* deserializer, const char* fieldName);
Option<u32> JsonTryGetU32(JsonDeserializer* deserializer, const char* fieldName);
Option<f32> JsonTryGetF32(JsonDeserializer* deserializer, const char* fieldName);
Option<bool> JsonTryGetBool(JsonDeserializer* deserializer, const char* fieldName);
Option<char32*> JsonTryGetString(JsonDeserializer* deserializer, const char* fieldName);
json_array_s* JsonTryGetArray(JsonDeserializer* deserializer, const char* fieldName);
template <typename T, u32 Size>
Option<Vector<T, Size>> JsonTryGetVector(JsonDeserializer* deserializer, const char* fieldName);

bool DeserializePart(JsonDeserializer* deserializer);
bool DeserializeWire(JsonDeserializer* deserializer);
