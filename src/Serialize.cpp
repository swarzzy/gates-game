#include "Serialize.h"

JsonSerializer::JsonSerializer(Allocator* allocator) {
    builder = StringBuilder(allocator);
    outputBuffer = DArray<char>(allocator);
    builder.Reserve(DefaultBufferSize / sizeof(char32));
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

JsonDeserializer::JsonDeserializer(Allocator* allocator)
    : destinationAllocator(allocator) {
    for (u32 i = 0; i < array_count(prototypes); i++) {
        prototypes[i] = HashMap<PrototypeKey, json_value_s*>(allocator);
    }

    parts = DArray<json_object_s*>(allocator);
    wires = DArray<json_object_s*>(allocator);

    scratchPart.pinRelPositions = DArray<v2>(allocator);
    scratchWire.nodes = DArray<DeskPosition>(allocator);
}

bool ParseDeskDescription(JsonDeserializer* deserializer, const char* json, u32 lenZ) {
    bool result = true;

    deserializer->parts.Clear();
    deserializer->wires.Clear();

    json_parse_result_s parseResult;
    // TODO allocator
    json_value_s* root = json_parse_ex(json, lenZ - 1, json_parse_flags_allow_json5, nullptr, nullptr, &parseResult);
    if (!root) {
        result = false;
        return result;
    }

    defer {
        // TODO: if (root) free
        if (result == false) {
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
