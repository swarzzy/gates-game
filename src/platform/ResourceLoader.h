#pragma once

#include "../Common.h"
#include "../Platform.h"
#include "Allocation.h"

extern PlatformHeap* ResourceLoaderScratchHeap;

void __cdecl ResourceLoaderInvoke(ResourceLoaderCommand command, void* _args, void* result);
LoadImageResult* ResourceLoaderLoadImage(const char* filename, b32 flipY, u32 forceBPP, Allocator* allocator);
void ResourceLoaderBakeFont(Font* result, const char* filename, Allocator* allocator, f32 height, CodepointRange* ranges, u32 rangeCount);
