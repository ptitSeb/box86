cmake_minimum_required(VERSION 2.6)

project(box86)

enable_testing()

option(PANDORA "Set to ON if targeting an OpenPandora device" ${PANDORA})
option(USE_CCACHE "Set to ON to use ccache if present in the system" ${USE_CCACHE})


# Pandora
if(PANDORA)
    add_definitions(-DPANDORA)
    add_definitions(-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp -ftree-vectorize -fsingle-precision-constant -ffast-math)
endif()

add_definitions(-g -std=gnu99 -funwind-tables -O3 -fvisibility=hidden)

if(USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif()
endif()

#include_directories(include)

SET(ELFLOADER_SRC
    src/main.c
    src/pathcoll.c
    src/box86context.c
    src/fileutils.c
    src/elfloader.c
    src/elfparser.c
    src/elfload_dump.c
    src/stack.c
    src/x86emu.c
    src/x86run.c
    src/x86run66.c
    src/x86run_private.c
    src/x86syscall.c
    src/x86primop.c
    src/x86trace.c
    src/x86int3.c
)

add_executable(box86 ${ELFLOADER_SRC})
target_link_libraries(box86 m dl rt)

add_test(test01 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box86 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test01 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref01.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

#add_test(test02 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box86 
#    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test02 -D TEST_OUTPUT=tmpfile.txt 
#    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref02.txt
#    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

#add_test(test03 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box86 
#    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test03 -D TEST_OUTPUT=tmpfile.txt 
#    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref03.txt
#    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

#add_test(test01 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box86 
#    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test04 yeah -D TEST_OUTPUT=tmpfile.txt 
#    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref04.txt
#    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )