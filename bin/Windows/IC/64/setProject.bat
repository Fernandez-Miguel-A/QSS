@echo off
rem QSS Windows Intel C++ 64-Bit Setup

if not (%QSS%) == () goto Step2
set QSS=%~dp0
set QSS=%QSS:~0,-19%
:Step2

set PlatformBits=Windows\IC\64

set Path=%Path%;%QSS%\bin\%PlatformBits%

call setPython.64.bat
call setIC.64.bat
call setGTest.bat
set Path=%Path%;%GTEST_BIN%

%~dp0..\setProject.bat
