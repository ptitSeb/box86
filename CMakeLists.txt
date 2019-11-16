cmake_minimum_required(VERSION 2.6)

project(box86)

enable_testing()
enable_language(C)

option(PANDORA "Set to ON if targeting an OpenPandora device" ${PANDORA})
option(GAMESHELL "Set to ON if targeting a Gameshell device" ${GAMESHELL})
option(USE_CCACHE "Set to ON to use ccache if present in the system" ${USE_CCACHE})
option(LD80BITS "Set to ON if host device have 80bits long double (i.e. i386)" ${LD80BITS})
option(NOALIGN "Set to ON if host device doesn't need re-align (i.e. i386)" ${NOALIGN})
option(HAVE_TRACE "Set to ON to have Trace ability (needs ZydisInfo library)" ${HAVE_TRACE})
option(USE_FLOAT "Set to ON to use only float, no double, in all x87 Emulation" ${USE_FLOAT})
option(NOLOADADDR "Set to ON to avoid fixing the load address of Box86" ${NO_LOADAADR})
option(ARM_DYNAREC "Set to ON to use ARM Dynamic Recompilation (WIP, don't use yet)" ${ARM_DYNAREC})
option(X86_CPP_LIBRARY_PRESENT "Set to ON if you have an accessible libstdc++.so.6 32bit ELF lib in your PATH" ${X86_CPP_LIBRARY_PRESENT})

find_package(PythonInterp)
if(NOT PYTHONINTERP_FOUND)
    message( FATAL_ERROR "You need a Python interpretor, CMake will exit." )
endif()

# Pandora
if(PANDORA)
    add_definitions(-DPANDORA)
    #add_definitions(-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp -ftree-vectorize -fsingle-precision-constant -ffast-math)
endif()

# Gameshell
if(GAMESHELL)
    add_definitions(-march=armv7-a+fp -mfpu=neon -mfloat-abi=hard )
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

if(ARM_DYNAREC)
    add_definitions(-DDYNAREC)
    add_definitions(-DARM)
    add_definitions(-marm)
    enable_language(ASM)
endif()



add_definitions(-g -std=gnu99 -funwind-tables -O3 -fvisibility=hidden)

