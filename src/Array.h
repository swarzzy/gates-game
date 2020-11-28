#pragma once

#define ForEach(array, it) do { for (u32 concat(concat(_index_, it), _) = 0; concat(concat(_index_, it), _) < (array)->_Count(); concat(concat(_index_, it), _)++) { auto it = (array)->_DataPtr() + concat(concat(_index_, it), _);
#define EndEach } } while(false)

template <typename T>
struct Array;

template <typename T>
struct ArrayRef;

template <typename T>
using DArray = Array<T>;

template <typename T, typename Derived>
struct ArrayBase {
    T& operator[](u32 i);
    T* At(u32 index);

    ArrayRef<T> AsRef();

    T* Last();
    T* First() { return data; }

    void Fill(T& value);

    u32 IndexFromPtr(const T* it);

    void Reverse();

    template <typename Fn>
    u32 CountIf(Fn callback);

    template <typename Fn>
    void Each(Fn callback);

    template <typename Fn>
    T* FindFirst(Fn callback);

    forceinline T* _DataPtr();
    forceinline u32 _Count();
};

template <typename T>
struct ArrayRef : ArrayBase<T, ArrayRef<T>> {
    T* data;
    u32 count;
    forceinline T* Data() { return data; }
    forceinline u32 Count() { return count; }
    forceinline ArrayRef(T* Data, u32 Count) : data(Data), count(Count) {}

    static ArrayRef Empty() { return ArrayRef(nullptr, 0); }
};

template <typename T, u32 Size>
struct SArray : ArrayBase<T, SArray<T, Size>> {
    T data[Size];
    forceinline T* Data() { return data; }
    forceinline u32 Count() { return Size; }
};

template <typename T>
struct Array : ArrayBase<T, Array<T>> {
    T* data = nullptr;
    u32 count = 0;
    u32 capacity = 0;
    Allocator* allocator = nullptr;

    Array() = default;
    Array(Allocator* alloc) : allocator(alloc) {}
    Array(Allocator* alloc, u32 size) : allocator(alloc) { Resize(size); }

    forceinline T* Data() { return data; }
    forceinline u32 Count() { return count; }

    u32 Capacity() { return capacity; }

    Array Clone();
    void CopyTo(Array<T>* other);
    void CopyTo(Array<T>* other, u32 copyCount);

    void Clear();
    void FreeBuffers();

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
    void Erase(u32 index);
    void EraseUnsorted(u32 index);


    T* Insert(u32 index);
    T* InsertN(u32 index, u32 n);
    void Insert(u32 index, const T& v);

    void KillDuplicatesUnsorted();

    u32 _GrowCapacity(u32 sz);
};
