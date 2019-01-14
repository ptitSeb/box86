cmake_minimum_required(VERSION 2.6)

project(box86)

enable_testing()

option(PANDORA "Set to ON if targeting an OpenPandora device" ${PANDORA})
option(USE_CCACHE "Set to ON to use ccache if present in the system" ${USE_CCACHE})
option(LD80BITS "Set to ON if host device have 80bits long double (i.e. i386)" ${LD80BITS})
option(NOALIGN "Set to ON if host device doesn't need re-align (i.e. i386)" ${NOALIGN})


# Pandora
if(PANDORA)
    add_definitions(-DPANDORA)
    add_definitions(-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp -ftree-vectorize -fsingle-precision-constant -ffast-math)
endif()

if(LD80BITS)
    add_definitions(-DHAVE_LD80BITS)
endif()

if(NOALIGN)
    add_definitions(-DNOALIGN)
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

set(ELFLOADER_SRC
    "${CMAKE_HOME_DIRECTORY}/src/bridge.c"
    "${CMAKE_HOME_DIRECTORY}/src/main.c"
    "${CMAKE_HOME_DIRECTORY}/src/pathcoll.c"
    "${CMAKE_HOME_DIRECTORY}/src/box86context.c"
    "${CMAKE_HOME_DIRECTORY}/src/fileutils.c"
    "${CMAKE_HOME_DIRECTORY}/src/elfloader.c"
    "${CMAKE_HOME_DIRECTORY}/src/elfparser.c"
    "${CMAKE_HOME_DIRECTORY}/src/elfload_dump.c"
    "${CMAKE_HOME_DIRECTORY}/src/library.c"
    "${CMAKE_HOME_DIRECTORY}/src/librarian.c"
    "${CMAKE_HOME_DIRECTORY}/src/stack.c"
    "${CMAKE_HOME_DIRECTORY}/src/threads.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapper.c"
    "${CMAKE_HOME_DIRECTORY}/src/x86emu.c"
    "${CMAKE_HOME_DIRECTORY}/src/x86run.c"
    "${CMAKE_HOME_DIRECTORY}/src/x86run0f.c"
    "${CMAKE_HOME_DIRECTORY}/src/x86run66.c"
    "${CMAKE_HOME_DIRECTORY}/src/x86run660f.c"
    "${CMAKE_HOME_DIRECTORY}/src/x86run_private.c"
    "${CMAKE_HOME_DIRECTORY}/src/x86syscall.c"
    "${CMAKE_HOME_DIRECTORY}/src/x86primop.c"
    "${CMAKE_HOME_DIRECTORY}/src/x86trace.c"
    "${CMAKE_HOME_DIRECTORY}/src/x86int3.c"
    "${CMAKE_HOME_DIRECTORY}/src/x87emu_private.c"
    "${CMAKE_HOME_DIRECTORY}/src/x87run.c"
    "${CMAKE_HOME_DIRECTORY}/src/myalign.c"
    "${CMAKE_HOME_DIRECTORY}/src/callback.c"
    "${CMAKE_HOME_DIRECTORY}/src/sdl1rwops.c"
    "${CMAKE_HOME_DIRECTORY}/src/sdl2rwops.c"
)

set(WRAPPEDS
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibc.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibpthread.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibrt.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibgl.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibglu.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibx11.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibasound.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibdl.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibm.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedsdl1.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedsdl1mixer.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedsdl1image.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedsdl2.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedsdl2mixer.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedsdl2image.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedsmpeg2.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedvorbisfile.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedpng16.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedldlinux.c"
)
set(WRAPPER "${CMAKE_HOME_DIRECTORY}/src/wrapper.c" "${CMAKE_HOME_DIRECTORY}/src/wrapper.h")
add_custom_command(OUTPUT ${WRAPPER} COMMAND "${CMAKE_HOME_DIRECTORY}/rebuild_wrappers.py" "${CMAKE_HOME_DIRECTORY}" MAIN_DEPENDENCY "${CMAKE_HOME_DIRECTORY}/rebuild_wrappers.py" DEPENDS ${WRAPPEDS})
set_source_files_properties(${WRAPPER} PROPERTIES GENERATED TRUE)

add_executable(box86 ${ELFLOADER_SRC} ${WRAPPEDS})
target_link_libraries(box86 m dl rt pthread)

target_link_libraries(box86 -Wl,-Ttext-segment,0xA8000000)

add_test(test01 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box86 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test01 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref01.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test02 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box86 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test02 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref02.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test03 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box86 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test03 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref03.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test04 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box86 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test04 -D TEST_ARGS2=yeah -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref04.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test05 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box86 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test05 -D TEST_ARGS2=7 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref05.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test06 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box86 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test06 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref06.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test07 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box86 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test07 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref07.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test08 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box86 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test08 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref08.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )
