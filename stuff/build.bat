@echo off
cl /MDd /Zi /GX /nologo %1.cpp /Fo..\Debug\%1.obj /Fd..\Debug\%1.pdb /Fe..\Debug\%1.exe ..\Debug\stuff_debug.lib
if ERRORLEVEL 1 goto done
rem ..\Debug\%1.exe
rem start %1.ps
:done