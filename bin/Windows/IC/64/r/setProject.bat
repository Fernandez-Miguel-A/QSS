@echo off
rem QSS Windows Intel C++ 64-Bit Release Setup

set QSS_bin=%~dp0
set QSS_bin=%QSS_bin:~0,-1%
if (%QSS%) == () set QSS=%QSS_bin:~0,-20%

set PlatformFull=Windows\IC\64\r

set Path=%Path%;%QSS_bin%

set LIB=%LIB%;%QSS_bin%

%~dp0..\setProject.bat
