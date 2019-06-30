cmake_minimum_required(VERSION 2.6)

project(box86)

enable_testing()

option(PANDORA "Set to ON if targeting an OpenPandora device" ${PANDORA})
option(USE_CCACHE "Set to ON to use ccache if present in the system" ${USE_CCACHE})
option(LD80BITS "Set to ON if host device have 80bits long double (i.e. i386)" ${LD80BITS})
option(NOALIGN "Set to ON if host device doesn't need re-align (i.e. i386)" ${NOALIGN})
option(HAVE_TRACE "Set to ON to have Trace ability (needs ZydisInfo library)" ${HAVE_TRACE})
option(USE_FLOAT "Set to ON to use only float, no double, in all x87 Emulation" ${USE_FLOAT})
option(NOLOADADDR "Set to ON to avoid fixing the load address of Box86" ${NO_LOADAADR})
option(X86_CPP_LIBRARY_PRESENT "Set to ON if you have an accessible libstdc++.so.6 32bit ELF lib in your PATH" ${X86_CPP_LIBRARY_PRESENT})

# Pandora
if(PANDORA)
    add_definitions(-DPANDORA)
    #add_definitions(-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp -ftree-vectorize -fsingle-precision-constant -ffast-math)
endif()

if(LD80BITS)
    add_definitions(-DHAVE_LD80BITS)
endif()

if(NOALIGN)
    add_definitions(-DNOALIGN)
endif()

if(HAVE_TRACE)
    add_definitions(-DHAVE_TRACE)
endif()

if(USE_FLOAT)
    add_definitions(-DUSE_FLOAT)
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
    "${CMAKE_HOME_DIRECTORY}/src/x86run66.c"
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
    "${CMAKE_HOME_DIRECTORY}/src/signals.c"
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
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibvorbis.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibogg.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedpng12.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedpng16.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibxxf86vm.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibxrandr.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibxext.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedxinerama.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibxfixes.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibxcursor.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibxrender.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibxi.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibxss.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibx11xcb.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibxcb.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibz.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedopenal.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedalut.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibjpeg.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedcurl.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedudev0.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappeddbus.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappediconv.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedcrypto.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedlibssl.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedutil.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrappedldlinux.c"
)
set(WRAPPER "${CMAKE_HOME_DIRECTORY}/src/wrapper.c" "${CMAKE_HOME_DIRECTORY}/src/wrapper.h")
add_custom_command(OUTPUT ${WRAPPER} COMMAND "${CMAKE_HOME_DIRECTORY}/rebuild_wrappers.py" "${CMAKE_HOME_DIRECTORY}" "PANDORA" "HAVE_LD80BITS" "NOALIGN" "HAVE_TRACE" "USE_FLOAT" MAIN_DEPENDENCY "${CMAKE_HOME_DIRECTORY}/rebuild_wrappers.py" DEPENDS ${WRAPPEDS} BYPRODUCTS ${WRAPPER})

if(USE_FLOAT)
    set(BOX86 box86f)
else()
    set(BOX86 box86)
endif()

add_executable(${BOX86} ${ELFLOADER_SRC} ${WRAPPEDS})
target_link_libraries(${BOX86} m dl rt pthread)

if(NOT NOLOADADDR)
    target_link_libraries(${BOX86} -Wl,-Ttext-segment,0xA8000000)
endif()

add_test(test01 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test01 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref01.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test02 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test02 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref02.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test03 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test03 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref03.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test04 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test04 -D TEST_ARGS2=yeah -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref04.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test05 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test05 -D TEST_ARGS2=7 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref05.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test06 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test06 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref06.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test07 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test07 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref07.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test08 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test08 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref08.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test09 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test09 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref09.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

if(X86_CPP_LIBRARY_PRESENT)
    add_test(test10 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test10 -D TEST_OUTPUT=tmpfile.txt 
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref10.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )
endif(X86_CPP_LIBRARY_PRESENT)

add_test(test11 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test11 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref11.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )
