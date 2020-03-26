@echo off

if not defined DevEnvDir (
   call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

if not exist "%~dp0build" mkdir "%~dp0build"
pushd "%~dp0build"

REM Optimization switches: /O2 /Oi /fp:fast
REM  -W4 -wd4201 -wd4100 -wd4505 -wd4189
set CommonCompilerFlags=/nologo /EHa -FC -WX -W4 -wd4201 -wd4100 -wd4505 -wd4189 -MTd -Oi -Od -GR -Zi
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib shell32.lib opengl32.lib glfw3.lib

set AdditionalIncludesDir=/I C:\Users\test\Documents\dev\crapengine\dep\includes
set AdditionalLibDir=/LIBPATH:C:\Users\test\Documents\dev\crapengine\dep\lib

REM robocopy ..\Shaders Shaders /MIR
REM robocopy ..\data data /MIR

cl /Feapp.exe %CommonCompilerFlags% %AdditionalIncludesDir% "..\dep\includes\IMGUI\*.cpp" "..\src\main.cpp" /link %AdditionalLibDir% %CommonLinkerFlags%
popd
