#include "Allocation.h"
#include "../../ext/mimalloc-1.6.4/include/mimalloc.h"

void MiMallocErrorCallback(int err, void* arg) {
    switch (err) {
    case EAGAIN: { log_print("[mimalloc] Error EAGAIN. Double free was detected\n");} break;
    case EFAULT: { log_print("[mimalloc] Error EFAULT. Corrupted free list or meta-data was detected\n");} break;
    case ENOMEM: { log_print("[mimalloc] Error ENOMEM. Not enough memory available to satisfy the request\n");} break;
    case EOVERFLOW: { log_print("[mimalloc] Error EOVERFLOW. Too large a request\n");} break;
    case EINVAL: { log_print("[mimalloc] Error EINVAL. Trying to free or re-allocate an invalid pointer\n");} break;
    invalid_default();
    }
    assert(false);
}

void MiMallocInit() {
    mi_register_error(MiMallocErrorCallback, nullptr);
}

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
