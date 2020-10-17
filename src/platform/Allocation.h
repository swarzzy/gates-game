#pragma once

struct PlatformHeap;

void MiMallocInit();

PlatformHeap* CreateHeap();

void* HeapAlloc(PlatformHeap* heap, usize size, bool zero);
void* HeapRealloc(PlatformHeap* heap, void* p, usize size, bool zero);
void Free(void* ptr);
