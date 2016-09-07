# QSS Make Initialization File
#
# Language: GNU Make
#
# Platform: Windows/VC/64/d

# Variables
CXXFLAGS = /nologo /EHsc /W3 /wd4068 /wd4101 /wd4102 /wd4244 /wd4258 /wd4355 /wd4996 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /TP /Od /Ob0 /Z7 /RTCsu /MTd
CFLAGS = /nologo /W3 /wd4244 /wd4996 /DWIN32_LEAN_AND_MEAN /TC /Od /Ob0 /Z7 /RTCsu /MTd
# Disabled Warnings:
#  4068 Unknown pragma
#  4101 Unreferenced local variable
#  4102 Unreferenced label
#  4244 Narrowing conversions
#  4258 Definition from the loop is ignored
#  4355 Passing this pointer in class initializer (object is incomplete so bases/members can only use this in limited ways)
#  4996 Deprecated functions (/D_SCL_SECURE_NO_WARNINGS /D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS)
# /D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 /D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 enables secure template overloads
# /RTCc gave exe blocked by Windows 8.1 Defender with ostringstream
LDFLAGS := /nologo /F8388608
LINKFLAGS := /link /LIBPATH:$(QSS_bin) /DEBUG

include $(QSS_bin)\..\GNUmakeinit.mk
