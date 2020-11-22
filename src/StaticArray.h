#pragma once

template <typename T, u32 Size>
struct StaticArray {
    T data[Size];

    T& operator[](u32 i) { return data[i]; }

    u32 Count() { return Size; }
    T* Data() { return data; }
};
