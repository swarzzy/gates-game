#pragma once

#define ForEachInHash(map, it) do {\
    usize concat(__visited, it) = 0;\
    usize concat(__map_capacity, it) = (map)->Capacity();\
    for (usize concat(concat(_index_, it), _) = 0; concat(concat(_index_, it), _) < (map)->Capacity(); concat(concat(_index_, it), _)++) { \
    if ((map)->occupancyTable[concat(concat(_index_, it), _)]) {\
    auto it = (map)->valuesTable + concat(concat(_index_, it), _);

#define  EndEachInHash(it) concat(__visited, it)++; if (concat(__visited, it) == concat(__map_capacity, it)) break; } } } while (false)


template <typename Key, typename Value>
struct HashMap {
    Allocator* allocator = nullptr;
    b8* occupancyTable = nullptr;
    Key* keysTable = nullptr;
    Value* valuesTable = nullptr;
    usize capacity = 0;
    usize count = 0;

    HashMap() = default;
    explicit HashMap(Allocator* a) : allocator(a) {};

    usize Count() { return count; }
    usize Capacity() { return capacity; }

    void Reserve(usize sz);

    void FreeBuffers();
    void Clear();

    Value* Add(Key& key);
    Value* Find(Key& key);
    bool Delete(Key& key);

    void _AllocateBuffers(usize sz);

    template <bool SerachForEmpty>
    Tuple<bool, usize> _FindEntry(Key& key);
};
