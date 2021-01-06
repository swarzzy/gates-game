#pragma once

#include "Part.h"

struct JsonSerializer {
    const usize DefaultBufferSize = Megabytes(2);
    StringBuilder builder = StringBuilder();
    DArray<char> outputBuffer = DArray<char>();
    i32 indentLevel = 0;

    JsonSerializer() = default;
    JsonSerializer(Allocator* allocator);

    void BeginArray();
    void EndArray();
    void BeginStruct();
    void EndStruct(bool comma);

    void WriteValue(const char32* value, bool newLine, bool comma);
    void WriteValue(u32 value, bool newLine, bool comma);
    void WriteValue(i32 value, bool newLine, bool comma);
    void WriteValue(f32 value, bool newLine, bool comma);
    void WriteValue(bool value, bool newLine, bool comma);
    template <typename T, u32 Size>
    void WriteValue(Vector<T, Size> value, bool newLine, bool comma);
    template <typename T>
    void WriteValue(ArrayRef<T> value, bool newLine, bool comma);


#define WriteFieldM(var, member, comma) WriteField(U#member, var->member, comma);

    template <typename T>
    void WriteField(const char32* name, T value, bool comma);

    ArrayRef<char> GenerateStringUtf8();

    template <typename T>
    void _WriteValue(T value, bool newLine, bool quoted, bool comma);


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

void SerializeToJson(JsonSerializer* serializer, SerializedPart* serialized) {
    serializer->BeginStruct();
    serializer->WriteFieldM(serialized, id, true);
    serializer->WriteFieldM(serialized, type, true);
    serializer->WriteFieldM(serialized, pTile, true);
    serializer->WriteFieldM(serialized, pOffset, true);
    serializer->WriteFieldM(serialized, dim, true);
    serializer->WriteFieldM(serialized, active, true);
    serializer->WriteFieldM(serialized, clockDiv, true);
    serializer->WriteFieldM(serialized, label, true);
    serializer->WriteFieldM(serialized, inputCount, true);
    serializer->WriteFieldM(serialized, outputCount, true);
    serializer->WriteField(U"pinRelPositions", serialized->pinRelPositions.AsRef(), false);
    serializer->EndStruct(false);
}
