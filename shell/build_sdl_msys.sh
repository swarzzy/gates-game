#!/bin/bash

##########################################
# Build script for Msys2 x64 using clang #
##########################################

CXX=clang++ CC=clang cmake ext/SDL2-2.0.12/ -B./build/SDL_CLANG/ -G"MSYS Makefiles" -DSDL_STATIC=ON -DSDL_SHARED=OFF -DFORCE_STATIC_VCRT=ON
cmake --build build/SDL_CLANG
cp build/SDL_CLANG/libSDL2.a build/libSDL2.a
