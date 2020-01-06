cmake_minimum_required(VERSION 3.12)
project(tp1)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(tp1)

add_executable(tp1
        tp1/.idea/misc.xml
        tp1/.idea/modules.xml
        tp1/.idea/tp1.iml
        tp1/.idea/workspace.xml
        tp1/comp.dSYM/Contents/Resources/DWARF/comp
        tp1/comp.dSYM/Contents/Info.plist
        tp1/comp
        tp1/debug.h
        tp1/petit-comp.c
        tp1/test.c
        debug.h
        petit-comp.c
        test.c)
