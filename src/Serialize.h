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
    u32 inputId;
    u32 outputId;
    DArray<DeskPosition> nodes;
};

void SerializeToJson(JsonSerializer* serializer, SerializedWire* wire) {
    serializer->WriteField(U"inputId", wire->inputId);
    serializer->WriteField(U"outputId", wire->outputId);
    serializer->WriteField(U"nodes", wire->nodes.AsRef(), false);
}

void SerializeToJson(JsonSerializer* serializer, SerializedPart* serialized) {
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
