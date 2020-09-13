@echo off

rmdir /S /Q build\SDL_MSVC\
cmake -Sext/SDL2-2.0.12 -Bbuild/SDL_MSVC -G"Visual Studio 15 2017 Win64" -DSDL_STATIC=ON -DSDL_SHARED=OFF -DFORCE_STATIC_VCRT=ON
cmake --build build/SDL_MSVC
mkdir build\lib\
copy /Y build\SDL_MSVC\Debug\SDL2d.lib build\lib\SDL2d_x64.lib

rem rmdir /S /Q build\SDL_MSVC\
rem cmake -Sext/SDL2-2.0.12 -Bbuild/SDL_MSVC -G"Visual Studio 15 2017" -DSDL_STATIC=ON -DSDL_SHARED=OFF -DFORCE_STATIC_VCRT=ON -DHAVE_LIBC=TRUE
rem cmake --build build/SDL_MSVC
rem copy /Y build\SDL_MSVC\Debug\SDL2d.lib build\lib\SDL2d_x86.lib
