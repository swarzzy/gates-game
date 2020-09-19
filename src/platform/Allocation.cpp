#include "Allocation.h"
#include "../../ext/mimalloc-1.6.4/include/mimalloc.h"

PlatformHeap* CreateHeap() {
    PlatformHeap* heap = (PlatformHeap*)mi_heap_new();
    return heap;
}

#if defined(COMPILER_MSVC)
__declspec(restrict)
#endif
void* HeapAlloc(PlatformHeap* heap, usize size, bool zero) {
    void* mem = nullptr;
    if (zero) {
        mem = mi_heap_zalloc((mi_heap_t*)heap, (size_t)size);
    } else {
        mem = mi_heap_malloc((mi_heap_t*)heap, (size_t)size);
    }
    return mem;
}

#if defined(COMPILER_MSVC)
__declspec(restrict)
#endif
void* HeapRealloc(PlatformHeap* heap, void* p, usize size, bool zero) {
    void* mem = nullptr;
    if (zero) {
        mem = mi_heap_recalloc((mi_heap_t*)heap, p, 1, (size_t)size);
    } else {
        mem = mi_heap_realloc((mi_heap_t*)heap, p, (size_t)size);
    }
    return mem;
}

void Free(void* ptr) {
    mi_free(ptr);
}
