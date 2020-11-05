#pragma once

#define ForEach(array, it) do { for (u32 index = 0; index < (array)->Count(); index++) { auto it = (array)->Data() + index;
#define EndEach } } while(false)

template <typename T>
struct Array {
    T* data = nullptr;
    u32 count = 0;
    u32 capacity = 0;
    Allocator* allocator = nullptr;

    Array() = default;
    Array(Allocator* alloc) : allocator(alloc) {}
    Array(Allocator* alloc, u32 size) : allocator(alloc) { Resize(size); }

    T& operator[](u32 i);

    u32 Count() { return count; }
    u32 Capacity() { return capacity; }
    T* Data() { return data; }

    Array Clone();
    void FreeBuffers();
    void Clear();
    void Fill(T& value);

    void Resize(u32 newSize);
    void Reserve(u32 newCapacity);
    // Resize a vector to a smaller size, guaranteed not to cause a reallocation
    void Shrink(u32 newSize);
    // Resize a vector and reallocate storage
    void ShrinkBuffers(u32 newSize);


    T* Push();
    void Push(const T& v);
    void Pop();
    void Erase(const T* it);
    void EraseUnsorted(const T* it);
    T* Insert(u32 index);
    void Insert(u32 index, const T& v);

    u32 IndexFromPtr(const T* it);

    u32 _GrowCapacity(u32 sz);
};
