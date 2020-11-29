@echo off

if not defined DevEnvDir (
   call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

if not exist "%~dp0build" mkdir "%~dp0build"
pushd "%~dp0build"

REM Optimization switches: /O2 /Oi /fp:fast
REM  -W4 -wd4201 -wd4100 -wd4505 -wd4189
set CommonCompilerFlags=/nologo /EHa -FC -WX -W4 -wd4201 -wd4100 -wd4505 -wd4189 -MTd -Oi -Od -GR -Zi /DPLATEFORM_WIN32
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib shell32.lib Comdlg32.lib opengl32.lib glfw3.lib imgui.lib assimp.lib /NODEFAULTLIB:LIBCMT
REM set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib shell32.lib Comdlg32.lib opengl32.lib glfw3.lib assimp.lib /NODEFAULTLIB:LIBCMT
set AdditionalIncludesDir=/I C:\Users\test\Documents\dev\crapengine\dep\includes
set AdditionalLibDir=/LIBPATH:C:\Users\test\Documents\dev\crapengine\dep\lib

if not exist "%~dp0build\assimp-vc142-mtd.dll" copy ..\dep\lib\assimp-vc142-mtd.dll ..\build\
REM robocopy ..\Shaders Shaders /MIR
REM robocopy ..\data data /MIR

cl /Feapp.exe %CommonCompilerFlags% %AdditionalIncludesDir% "..\src\main.cpp" /link %AdditionalLibDir% %CommonLinkerFlags%

popd

REM Compiler Flags:

REM Zi  : debug info (Z7 older debug format for complex builds)
REM Zo  : More debug info for optimized builds
REM FC  : Full path on errors
REM Oi  : Always do intrinsics with you can
REM Od  : No optimizations
REM O2  : Full optimizations
REM MT  : Use the c static lib instead of searching for dll at run-time
REM MTd : Sabe as MT but using the debug version of CRT
REM GR- : Turn off C++ run-time type info
REM Gm- : Turn off incremental build
REM EHa-: Turn off exception handling
REM WX  : Treat warning as errors
REM W4  : Set Warning level to 4 (Wall to all levels)
REM wd  : Ignore warning
REM fp:fast    : Ignores the rules in some cases to optimize fp operations
REM Fmfile.map : Outputs a map file (mapping of the functions on the exr)

REM Linker Options:

REM subsystem:windows,5.1 : Make exe compatible with Windows XP (only works on x86)
REM opt:ref               : Don't put unused things in the exe
REM incremental:no        : Don't need to do incremental builds
REM LD                    : Build a dll
REM PDB:file.pdb          : Change the .pdb's path
