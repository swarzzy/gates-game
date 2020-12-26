#include "String.h"

template <typename Char>
StringT<Char> StringT<Char>::MakeFromRawBuffer(void* buffer, u16 length, bool dynamicallyAllocated) {
    StringT result = {};

    result.data = (Char*)buffer;

    *(result._Length()) = length;
    *(result._DynamicallyAllocated()) = dynamicallyAllocated ? 1 : 0;

    return result;
}
