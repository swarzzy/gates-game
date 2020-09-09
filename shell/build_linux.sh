#!/bin/bash

##########################################
# Build script linux using clang         #
##########################################

ObjOutDir=build/obj/
BinOutDir=build/

mkdir -p $BinOutDir
mkdir -p $ObjOutDir

CommonDefines="-Iext/SDL2-2.0.12/include -D_CRT_SECURE_NO_WARNINGS -DPLATFORM_LINUX"
CommonCompilerFlags="-fPIC -std=c++17 -ffast-math -fno-rtti -fno-exceptions -static-libgcc -static-libstdc++ -fno-strict-aliasing -Werror -Wno-switch"
DebugCompilerFlags="-O0 -fno-inline-functions -g"
ReleaseCompilerFlags="-O2 -finline-functions -g"
PlatformLinkerFlags="-Lbuild -lSDL2 -ldl -pthread"
GameLinkerFlags=""

ConfigCompilerFlags=$DebugCompilerFlags

clang++ -save-temps=obj -DPLATFORM_CODE -o $BinOutDir/linux_pong $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags src/platform/SDLLinuxPlatform.cpp $PlatformLinkerFlags
clang++ -save-temps=obj -o $BinOutDir/pong.so $CommonDefines $IncludeDirs $CommonCompilerFlags $ConfigCompilerFlags src/GameEntry.cpp -shared  $GameLinkerFlags
