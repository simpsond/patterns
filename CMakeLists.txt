cmake_minimum_required(VERSION 3.25)
project(patterns C)

set(CMAKE_C_STANDARD 99)
include_directories(
        /opt/vbcc/targets/m68k-amigaos/include
        /Volumes/workspace/amiga_dev/NDK/NDK3.2R4/Include_H
        MUI/Developer/C/Include
)


add_executable(patterns
        main.c
        utils.c
        gui_helpers.c
        )

add_executable(realloc_test
        utils.c
        realloc_test.c)
install(TARGETS patterns DESTINATION patterns)
install(TARGETS realloc_test DESTINATION patterns)
