#include "StringBuilder.h"

template <typename Char>
StringBuilderT<Char>::StringBuilderT(Allocator* alloc)
    : allocator(alloc) {
}

template <typename Char>
Char* StringBuilderT<Char>::_AllocateBuffer(u32 count) {
    u8* mem = (u8*)allocator->Alloc(sizeof(Char) * count + BufferPadding, false);
    Char* buf = (Char*)(mem + BufferPadding);
    assert(((uptr)buf % sizeof(Char)) == 0);
    return buf;
}

template <typename Char>
void StringBuilderT<Char>::_DeallocateBuffer() {
    u8* mem = (u8*)buffer - BufferPadding;
    allocator->Dealloc(mem);
}

template <typename Char>
StringBuilderT<Char>::StringBuilderT(Allocator* alloc, const Char* str, usize extraSpace) {
    usize lenZ = StringLengthZ(str);

    allocator = alloc;

    bufferCount = lenZ + extraSpace;
    buffer = _AllocateBuffer(bufferCount);
    assert(buffer);

    memcpy(buffer, str, sizeof(Char) * lenZ);
    buffer[lenZ - 1] = 0;
    at = lenZ - 1;
    free = extraSpace;
}

template <typename Char>
StringBuilderT<Char>::StringBuilderT(Allocator* alloc, const Char* str, usize lenZ, usize extraSpace) {
    allocator = alloc;

    bufferCount = lenZ + extraSpace;
    buffer = _AllocateBuffer(bufferCount);
    assert(buffer);

    memcpy(buffer, str, sizeof(Char) * lenZ);
    buffer[lenZ - 1] = 0;
    at = lenZ - 1;
    free = extraSpace;
}

template <typename Char>
void StringBuilderT<Char>::Reserve(usize size) {
    if (bufferCount < size) {
        Char* newBuffer = _AllocateBuffer(size);
        assert(newBuffer);

        if (at) {
            memcpy(newBuffer, buffer, sizeof(Char) * at + 1);
        }

        _DeallocateBuffer();
        buffer = newBuffer;
        bufferCount = size;
        free = bufferCount - at - 1;
    }
}

template <typename Char>
void StringBuilderT<Char>::FreeBuffers() {
    if (buffer) {
        _DeallocateBuffer(buffer);
        Allocator* alloc = allocator;
        *this = {};
        allocator = alloc;
    }
}

template <typename Char>
void StringBuilderT<Char>::Clear() {
    builder->at = 0;
    builder->free = builder->bufferCount;
    builder->buffer[0] = (Char)0;
}

template <typename Char>
StringT<Char> StringBuilderT<Char>::StealString() {
    StringT<Char> result = StringT<Char>();

    if (buffer) {
        result = StringT<Char>::MakeFromRawBuffer((u8*)buffer - BufferPadding, at, bufferCount);
        Allocator* alloc = allocator;
        *this = {};
        allocator = alloc;
    }

    return result;
}

template <typename Char>
StringT<Char> StringBuilderT<Char>::CopyString() {
    StringT<Char> result = StringT<Char>();

    if (buffer) {
        Char* copyBuffer = _AllocateBuffer(at + 1);
        memcpy(copyBuffer, buffer, sizeof(Char) * (at + 1));
        result = StringT<Char>::MakeFromRawBuffer((u8*)copyBuffer - BufferPadding, at, bufferCount);
    }

    return result;
}

template <typename Char>
void StringBuilderT<Char>::Append(const Char* str, usize lenZ) {
    if (free < (lenZ - 1)) {
        Reserve(Max(bufferCount + lenZ - 1 - free, bufferCount * 2));
        assert(free >= (lenZ - 1));
    }

    memcpy(buffer + at, str, sizeof(Char) * lenZ);
    at += lenZ - 1;
    free -= lenZ - 1;
}

template <typename Char>
void StringBuilderT<Char>::Append(const Char* str) {
    Append(str, StringLengthZ(str));
}

template <typename Char>
void StringBuilderT<Char>::Append(i32 i) {
    if constexpr (EqualTypes<Char, char>) {
        Char tmp[64];
        _ltoa((long)i, tmp, 10);
        Append(tmp);
    } else {
        static_assert(false);
    }
}

template <typename Char>
void StringBuilderT<Char>::Append(u32 u) {
    if constexpr (EqualTypes<Char, char>) {
        Char tmp[64];
        _ultoa((unsigned long)u, tmp, 10);
        Append(tmp);
    } else {
        static_assert(false);
    }
}

template <typename Char>
void StringBuilderT<Char>::Append(f64 f) {
    if constexpr (EqualTypes<Char, char>) {
        Char tmp[_CVTBUFSIZE];
        // TODO: _gcvt is not thread safe
        // TODO: Do something with this magic digits param
        auto ret = _gcvt(f, 8, tmp);
        assert(ret);
        Append(tmp);
    } else {
        static_assert(false);
    }
}

template <typename Char>
void StringBuilderT<Char>::Append(f32 f) {
    Append((f64)f);
}
