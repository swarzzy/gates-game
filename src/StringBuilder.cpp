#include "StringBuilder.h"

void StringBuilder::_Init(Allocator* alloc, usize count) {
    allocator = alloc;

    bufferCount = count;
    buffer = allocator->Alloc<char32>(bufferCount, false);
    assert(buffer);
    buffer[0] = 0;

    at = 0;
    free = count - 1;
}


StringBuilder::StringBuilder(Allocator* alloc)
    : allocator(alloc) {
}

StringBuilder::StringBuilder(Allocator* alloc, const char32* str, usize extraSpace) {
    usize size = StringLengthZ(str);
    _Init(alloc, size + extraSpace);
    Append(str, size);
}

StringBuilder::StringBuilder(Allocator* alloc, const char32* str, usize sizeZ, usize extraSpace) {
    _Init(alloc, sizeZ + extraSpace);
    Append(str, sizeZ);
}

StringBuilder::StringBuilder(Allocator* alloc, const char* str, usize lenZ, usize extraSpace) {
    _Init(alloc, lenZ + extraSpace);
    Append(str, lenZ);
}

StringBuilder::StringBuilder(Allocator* alloc, const char* str, usize extraSpace) {
    usize size = StringLengthZ(str);
    _Init(alloc, size + extraSpace);
    Append(str, size);
}

void StringBuilder::Reserve(usize size) {
    if (bufferCount < size) {
        char32* newBuffer = allocator->Alloc<char32>(size, false);
        assert(newBuffer);

        if (at) {
            memcpy(newBuffer, buffer, (at + 1) * sizeof(char32));
        }

        allocator->Dealloc(buffer);
        buffer = newBuffer;
        bufferCount = size;
        free = bufferCount - at - 1;
    }
}

void StringBuilder::FreeBuffers() {
    if (buffer) {
        allocator->Dealloc(buffer);
        Allocator* alloc = allocator;
        *this = {};
        allocator = alloc;
    }
}

void StringBuilder::Clear() {
    at = 0;
    free = bufferCount;
    buffer[0] = (char32)0;
}

char32* StringBuilder::StealString() {
    char32* result = buffer;

    Allocator* alloc = allocator;
    *this = {};
    allocator = alloc;

    return result;
}

char32* StringBuilder::CopyString() {
    char32* result = buffer ? ::CopyString(buffer, at + 1, allocator) : nullptr;
    return result;
}

char* StringBuilder::CopyStringAsASCII() {
    usize copySize = at + 1;
    char* copyBuffer = allocator->Alloc<char>(copySize, false);

    for (usize i = 0; i < copySize; i++) {
        copyBuffer[i] = (char)buffer[i];
    }

    return copyBuffer;
}

void StringBuilder::Append(const char32* str, usize sizeZ) {
    if (free < (sizeZ - 1)) {
        Reserve(Max(bufferCount + sizeZ - 1 - free, bufferCount * 2));
        assert(free >= (sizeZ - 1));
    }

    memcpy(buffer + at, str, (sizeZ - 1) * sizeof(char32));
    at += sizeZ - 1;
    free -= sizeZ - 1;
    buffer[at] = 0;
}

void StringBuilder::Append(const char32* str) {
    Append(str, StringLengthZ(str));
}

void StringBuilder::Append(const char* str, usize lenZ) {
    if (str && *str && lenZ) {
        char32 inplaceBuffer[128];
        usize actualLength = 1;
        bool tooBig = false;
        bool valid = true;

        {
            // Validate string, count number of bytes and write to stack buffer if there is enough place
            u32 codepoint;
            u32 state = 0;
            for (; *str; str++) {
                if (!Utf8Decode(&state, &codepoint, *str)) {
                    inplaceBuffer[actualLength - 1] = (char32)codepoint;
                    actualLength++;
                    if (actualLength >= array_count(inplaceBuffer)) {
                        tooBig = true;
                    }
                }
            }

            if (state != UTF8_ACCEPT) {
                valid = false;
            }
        }

        if (valid) {
            if (!tooBig) {
                Append(inplaceBuffer, actualLength);
            } else {
                if (free < (actualLength - 1)) {
                    Reserve(Max(bufferCount + actualLength - 1 - free, bufferCount * 2));
                    assert(free >= (actualLength - 1));
                }

                {
                    // Convert again and push to builder buffer
                    u32 codepoint;
                    u32 state = 0;
                    for (; *str; str++) {
                        if (!Utf8Decode(&state, &codepoint, *str)) {
                            buffer[at] = (char32)codepoint;
                            at++;
                            free--;
                        }
                    }

                    at--;
                    free++;
                    buffer[at] = 0;


                    assert(state == UTF8_ACCEPT);
                }
            }
        }
    }
}

void StringBuilder::Append(const char* str) {
    if (str) {
        Append(str, StringLengthZ(str));
    }
}

void StringBuilder::Append(i32 i) {
    char tmp[64];
    _ltoa((long)i, tmp, 10);
    Append(tmp);
}

void StringBuilder::Append(u32 u) {
    char tmp[64];
    _ultoa((unsigned long)u, tmp, 10);
    Append(tmp);
}

void StringBuilder::Append(f64 f, usize fracDigits) {
    char tmp[256];
    int _fracDigits = Max((usize)0, Min((usize)24, fracDigits));
    stbsp_snprintf(tmp, 256, "%.*g", _fracDigits, f);
    Append(tmp);
}

void StringBuilder::Append(f32 f, usize fracDigits) {
    Append((f64)f, fracDigits);
}

void StringBuilder::Append(bool b) {
    if (b) {
        const char32* v = U"true";
        Append(v, StringLengthZ(v));
    } else {
        const char32* v = U"false";
        Append(v, StringLengthZ(v));
    }
}
