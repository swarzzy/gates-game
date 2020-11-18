#include "Array.h"

template <typename T>
T& Array<T>::operator[](u32 i) {
    assert(i < count);
    return data[i];
}

template <typename T>
T* Array<T>::Last() {
    T* result = nullptr;
    if (count) {
        result = data + (count - 1);
    }
    return result;
}

template <typename T>
Array<T> Array<T>::Clone() {
    Array<T> clone (allocator);
    clone.Resize(count);
    memcpy(clone.data, data, (size_t)count * sizeof(T));
    return clone;
}

template <typename T>
void Array<T>::CopyTo(Array<T>* other) {
    CopyTo(other, count);
}

template <typename T>
void Array<T>::CopyTo(Array<T>* other, u32 copyCount) {
    if (copyCount > other->capacity) {
        other->Reserve(other->_GrowCapacity(copyCount));
    }
    memcpy(other->data, data, sizeof(T) * copyCount);
    other->count = copyCount;
}

template <typename T>
void Array<T>::Append(const Array<T>* other) {
    Append(other.data, other.count);
}

template <typename T>
void Array<T>::Append(T* data, u32 n) {
    if (n > 0) {
        T* mem = PushBackN(n);
        memcpy(mem, data, sizeof(T) * n);
    }
}

template <typename T>
void Array<T>::Prepend(const Array<T>* other) {
    Prepend(other.data, other.count);
}

template <typename T>
void Array<T>::Prepend(T* data, u32 n) {
    if (n > 0) {
        T* mem = PushFrontN(n);
        memcpy(mem, data, sizeof(T) * n);
    }
}


template <typename T>
void Array<T>::FreeBuffers() {
    if (data) {
        allocator->Dealloc(data);
    }
    count = 0;
    capacity = 0;
    data = nullptr;
}

template <typename T>
void Array<T>::Clear() {
    count = 0;
}

template <typename T>
void Array<T>::Fill(T& value) {
    for (u32 i = 0; i < count; i++) {
        memcpy(data + i, &value, sizeof(T));
    }
}

template <typename T>
void Array<T>::Resize(u32 newSize) {
    if (newSize > capacity) {
        Reserve(_GrowCapacity(newSize));
    }
    count = newSize;
}

template <typename T>
void Array<T>::Reserve(u32 newCapacity) {
    if (newCapacity > capacity) {
        T* newData = allocator->Alloc<T>(newCapacity, false);
        if (data) {
            memcpy(newData, data, (size_t)count * sizeof(T));
            allocator->Dealloc(data);
        }
        data = newData;
        capacity = newCapacity;
    }
}

template <typename T>
void Array<T>::Shrink(u32 newSize) {
    assert(newSize <= size);
    count = newSize;
}

template <typename T>
void Array<T>::ShrinkBuffers(u32 newSize) {
    unreachable();
}

template <typename T>
T* Array<T>::PushBack() {
    if (count == capacity) {
        Reserve(_GrowCapacity(count + 1));
    }
    return data + count++;
}

template <typename T>
T* Array<T>::PushBackN(u32 num) {
    if ((count + num) >= capacity) {
        Reserve(_GrowCapacity(count + num));
    }

    T* result = data + count;
    count += num;
    return result;
}


template <typename T>
void Array<T>::PushBack(const T& v) {
    auto entry = PushBack();
    memcpy(entry, &v, sizeof(v));
}

template <typename T>
T* Array<T>::PushFront() {
    T* result = nullptr;
    if (count == 0) {
        result = PushBack();
    } else {
        result = Insert(0);
    }
    return result;
}

template <typename T>
T* Array<T>::PushFrontN(u32 n) {
    T* result = nullptr;
    if (count == 0) {
        result = PushBackN(n);
    } else {
        result = InsertN(0, n);
    }
    return result;
}

template <typename T>
void Array<T>::PushFront(const T& v) {
    auto entry = PushFront();
    memcpy(entry, &v, sizeof(v));
}

template <typename T>
void Array<T>::Pop() {
    assert(count > 0);
    count--;
}

template <typename T>
void Array<T>::Erase(const T* it) {
    assert(it >= data && it < data + count);
    uptr off = it - data;
    memmove(data + off, data + off + 1, ((size_t)count - (size_t)off - 1) * sizeof(T));
    count--;
}

template <typename T>
void Array<T>::EraseUnsorted(const T* it) {
    assert(it >= data && it < data + count);
    uptr off = it - data;
    if (it < data + count - 1) {
        memcpy(data + off, data + count - 1, sizeof(T));
    }
    count--;
}

template <typename T>
T* Array<T>::InsertN(u32 index, u32 n) {
    assert(index < count);
    if ((count + n) >= capacity) {
        Reserve(_GrowCapacity(count + n));
    }

    if (index < count) {
        memmove(data + index + n, data + index, ((size_t)count - (size_t)index) * sizeof(T));
    }
    count += n;
    return data + index;
}

template <typename T>
T* Array<T>::Insert(u32 index) {
    assert(index < count);
    if (count == capacity) {
        Reserve(_GrowCapacity(count + 1));
    }
    if (index < count) {
        memmove(data + index + 1, data + index, ((size_t)count - (size_t)index) * sizeof(T));
    }
    count++;
    return data + index;
}

template <typename T>
void Array<T>::Insert(u32 index, const T& v) {
    auto entry = Insert(index);
    memcpy(entry, &v, sizeof(v));
}

template <typename T>
u32 Array<T>::IndexFromPtr(const T* it) {
    assert(it >= data && it < data + count);
    uptr off = it - data;
    return (u32)off;
}

template <typename T>
u32 Array<T>::_GrowCapacity(u32 sz) {
    u32 newCapacity = capacity ? (capacity + capacity / 2) : 8;
    return newCapacity > sz ? newCapacity : sz;
}

template <typename T>
void Array<T>::Reverse() {
    for (u32 i = 0, j = count - 1; i < j; i++, j--) {
        T tmp = data[i];
        data[i] = data[j];
        data[j] = tmp;
    }
}
