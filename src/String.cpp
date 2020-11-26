#include "String.h"

template <typename Char, u32 Size>
StaticString<Char, Size>::StaticString() {
    data[0] = (Char)0;
}

template <typename Char>
DynamicString<Char>::DynamicString(Allocator* alloc) {
    static_assert(((sizeof(u16) * 2) % 2) == 0);

    allocator = alloc;
    data = allocator.Alloc<Char>((sizeof(u16) * 2) / 2 + 16, false);
    _At() = 0;
    _Capacity() = 16;
    Buffer()[0] = (Char)0;
}
