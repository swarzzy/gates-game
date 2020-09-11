#!/bin/bash

##########################################
# Build script for Msys2 x64 using clang #
##########################################

Sources="ext/mimalloc-1.6.4/src/alloc.c ext/mimalloc-1.6.4/src/alloc-aligned.c ext/mimalloc-1.6.4/src/alloc-posix.c ext/mimalloc-1.6.4/src/arena.c ext/mimalloc-1.6.4/src/heap.c ext/mimalloc-1.6.4/src/init.c ext/mimalloc-1.6.4/src/options.c ext/mimalloc-1.6.4/src/os.c ext/mimalloc-1.6.4/src/page.c ext/mimalloc-1.6.4/src/random.c ext/mimalloc-1.6.4/src/region.c ext/mimalloc-1.6.4/src/segment.c ext/mimalloc-1.6.4/src/stats.c"
ObjFiles="alloc.o alloc-aligned.o alloc-posix.o arena.o heap.o init.o options.o os.o page.o random.o region.o segment.o stats.o"

DefinesDebug="-DMI_DEBUG=3 -D_CRT_SECURE_NO_WARNINGS -DMI_SHOW_ERRORS=1"
DefinesRelease="-DNDEBUG -D_CRT_SECURE_NO_WARNINGS"
CommonCompilerFlags="-Iext/mimalloc-1.6.4/include/ -ffast-math -fno-rtti -fno-exceptions -fno-strict-aliasing -Werror -Wno-switch"
DebugCompilerFlags="-O0 -fno-inline-functions -g"
ReleaseCompilerFlags="-O2 -finline-functions -g"

CompilerFlags="$CommonCompilerFlags $DebugCompilerFlags $DefinesDebug $Sources"

clang --compile $CompilerFlags
llvm-ar rcs build/libmimalloc-static.a $ObjFiles
rm $ObjFiles
