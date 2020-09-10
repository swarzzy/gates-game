@echo off

set ObjOutDir=..\build\mimalloc_msvc\obj\
set BinOutDir=..\build\mimalloc_msvc\

IF NOT EXIST %BinOutDir% mkdir %BinOutDir%
IF NOT EXIST %ObjOutDir% mkdir %ObjOutDir%

set Sources=..\ext\mimalloc-1.6.4\src\alloc.c ..\ext\mimalloc-1.6.4\src\alloc-aligned.c ..\ext\mimalloc-1.6.4\src\alloc-posix.c ..\ext\mimalloc-1.6.4\src\arena.c ..\ext\mimalloc-1.6.4\src\heap.c ..\ext\mimalloc-1.6.4\src\init.c ..\ext\mimalloc-1.6.4\src\options.c ..\ext\mimalloc-1.6.4\src\os.c ..\ext\mimalloc-1.6.4\src\page.c ..\ext\mimalloc-1.6.4\src\random.c ..\ext\mimalloc-1.6.4\src\region.c ..\ext\mimalloc-1.6.4\src\segment.c ..\ext\mimalloc-1.6.4\src\stats.c
set ObjFiles=..\build\mimalloc_msvc\obj\alloc.obj ..\build\mimalloc_msvc\obj\alloc-aligned.obj ..\build\mimalloc_msvc\obj\alloc-posix.obj ..\build\mimalloc_msvc\obj\arena.obj ..\build\mimalloc_msvc\obj\heap.obj ..\build\mimalloc_msvc\obj\init.obj ..\build\mimalloc_msvc\obj\options.obj ..\build\mimalloc_msvc\obj\os.obj ..\build\mimalloc_msvc\obj\page.obj ..\build\mimalloc_msvc\obj\random.obj ..\build\mimalloc_msvc\obj\region.obj ..\build\mimalloc_msvc\obj\segment.obj  ..\build\mimalloc_msvc\obj\stats.obj

set Defines=/D "MI_DEBUG=3" /D "_CRT_SECURE_NO_WARNINGS" /D "MI_SHOW_ERRORS=1"
set CommonCompilerFlags=/I..\ext\mimalloc-1.6.4\include /Gm- /fp:fast /GR- /nologo /diagnostics:classic /WX /Z7 /W3 /FS
set DebugCompilerFlags=/Od /RTC1 /MTd /Fd%BinOutDir%\
set ReleaseCompilerFlags=/O2 /MT

set LinkerFlags=/MACHINE:X64 /OUT:%BinOutDir%\mimalloc-static.lib

set ConfigCompilerFlags=%DebugCompilerFlags%

echo Building mimalloc...
cl /c /MP /Fo%ObjOutDir% %Defines% %CommonCompilerFlags% %ConfigCompilerFlags% %Sources%
lib %LinkerFlags% %ObjFiles%

copy /Y ..\build\mimalloc_msvc\mimalloc-static.lib ..\build\mimalloc-static.lib
rmdir /S /Q %BinOutDir%
