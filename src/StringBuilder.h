#pragma once

#include "Common.h"
#include "String.h"

// TODO: Wide strings support
template <typename T> struct StringBuilderT;
typedef StringBuilderT<char> StringBuilder;
typedef StringBuilderT<char16> StringBuilderW;

template <typename Char>
struct StringBuilderT {
    static const usize BufferPadding = sizeof(u16) * 2;
    Char* buffer = nullptr;
    Allocator* allocator = nullptr;
    usize bufferCount = 0;
    usize at = 0;
    usize free = 0;

    StringBuilderT() = default;
    explicit StringBuilderT(Allocator* allocator);
    StringBuilderT(Allocator* allocator, const Char* str, usize lenZ, usize extraSpace = 0);
    StringBuilderT(Allocator* allocator, const Char* str, usize extraSpace = 0);

    void Reserve(usize size);

    void FreeBuffers();
    void Clear();

    StringT<Char> StealString();
    StringT<Char> CopyString();

    //StringBuilder ConvertToANSI();
    //StringBuilderW ConvertToUnicode();

#define AppendLiteral(literal) Append(literal, sizeof(literal))
    void Append(const Char* str, usize lenZ);
    void Append(const Char* str);
    void Append(i32 i);
    void Append(u32 u);
    void Append(f64 f);
    void Append(f32 f);

    // Internal
    Char* _AllocateBuffer(u32 count);
    void _DeallocateBuffer();
};
