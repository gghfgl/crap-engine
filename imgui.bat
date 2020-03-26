@echo off

if not defined DevEnvDir (
   call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)

if not exist "%~dp0slib" mkdir "%~dp0slib"
pushd "%~dp0slib"

REM Optimization switches: /O2 /Oi /fp:fast
REM  -W4 -wd4201 -wd4100 -wd4505 -wd4189
set CommonCompilerFlags=/nologo /EHa -FC -WX -W4 -wd4201 -wd4100 -wd4505 -wd4189 -MTd -Oi -Od -GR -Zi
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib gdi32.lib shell32.lib opengl32.lib glfw3.lib
set AdditionalIncludesDir=/I C:\Users\test\Documents\dev\crapengine\dep\includes
set AdditionalLibDir=/LIBPATH:C:\Users\test\Documents\dev\crapengine\dep\lib

REM Create imgui *.obj before create lib
cl /c /EHsc %AdditionalIncludesDir% "C:\Users\test\Documents\dev\imgui\*.cpp" /link %AdditionalLibDir% %CommonLinkerFlags%

REM buld lib
lib "..\slib\*.obj"
popd
