#!/bin/bash

##########################################
# Build script for Linux using clang     #
##########################################

CXX=clang++ CC=clang cmake ext/SDL2-2.0.12/ -B./build/SDL/ -G"Unix Makefiles" -DSDL_STATIC=ON -DSDL_SHARED=OFF -DFORCE_STATIC_VCRT=ON -DVIDEO_X11=ON
cmake --build build/SDL
cp build/SDL/libSDL2.a build/libSDL2.a
