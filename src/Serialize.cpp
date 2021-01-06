#include "Serialize.h"

JsonSerializer::JsonSerializer(Allocator* allocator) {
    builder = StringBuilder(allocator);
    outputBuffer = DArray<char>(allocator);
    builder.Reserve(DefaultBufferSize / sizeof(char32));
}

void JsonSerializer::_Indent() {
    for (i32 i = 0; i < indentLevel; i++) {
        builder.Append(U"    ");
    }
}

void JsonSerializer::BeginArray() {
    _Indent();
    builder.Append(U"[\n");
    indentLevel++;
}

void JsonSerializer::EndArray() {
    assert(indentLevel > 0);
    if (indentLevel > 0) {
        indentLevel--;
        builder.Append(U"\n");
        _Indent();
        builder.Append(U"],\n");
    }
}

void JsonSerializer::BeginStruct() {
    _Indent();
    builder.Append(U"{\n");
    indentLevel++;
}

void JsonSerializer::EndStruct(bool comma) {
    assert(indentLevel > 0);
    if (indentLevel > 0) {
        indentLevel--;
        _Indent();
        builder.Append(U"}");
        if (comma) builder.Append(U",");
        //else builder.Append(U"\n");
    }
}

template <typename T>
void JsonSerializer::WriteField(const char32* name, T value, bool comma) {
    _Indent();
    builder.Append(U"\"");
    builder.Append(name);
    builder.Append(U"\": ");
    WriteValue(value, true, comma);
}

template <typename T>
void JsonSerializer::_WriteValue(T value, bool newLine, bool quoted, bool comma) {
    if (quoted) builder.Append("\"");
    builder.Append(value);
    if (quoted) builder.Append("\"");
    if (comma) builder.Append(", ");
    if (newLine) builder.Append(U"\n");
}

void JsonSerializer::WriteValue(u32 value, bool newLine, bool comma) {
    _WriteValue(value, newLine, false, comma);
}

void JsonSerializer::WriteValue(i32 value, bool newLine, bool comma) {
    _WriteValue(value, newLine, false, comma);
}

void JsonSerializer::WriteValue(f32 value, bool newLine, bool comma) {
    _WriteValue(value, newLine, false, comma);
}

void JsonSerializer::WriteValue(bool value, bool newLine, bool comma) {
    _WriteValue(value, newLine, false, comma);
}

void JsonSerializer::WriteValue(const char32* value, bool newLine, bool comma) {
    _WriteValue(value, newLine, true, comma);
}

template <typename T, u32 Size>
void JsonSerializer::WriteValue(Vector<T, Size> value, bool newLine, bool comma) {
    builder.Append("[ ");
    for (u32 i = 0; i < Size; i++) {
        _WriteValue(value.data[i], false, false, i != (Size - 1));
    }

    builder.Append(" ]");
    if (comma) builder.Append(", ");
    if (newLine) builder.Append("\n");
}

template <typename T>
void JsonSerializer::WriteValue(ArrayRef<T> value, bool newLine, bool comma) {
    builder.Append("[ \n");
    indentLevel++;
    for (u32 i = 0; i < value.Count(); i++) {
        _Indent();
        WriteValue(value.Data()[i], true, i != (value.Count() - 1));
    }
    indentLevel--;
    _Indent();

    builder.Append(" ]");
    if (comma) builder.Append(", ");
    if (newLine) builder.Append("\n");
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
