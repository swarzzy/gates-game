#pragma once

#include "Common.h"
#include "String.h"

struct StringBuilder {
    char32* buffer = nullptr;
    Allocator* allocator = nullptr;
    usize bufferCount = 0;
    usize at = 0;
    usize free = 0;

    StringBuilder() = default;
    explicit StringBuilder(Allocator* allocator);
    StringBuilder(Allocator* allocator, const char32* str, usize lenZ, usize extraSpace = 0);
    StringBuilder(Allocator* allocator, const char32* str, usize extraSpace = 0);
    StringBuilder(Allocator* allocator, const char* str, usize lenZ, usize extraSpace = 0);
    StringBuilder(Allocator* allocator, const char* str, usize extraSpace = 0);

    const char32* Buffer() { return buffer; }
    usize Length() { return at; }
    usize LengthZ() { return at + 1; }
    usize CapacityZ() { return bufferCount; }
    usize At() { return at; }

    void Reserve(usize size);

    void FreeBuffers();
    void Clear();

    char32* StealString();
    char32* CopyString();
    char* CopyStringAsASCII();

#define AppendLiteral(literal) Append(literal, sizeof(literal))
    void Append(const char32* str, usize lenZ);
    void Append(const char32* str);
    void Append(const char* str, usize lenZ);
    void Append(const char* str);
    void Append(i32 i);
    void Append(u32 u);
    void Append(f64 f, usize fracDigits = 8);
    void Append(f32 f, usize fracDigits = 8);
    void Append(bool b);

    void _Init(Allocator* alloc, const usize count);
};
