#include "HashMap.h"

template <typename Key, typename Value>
void HashMap<Key, Value>::Reserve(usize sz) {
    if (sz > capacity) {
        auto newMap = HashMap<Key, Value>(allocator);
        newMap._AllocateBuffers(sz);

        if (count) {
            for (usize i = 0; i < capacity; i++) {
                if (occupancyTable[i]) {
                    Value* v = newMap.Add(keysTable[i]);
                    assert(v);
                    *v = valuesTable[i];
                }
            }
        }

        assert(count == newMap.count);

        FreeBuffers();
        *this = newMap;

        assert(IsPowerOfTwo((u32)capacity));
    }
}

template <typename Key, typename Value>
void HashMap<Key, Value>::FreeBuffers() {
    if (capacity) {
        allocator->Dealloc(occupancyTable);
        allocator->Dealloc(keysTable);
        allocator->Dealloc(valuesTable);
        occupancyTable = nullptr;
        keysTable = nullptr;
        valuesTable = nullptr;
        capacity = 0;
        count = 0;
    }
}

template <typename Key, typename Value>
void HashMap<Key, Value>::Clear() {
    count = 0;
    memset(occupancyTable, 0, sizeof(b8) * capacity);
}

template <typename Key, typename Value>
Value* HashMap<Key, Value>::Add(Key& key) {
    Value* result = nullptr;
    if (capacity == 0) {
        Reserve(16);
    } else {
        f32 load = (f32)(count + 1) / (f32)capacity;
        if (load > 0.8f) {
            Reserve(capacity * 2);
        }
    }

    assert(count < capacity);

    auto[found, index] = _FindEntry<true>(key);
    assert(found);
    occupancyTable[index] = true;
    keysTable[index] = key;
    result = valuesTable + index;
    count++;

    return result;
}

template <typename Key, typename Value>
Value* HashMap<Key, Value>::Find(Key& key) {
    Value* result = nullptr;
    auto[found, index] = _FindEntry<false>(key);
    if (found) {
        result = valuesTable + index;
    }
    return result;
}

template <typename Key, typename Value>
bool HashMap<Key, Value>::Delete(Key& key) {
    bool result = false;
    auto[found, index] = _FindEntry<false>(key);
    if (found) {
        assert(count > 0);
        count--;

        occupancyTable[index] = false;
        result = true;
    }
    return result;
}

template <typename Key, typename Value>
void HashMap<Key, Value>::_AllocateBuffers(usize sz) {
    assert(!occupancyTable);
    assert(!keysTable);
    assert(!valuesTable);
    assert(!capacity);
    assert(!count);
    occupancyTable = allocator->Alloc<b8>(sz, true);
    keysTable = allocator->Alloc<Key>(sz, false);
    valuesTable = allocator->Alloc<Value>(sz, false);
    capacity = sz;
}

template <typename Key, typename Value>
template <bool SerachForEmpty>
Tuple<bool, usize> HashMap<Key, Value>::_FindEntry(Key& key) {
    compile_if (SerachForEmpty) {
        if (capacity == count) {
            return MakeTuple(false, (usize)0);
        }
    }

    u32 hashMask = capacity - 1;
    u32 hash = HashU32(key);
    u32 firstIndex = hash & hashMask;
    for (u32 offset = 0; offset < (u32)capacity; offset++) {
        u32 index = (firstIndex + offset) & hashMask;

        compile_if(SerachForEmpty) {
            if (!occupancyTable[index]) {
                return MakeTuple(true, (usize)index);
            }
        } else {
            if (occupancyTable[index] && HashCompareKeys(keysTable[index], key)) {
                return MakeTuple(true, (usize)index);
            }
        }
    }

    return MakeTuple(false, (usize)0);
}
