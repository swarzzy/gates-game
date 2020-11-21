#pragma once

#define ForEach(array, it) do { for (u32 _index_ = 0; _index_ < (array)->Count(); _index_++) { auto it = (array)->Data() + _index_;
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
    T* Last();
    T* First() { return data; }

    u32 Count() { return count; }
    u32 Capacity() { return capacity; }
    T* Data() { return data; }

    Array Clone();
    void CopyTo(Array<T>* other);
    void CopyTo(Array<T>* other, u32 copyCount);
    void FreeBuffers();
    void Clear();
    void Fill(T& value);
    void Append(const Array<T>* other);
    void Append(T* data, u32 n);
    void Prepend(const Array<T>* other);
    void Prepend(T* data, u32 n);

    void Resize(u32 newSize);
    void Reserve(u32 newCapacity);
    // Resize a vector to a smaller size, guaranteed not to cause a reallocation
    void Shrink(u32 newSize);
    // Resize a vector and reallocate storage
    void ShrinkBuffers(u32 newSize);

    // TODO: N - versions are not tested
    T* PushBack();
    T* PushFront();
    T* PushBackN(u32 n);
    T* PushFrontN(u32 n);
    void PushFront(const T& v);
    void PushBack(const T& v);
    void Pop();
    void Erase(const T* it);
    void EraseUnsorted(const T* it);
    T* Insert(u32 index);
    T* InsertN(u32 index, u32 n);
    void Insert(u32 index, const T& v);

    // Some crazy functional stuff
    void Reverse();

    template <typename Fn>
    u32 CountIf(Fn callback);

    template <typename Fn>
    void Each(Fn callback);

    u32 IndexFromPtr(const T* it);

    u32 _GrowCapacity(u32 sz);
};