if(USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif()
endif()

include_directories(
    "${CMAKE_HOME_DIRECTORY}/src/include"
    "${CMAKE_HOME_DIRECTORY}/src"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/generated"
)

set(ELFLOADER_SRC
    "${CMAKE_HOME_DIRECTORY}/src/main.c"
    "${CMAKE_HOME_DIRECTORY}/src/box86context.c"
    "${CMAKE_HOME_DIRECTORY}/src/tools/bridge.c"
    "${CMAKE_HOME_DIRECTORY}/src/tools/pathcoll.c"
    "${CMAKE_HOME_DIRECTORY}/src/tools/fileutils.c"
    "${CMAKE_HOME_DIRECTORY}/src/tools/callback.c"
    "${CMAKE_HOME_DIRECTORY}/src/tools/box86stack.c"
    "${CMAKE_HOME_DIRECTORY}/src/elfs/elfloader.c"
    "${CMAKE_HOME_DIRECTORY}/src/elfs/elfparser.c"
    "${CMAKE_HOME_DIRECTORY}/src/elfs/elfload_dump.c"
    "${CMAKE_HOME_DIRECTORY}/src/librarian/library.c"
    "${CMAKE_HOME_DIRECTORY}/src/librarian/librarian.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/generated/wrapper.c"
    "${CMAKE_HOME_DIRECTORY}/src/emu/x86emu.c"
    "${CMAKE_HOME_DIRECTORY}/src/emu/x86run.c"
    "${CMAKE_HOME_DIRECTORY}/src/emu/x86run66.c"
    "${CMAKE_HOME_DIRECTORY}/src/emu/x86run_private.c"
    "${CMAKE_HOME_DIRECTORY}/src/emu/x86syscall.c"
    "${CMAKE_HOME_DIRECTORY}/src/emu/x86primop.c"
    "${CMAKE_HOME_DIRECTORY}/src/emu/x86trace.c"
    "${CMAKE_HOME_DIRECTORY}/src/emu/x86int3.c"
    "${CMAKE_HOME_DIRECTORY}/src/emu/x87emu_private.c"
    "${CMAKE_HOME_DIRECTORY}/src/emu/x87run.c"
    "${CMAKE_HOME_DIRECTORY}/src/libtools/myalign.c"
    "${CMAKE_HOME_DIRECTORY}/src/libtools/threads.c"
    "${CMAKE_HOME_DIRECTORY}/src/libtools/sdl1rwops.c"
    "${CMAKE_HOME_DIRECTORY}/src/libtools/sdl2rwops.c"
    "${CMAKE_HOME_DIRECTORY}/src/libtools/signals.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec.c"
)

set(WRAPPEDS
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibc.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibpthread.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibrt.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibgl.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibglu.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibx11.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibasound.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibdl.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibm.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedsdl1.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedsdl1mixer.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedsdl1image.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedsdl1ttf.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedsdl2.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedsdl2mixer.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedsdl2image.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedsmpeg.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedsmpeg2.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedvorbisfile.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibvorbis.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibogg.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedpng12.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedpng16.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibxxf86vm.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibxrandr.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibxext.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedxinerama.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibxfixes.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibxcursor.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibxrender.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibxi.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibxss.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibxau.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibxdmcp.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibx11xcb.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibxcb.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibz.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedopenal.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedalut.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibjpeg.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedcurl.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedudev0.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappeddbus.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappediconv.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedcrypto.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibssl.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedutil.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibuuid.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedlibresolv.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedpulsesimple.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedmonounity.c"
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/wrappedldlinux.c"
)

set(WRAPPEDS_HEAD "")
foreach(A ${WRAPPEDS})
    string(REPLACE ".c" "_private.h" B ${A})
    set(WRAPPEDS_HEAD ${WRAPPEDS_HEAD} ${B})
endforeach()

set(WRAPPER "${CMAKE_HOME_DIRECTORY}/src/wrapped/generated/wrapper.c" "${CMAKE_HOME_DIRECTORY}/src/wrapped/generated/wrapper.h")
add_custom_command(OUTPUT ${WRAPPER} COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_HOME_DIRECTORY}/rebuild_wrappers.py" "${CMAKE_HOME_DIRECTORY}" "PANDORA" "HAVE_LD80BITS" "NOALIGN" "HAVE_TRACE" "USE_FLOAT" MAIN_DEPENDENCY "${CMAKE_HOME_DIRECTORY}/rebuild_wrappers.py" DEPENDS ${WRAPPEDS} ${WRAPPEDS_HEAD})

if(ARM_DYNAREC)
    set(DYNAREC_SRC
        "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynablock.c"

        "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm.c"
        "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_functions.c"
        "${CMAKE_HOME_DIRECTORY}/src/dynarec/arm_printer.c"

        "${CMAKE_HOME_DIRECTORY}/src/dynarec/arm_prolog.S"
        "${CMAKE_HOME_DIRECTORY}/src/dynarec/arm_epilog.S"
        "${CMAKE_HOME_DIRECTORY}/src/dynarec/arm_linker.S"
        "${CMAKE_HOME_DIRECTORY}/src/dynarec/arm_table.S"
    )

    set(DYNAREC_PASS
    "${CMAKE_HOME_DIRECTORY}/src/wrapped/generated/wrapper.h"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_helper.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_pass.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_00.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_0f.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_65.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_66.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_67.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_d8.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_d9.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_da.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_db.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_dc.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_dd.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_de.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_df.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_f0.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_660f.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_f20f.c"
    "${CMAKE_HOME_DIRECTORY}/src/dynarec/dynarec_arm_f30f.c"
    )

    add_library(dynarec_arm OBJECT ${DYNAREC_SRC})

    add_library(arm_pass0 OBJECT ${DYNAREC_PASS})
    set_target_properties(arm_pass0 PROPERTIES COMPILE_FLAGS "-DSTEP=0")
    add_library(arm_pass1 OBJECT ${DYNAREC_PASS})
    set_target_properties(arm_pass1 PROPERTIES COMPILE_FLAGS "-DSTEP=1")
    add_library(arm_pass2 OBJECT ${DYNAREC_PASS})
    set_target_properties(arm_pass2 PROPERTIES COMPILE_FLAGS "-DSTEP=2")
    add_library(arm_pass3 OBJECT ${DYNAREC_PASS})
    set_target_properties(arm_pass3 PROPERTIES COMPILE_FLAGS "-DSTEP=3")

    add_library(dynarec STATIC 
        $<TARGET_OBJECTS:dynarec_arm> 
        $<TARGET_OBJECTS:arm_pass0> 
        $<TARGET_OBJECTS:arm_pass1>
        $<TARGET_OBJECTS:arm_pass2>
        $<TARGET_OBJECTS:arm_pass3>
    )
endif()

if(USE_FLOAT)
    set(BOX86 box86f)
elseif(HAVE_TRACE)
    set(BOX86 box86t)
else()
    set(BOX86 box86)
endif()

add_executable(${BOX86} ${ELFLOADER_SRC} ${WRAPPEDS})
target_link_libraries(${BOX86} m dl rt pthread)
if(ARM_DYNAREC)
    target_link_libraries(${BOX86} dynarec)
endif()

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

add_test(test12 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test12 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref12.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(test13 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test13 -D TEST_OUTPUT=tmpfile.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref13.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

    file(GLOB extension_tests "${CMAKE_SOURCE_DIR}/tests/extensions/*.c")
foreach(file ${extension_tests})
    get_filename_component(testname "${file}" NAME_WE)
    add_test(NAME "${testname}" COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/extensions/${testname} -D TEST_OUTPUT=tmpfile.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/extensions/${testname}.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake)
endforeach()
