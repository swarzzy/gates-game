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

    memcpy(buffer + at, str, sizeZ * sizeof(char32));
    at += sizeZ - 1;
    free -= sizeZ - 1;
    buffer[at] = 0;
}

void StringBuilder::Append(const char32* str) {
    Append(str, StringLengthZ(str));
}

void StringBuilder::Append(const char* str, usize lenZ) {
    if (free < (lenZ - 1)) {
        Reserve(Max(bufferCount + lenZ - 1 - free, bufferCount * 2));
        assert(free >= (lenZ - 1));
    }

    for (usize i = 0; i < lenZ; i++) {
        buffer[at] = str[i];
        at++;
        free--;
    }

    at--;
    free++;

    buffer[at] = 0;
}

void StringBuilder::Append(const char* str) {
    Append(str, StringLengthZ(str));
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

void StringBuilder::Append(f64 f) {
    char tmp[_CVTBUFSIZE];
    // TODO: _gcvt is not thread safe
    // TODO: Do something with this magic digits param
    auto ret = _gcvt(f, 8, tmp);
    assert(ret);
    Append(tmp);
}

void StringBuilder::Append(f32 f) {
    Append((f64)f);
}
