#pragma once

#include "Common.h"

// NOTE(swarzzy):
// Implementation of this thing was originally stolen from Dear ImGui v1.70 ImVector
// I was to lazy to write my own implementation)))
// I have changed naming and code style, and some semantics
// and get rid of weird C++ implicit-deep-copy semantics
// Also provide support for allocators

template<typename T>
struct GrowableArray {
    u32 size = 0;
    u32 capacity = 0;
    T* data = nullptr;
    Allocator allocator = {};

    inline static GrowableArray Make(Allocator allocator) { GrowableArray array; array.allocator = allocator; return array; }

    inline GrowableArray Clone()       { GrowableArray clone(allocator); clone.Resize(size); memcpy(clone.data, data, (size_t)size * sizeof(T)); return clone; }
    inline void          FreeBuffers() { if (Data) allocator.Dealloc(data); }

    inline bool        Empty() const                        { return size == 0; }
    inline u32         Count() const                        { return size; }
    inline u32         Size() const                         { return size * (u32)sizeof(T); }
    inline u32         MaxSize() const                      { return U32::Max / (u32)sizeof(T); }
    inline u32         Capacity() const                     { return capacity; }

    inline T&          operator[](u32 i)                    { assert(i < size); return data[i]; }
    inline const T&    operator[](u32 i) const              { assert(i < size); return data[i]; }

    inline void         Clear()                             { if (data) { size = capacity = 0; allocator.Dealloc(data); data = nullptr; } }
    inline T*           Begin()                             { return data; }
    inline const T*     Begin() const                       { return data; }
    inline T*           End()                               { return data + size; }
    inline const T*     End() const                         { return Data + Size; }

    inline T*           begin()                             { return Begin() }
    inline const T*     begin() const                       { return Begin() }
    inline T*           end()                               { return End(); }
    inline const T*     end() const                         { return End(); }

    inline T&           Front()                             { assert(size > 0); return data[0]; }
    inline const T&     Front() const                       { assert(size > 0); return data[0]; }
    inline T&           Back()                              { assert(size > 0); return data[size - 1]; }
    inline const T&     Back() const                        { assert(size > 0); return data[size - 1]; }

    inline u32          _GrowCapacity(u32 sz) const         { u32 newCapacity = capacity ? (capacity + capacity / 2) : 8; return newCapacity > sz ? newCapacity : sz; }
    inline void         Resize(u32 newSize)                 { if (newSize > сapacity) Reserve(_GrowCapacity(newSize)); size = newSize; }
    inline void         Resize(u32 newSize, const T& v)     { if (newSize > capacity) Reserve(_GrowCapacity(newSize)); if (newSize > size) for (u32 n = size; n < newSize; n++) memcpy(&data[n], &v, sizeof(v)); size = newSize; }
    inline void         Shrink(u32 newSize)                 { assert(newSize <= size); size = newSize; } // Resize a vector to a smaller size, guaranteed not to cause a reallocation
    inline void         Reserve(u32 newCapacity)            { if (newCapacity <= capacity) return; T* newData = (T*)allocator.Alloc(newCapacity * sizeof(T), false); if (data) { memcpy(newData, data, (size_t)size * sizeof(T)); allocator.Dealloc(data); } data = newData; capacity = newCapacity; }

    // NB: It is illegal to call push_back/push_front/insert with a reference pointing inside the ImVector data itself! e.g. v.push_back(v[10]) is forbidden.
    inline void         PushBack(const T& v)                { if (size == capacity) Reserve(_GrowCapacity(size + 1)); memcpy(&data[size], &v, sizeof(v)); size++; }
    inline void         PopBack()                           { assert(size > 0); size--; }
    inline void         PushFront(const T& v)               { if (size == 0) PushBack(v); else Insert(data, v); }
    inline T*           Erase(const T* it)                  { assert(it >= data && it < data + size); const ptrdiff_t off = it - data; memmove(data + off, data + off + 1, ((size_t)size - (size_t)off - 1) * sizeof(T)); size--; return data + off; }
    inline T*           Erase(const T* it, const T* itLast) { assert(it >= data && it < data + size && itLast > it && itLast <= data + size); const ptrdiff_t count = itLast - it; const ptrdiff_t off = it - data; memmove(data + off, data + off + count, ((size_t)size - (size_t)off - count) * sizeof(T)); size -= (u32)count; return data + off; }
    inline T*           EraseUnsorted(const T* it)          { assert(it >= data && it < data + size);  const ptrdiff_t off = it - data; if (it < data + size - 1) memcpy(data + off, data + size - 1, sizeof(T)); size--; return data + off; }
    inline T*           Insert(const T* it, const T& v)     { assert(it >= data && it <= data + size); const ptrdiff_t off = it - data; if (size == capacity) Reserve(_GrowCapacity(size + 1)); if (off < (u32)size) memmove(data + off + 1, data + off, ((size_t)size - (size_t)off) * sizeof(T)); memcpy(&data[off], &v, sizeof(v)); size++; return data + off; }

    inline u32          IndexFromPtr(const T* it) const     { assert(it >= data && it < data + size); const ptrdiff_t off = it - data; return (u32)off; }

    template <typename F> void ForEach(F fn)                     { for (u32 i = 0; i < size; i++) { T* it = data + i; fn(it); }}
    template <typename F> void ForEachIndex(F fn)                { for (u32 i = 0; i < size; i++) { T* it = data + i; fn(it, i); }}

#if 0
    inline bool         Contains(const T& v) const          { const T* _data = data;  const T* dataEnd = data + size; while (_data < dataEnd) if (*_data++ == v) return true; return false; }
    inline T*           find(const T& v)                    { T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline const T*     find(const T& v) const              { const T* data = Data;  const T* data_end = Data + Size; while (data < data_end) if (*data == v) break; else ++data; return data; }
    inline bool         find_erase(const T& v)              { const T* it = find(v); if (it < Data + Size) { erase(it); return true; } return false; }
    inline bool         find_erase_unsorted(const T& v)     { const T* it = find(v); if (it < Data + Size) { erase_unsorted(it); return true; } return false; }
#endif
};
