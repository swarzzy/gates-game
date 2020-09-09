@echo off
msbuild ..\ext\mimalloc-1.6.4\ide\vs2017\mimalloc.vcxproj -nologo -m -p:Configuration=Debug -p:DebugSymbols=true -p:Platform=x64 -p:OutDir=..\..\..\..\build\mimalloc_msvc\ -p:IntermediateOutputPath=..\..\..\..\build\mimalloc_msvc\obj\ -p:OutputPath=OutDir=..\..\..\..\build\mimalloc_msvc\ -p:WindowsTargetPlatformVersion=10.0.17763.0
rmdir /S /Q ..\ext\mimalloc-1.6.4\out
copy /Y ..\build\mimalloc_msvc\mimalloc-static.lib ..\build\mimalloc-static.lib
