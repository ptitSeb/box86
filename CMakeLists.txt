cmake_minimum_required(VERSION 3.10)

option(PANDORA "Set to ON if targeting an OpenPandora device" ${PANDORA})
option(PYRA "Set to ON if targeting an OpenPandora device" ${PYRA})
option(RPI2 "Set to ON if targeting an RaspberryPI2 device" ${RPI2})
option(RPI3 "Set to ON if targeting an RaspberryPI3 device" ${RPI3})
option(RPI4 "Set to ON if targeting an RaspberryPI4 device" ${RPI4})
option(RPI3ARM64 "Set to ON if targeting an RaspberryPI3 device with multiarch arm64 and armhf" ${RPI3ARM64})
option(RPI4ARM64 "Set to ON if targeting an RaspberryPI4 device with multiarch arm64 and armhf" ${RPI4ARM64})
option(GOA_CLONE "Set to ON if targeting GO Advance clones, like RG351p/v, Gameforce Chi, RGB10..." ${GOA_CLONE})
option(RK3288 "Set to ON if targeting an Rockchip RK3288 based device" ${RK3288})
option(RK3399 "Set to ON if targeting an Rockchip RK3399 based device" ${RK3399})
option(RK3588 "Set to ON if targeting an Rockchip RK3588 / RK3588S based device" ${RK3588})
option(ODROIDN2 "Set to ON if targeting an ODROID-N2 device" ${ODROIDN2})
option(TEGRAX1 "Set to ON if targeting an Tegra X1 (ARM64) based device" ${TEGRAX1})
option(TEGRAX1ARM32 "Set to ON if targeting an Tegra X1 (ARM32) based device" ${TEGRAX1ARM32})
option(TEGRA_T194 "Set to ON if targeting an Tegra Xavier based device" ${TEGRA_T194})
option(TEGRA_T234 "Set to ON if targeting an Tegra Orin based device" ${TEGRA_T234})
option(PHYTIUM "Set to ON if targeting an Phytium (D2000 or FT2000/4) based device" ${PHYTIUM})
option(GAMESHELL "Set to ON if targeting a GameShell device" ${GAMESHELL})
option(ODROIDXU4 "Set to ON if targeting an ODROID-XU4 device" ${ODROIDXU4})
option(POWER9 "Set to ON if targeting a POWER9 processor" ${POWER9})
option(SD845 "Set to ON if targeting a Snapragon 845 based device" ${SD845})
option(SD865 "Set to ON if targeting a Snapragon 865 based device" ${SD865})
option(SD888 "Set to ON if targeting a Snapragon 888 based device" ${SD888})
option(ADLINK "Set to ON if targeting an ADLink AmpereAltra based device" ${ADLINK})
option(A64 "Set to ON if targeting an Allwinner A64 based device" ${A64})
option(ARM64 "Set to ON if targeting a generic ARM64 based device" ${ARM64})
option(BAD_SIGNAL "Set to ON to activate the workaround for incoherent si_info on SIGSEGV" ${BAD_SIGNAL})
option(LX2160A "Set to ON if targeting an LX2160A based device" ${LX2160A})
option(ANDROID "Set to ON if targeting an Android device" ${ANDROID})
option(TERMUX "Set to ON if targeting an Android device with Termux" ${TERMUX})
option(USE_CCACHE "Set to ON to use ccache if present in the system" ${USE_CCACHE})
option(HAVE_TRACE "Set to ON to have Trace ability (needs ZydisInfo library)" ${HAVE_TRACE})
option(NOLOADADDR "Set to ON to avoid fixing the load address of Box86" ${NO_LOADAADR})
option(NOGETCLOCK "Set to ON to avoid using clock_gettime with CLOCK_MONOTONIC_COARSE for RDTSC opcode (use gltimeofday instead)" ${NOGETCLOCK})
option(NOGIT "Set to ON if not building from a git clone repo (like when building from a zip download from github)" ${NOGIT})
option(NO_LIB_INSTALL "Set ON to not install a few x86 libs that are used by many program" ${NO_LIB_INSTALL})
option(NO_CONF_INSTALL "Set ON to not install config files" ${NO_CONF_INSTALL})

if(PANDORA OR PYRA OR RPI2 OR RPI3 OR RPI4 OR GAMESHELL OR ODROIDXU4 OR GOA_CLONE OR RK3288 OR RK3399 OR ODROIDN2 OR TEGRAX1 OR TEGRAX1ARM32 OR TEGRA_T194 OR TEGRA_T234 OR RPI3ARM64 OR RPI4ARM64 OR PHYTIUM OR SD845 OR SD865 OR SD888 OR ADLINK OR A64 OR LX2160A OR ARM64 OR RK3588)
    set(LD80BITS OFF CACHE BOOL "")
    set(NOALIGN OFF CACHE BOOL "")
    set(ARM_DYNAREC ON CACHE BOOL "")
    if(RK3588)
        set(BAD_SIGNAL ON CACHE BOOL "")
    endif()
endif()
if(TERMUX)
    set(ANDROID ON CACHE BOOL "")
    set(TERMUX_PATH "/data/data/com.termux/files")
endif()
if(ANDROID)
    set(NOLOADADDR ON CACHE BOOL "")
    set(BAD_SIGNAL ON CACHE BOOL "")
endif()
option(LD80BITS "Set to ON if host device have 80bits long double (i.e. i386)" ${LD80BITS})
option(NOALIGN "Set to ON if host device doesn't need re-align (i.e. i386)" ${NOALIGN})
option(ARM_DYNAREC "Set to ON to use ARM Dynamic Recompilation" ${ARM_DYNAREC})
option(POWERPCLE "Set to ON if targeting a little-endian PowerPC device" ${POWERPCLE})
option(SYSV_VARARG "Set to ON if host device uses SysVR4 vararg calling convention" ${SYSV_VARARG})

if(${CMAKE_VERSION} VERSION_LESS "3.12.2")
    find_package(PythonInterp 3)
    if(NOT PYTHONINTERP_FOUND)
        message( FATAL_ERROR "You need a Python interpretor, CMake will exit." )
    endif()
    if(${PYTHON_VERSION_MAJOR} LESS 3)
        message( FATAL_ERROR "You need a Python 3 interpretor, CMake will exit." )
    endif()
else()
    find_package(Python3)
    if(NOT Python3_Interpreter_FOUND)
        message( FATAL_ERROR "You need a Python interpretor, CMake will exit." )
    endif()
    set(PYTHON_EXECUTABLE ${Python3_EXECUTABLE} CACHE INTERNAL "The Python3 executable" FORCE)
endif()

cmake_host_system_information(RESULT CMAKE_HOST_SYSTEM_PROCESSOR QUERY OS_PLATFORM)

if("${CMAKE_HOST_SYSTEM_PROCESSOR}" STREQUAL "aarch64" AND (RK3399 OR ODROIDN2 OR TEGRAX1 OR TEGRA_T194 OR TEGRA_T234 OR RPI3ARM64 OR RPI4ARM64 OR PHYTIUM OR SD845 OR SD888 OR ADLINK OR A64 OR LX2160A OR RK3588 OR ARM64))
    set(COMPILER_TRIPLE arm-linux-gnueabihf-gcc)
    find_program(COMPILER_FOUND ${COMPILER_TRIPLE})
    
    if(COMPILER_FOUND)
        set(CMAKE_C_COMPILER ${COMPILER_FOUND})
    else()
        message(STATUS "Cross compiler ${COMPILER_TRIPLE} not found, using CC or -DCMAKE_C_COMPILER")
    endif()
endif()

project(box86 C ASM)

enable_testing()

string(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "i686"  _x86)
string(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "x86_64"  _x86_64)
string(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "aarch64"  _aarch64)
string(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "armhf"  _armhf)
string(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "arm"  _arm)
string(COMPARE EQUAL "${CMAKE_SYSTEM_PROCESSOR}" "armv8l"  _armv8l)

set(default_build_type "RelwithDebInfo")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(PANDORA)
    add_definitions(-DPANDORA)
    #add_definitions(-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp -ftree-vectorize -fsingle-precision-constant -ffast-math)
    set(CMAKE_ASM_FLAGS  "-marm -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp")
elseif(GAMESHELL)
    add_definitions(-DGAMESHELL)
    add_definitions(-marm -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
elseif(PYRA)
    add_definitions(-DPYRA)
    add_definitions(-mcpu=cortex-a15 -mfpu=neon -mfloat-abi=hard -marm)
    set(CMAKE_ASM_FLAGS  "-marm -mcpu=cortex-a15 -mfpu=neon -mfloat-abi=hard")
elseif(RPI2)
    add_definitions(-DRPI)
    add_definitions(-DRPI2)
    add_definitions(-marm -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
elseif(RPI3)
    add_definitions(-DRPI)
    add_definitions(-DRPI3)
    add_definitions(-marm -march=armv8-a+crc -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -march=armv8-a+crc -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(RPI4)
    add_definitions(-DRPI)
    add_definitions(-DRPI4)
    add_definitions(-marm -march=armv8-a+crc -mtune=cortex-a72 -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -march=armv8-a+crc -mtune=cortex-a72 -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(RPI3ARM64)
    add_definitions(-DRPI)
    add_definitions(-DRPI3ARM64)
    add_definitions(-marm -march=armv8-a+crc -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -march=armv8-a+crc -mtune=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(RPI4ARM64)
    add_definitions(-DRPI)
    add_definitions(-DRPI4ARM64)
    add_definitions(-marm -march=armv8-a+crc -mtune=cortex-a72 -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -march=armv8-a+crc -mtune=cortex-a72 -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(ODROIDXU4)
    add_definitions(-DODROIDXU4)
    add_definitions(-marm -mcpu=cortex-a15.cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -mcpu=cortex-a15.cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard")
elseif(RK3288)
    add_definitions(-DRK3288)
    add_definitions(-mcpu=cortex-a17 -mfpu=neon -mfloat-abi=hard -marm)
    set(CMAKE_ASM_FLAGS  "-marm -mcpu=cortex-a17 -mfpu=neon -mfloat-abi=hard")
elseif(SD845)
    add_definitions(-DSD845)
    add_definitions(-marm -march=armv8.2-a+simd+crypto -mtune=cortex-a75.cortex-a55 -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -march=armv8.2-a+simd+crypto -mtune=cortex-a75.cortex-a55 -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(SD865)
    add_definitions(-DSD865)
    #note that cortex-a77.cortex-a55 is not supported, so fall back to a76 instead
    add_definitions(-marm -march=armv8.2-a+simd+crypto -mtune=cortex-a76.cortex-a55 -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -march=armv8.2-a+simd+crypto -mtune=cortex-a76.cortex-a55 -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(SD888)
    add_definitions(-DSD888)
    add_definitions(-marm -march=armv8.4-a+simd+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -march=armv8.4-a+simd+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(ADLINK)
    add_definitions(-DADLINK)
    add_definitions(-marm -mcpu=neoverse-n1 -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -mcpu=neoverse-n1 -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(A64)
    add_definitions(-DA64)
    add_definitions(-marm -march=armv8-a+crc+simd+crypto -mcpu=cortex-a53+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -march=armv8-a+crc+simd+crypto -mcpu=cortex-a53+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(GOA_CLONE)
    add_definitions(-DGOA_CLONE)
    add_definitions(-marm -mcpu=cortex-a35 -mfpu=neon-vfpv3 -march=armv8-a+crc+simd+crypto -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -mcpu=cortex-a35 -mfpu=neon-vfpv3 -march=armv8-a+crc+simd+crypto -mfloat-abi=hard")
elseif(RK3399)
    add_definitions(-DRK3399)
    add_definitions(-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a72+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a72+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(RK3588)
    add_definitions(-DRK3588)
    add_definitions(-pipe -mcpu=cortex-a76.cortex-a55+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    add_definitions(-DBAD_SIGNAL)
    set(CMAKE_ASM_FLAGS  "-pipe -mcpu=cortex-a76.cortex-a55+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(ARM64)
    add_definitions(-DARM64)
    add_definitions(-pipe -march=armv8-a+crc+simd+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crc+simd+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(ODROIDN2)
    add_definitions(-DODROIDN2)
    add_definitions(-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a73.cortex-a53+crypto -mfpu=crypto-neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crc+simd+crypto -mcpu=cortex-a73.cortex-a53+crypto -mfpu=crypto-neon-fp-armv8 -mfloat-abi=hard")
elseif(TEGRAX1)
    add_definitions(-DTEGRAX1)
    add_definitions(-marm -pipe -march=armv8-a+crc -mcpu=cortex-a57 -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -pipe -march=armv8-a+crc -mcpu=cortex-a57 -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(TEGRAX1ARM32)
    add_definitions(-DTEGRAX1ARM32)
    add_definitions(-marm -pipe -march=armv8-a+crc -mcpu=cortex-a57 -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -pipe -march=armv8-a+crc -mcpu=cortex-a57 -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(TEGRA_T194)
    add_definitions(-DTEGRA_T194)
    add_definitions(-marm -pipe -march=armv8.2-a+fp16+simd+crypto+predres -mfpu=crypto-neon-fp-armv8 -mcpu=cortex-a76+crypto -mtune=cortex-a76 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -pipe -march=armv8.2-a+fp16+simd+crypto+predres -mfpu=crypto-neon-fp-armv8 -mcpu=cortex-a76+crypto -mtune=cortex-a76 -mfloat-abi=hard")
elseif(TEGRA_T234)
    add_definitions(-DTEGRA_T234)
    add_definitions(-marm -pipe -march=armv8.2-a+fp16+simd+crypto+predres -mfpu=crypto-neon-fp-armv8 -mcpu=cortex-a78ae+crypto -mtune=cortex-a78ae -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-marm -pipe -march=armv8.2-a+fp16+simd+crypto+predres -mfpu=crypto-neon-fp-armv8 -mcpu=cortex-a78ae+crypto -mtune=cortex-a78ae -mfloat-abi=hard")
elseif(PHYTIUM)
    add_definitions(-DPHYTIUM)
    add_definitions(-pipe -march=armv8.1-a+simd+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8.1-a+simd+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(LX2160A)
    add_definitions(-DLX2160A)
    add_definitions(-pipe -march=armv8-a+crypto+crc -mcpu=cortex-a72+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard)
    set(CMAKE_ASM_FLAGS  "-pipe -march=armv8-a+crypto+crc -mcpu=cortex-a72+crypto -mfpu=neon-fp-armv8 -mfloat-abi=hard")
elseif(ARM_DYNAREC)
    #if DYNAREC is selected alone, without any arch
    if(NOT ANDROID)
        set(CMAKE_ASM_FLAGS  "-pipe -march=armv7-a+simd -mfpu=neon")
    endif()
endif()
if(ANDROID)
    add_definitions(-DANDROID)
endif()
if(TERMUX)
    add_definitions(-DTERMUX)
endif()
if(BAD_SIGNAL)
    add_definitions(-DBAD_SIGNAL)
endif()

if(POWER9)
  add_definitions(-mcpu=power9 -mtune=power9)
endif()

if(NOGIT)
    add_definitions(-DNOGIT)
endif()

if(BOX86LIB)
    set(BOX86 box86)
elseif(HAVE_TRACE)
    set(BOX86 box86)
else()
    set(BOX86 box86)
endif()

try_compile(HAVE_ST_INO ${CMAKE_BINARY_DIR}/compile_tests ${CMAKE_SOURCE_DIR}/tests/test_st_ino.c)

if(HAVE_ST_INO)
  message("stat64 has __st_info field")
  add_definitions(-DHAVE_ST_INO)
endif()

set(BOX86_ELF_ADDRESS "0x62800000")

if(LD80BITS)
    add_definitions(-DHAVE_LD80BITS)
endif()

if(NOALIGN)
    add_definitions(-DNOALIGN)
    add_definitions(-pipe -march=core2 -msse2)
endif()

if(HAVE_TRACE)
    add_definitions(-DHAVE_TRACE)
endif()

if(ARM_DYNAREC)
    add_definitions(-DDYNAREC)
    add_definitions(-DARM)
    add_definitions(-marm)
    enable_language(ASM)
endif()

if(POWERPCLE)
  add_definitions(-DPOWERPCLE)
endif()

if(SYSV_VARARG)
  add_definitions(-DSYSV_VARARG)
endif()

if(NOGETCLOCK)
    add_definitions(-DNOGETCLOCK)
endif()

if(BOX86LIB)
    add_definitions(-DBUILD_LIB)
    if(BOX86DYNAMIC)
        add_definitions(-DBUILD_DYNAMIC)
    endif()
    add_definitions(-fPIC)
    if(NOT BOX86_ROOT)
        message(FATAL_ERROR "BOX86_ROOT not set when build as lib")
    endif()
else()
    set(BOX86_ROOT ${CMAKE_SOURCE_DIR})
endif()


add_definitions(-std=gnu11 -funwind-tables -fvisibility=hidden)

if(USE_CCACHE)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif()
endif()

include_directories(
    "${BOX86_ROOT}/src/include"
    "${BOX86_ROOT}/src"
    "${BOX86_ROOT}/src/wrapped/generated"
)

# git_head.h is a generated file
set_source_files_properties(
    "${BOX86_ROOT}/src/git_head.h"
    PROPERTIES GENERATED TRUE
    HEADER_FILE_ONLY TRUE)

set(ELFLOADER_SRC
    "${BOX86_ROOT}/src/main.c"
    "${BOX86_ROOT}/src/box86context.c"
    "${BOX86_ROOT}/src/build_info.c"
    "${BOX86_ROOT}/src/custommem.c"
    "${BOX86_ROOT}/src/mallochook.c"
    "${BOX86_ROOT}/src/tools/bridge.c"
    "${BOX86_ROOT}/src/tools/pathcoll.c"
    "${BOX86_ROOT}/src/tools/fileutils.c"
    "${BOX86_ROOT}/src/tools/callback.c"
    "${BOX86_ROOT}/src/tools/box86stack.c"
    "${BOX86_ROOT}/src/tools/my_cpuid.c"
    "${BOX86_ROOT}/src/tools/gtkclass.c"
    "${BOX86_ROOT}/src/tools/rcfile.c"
    "${BOX86_ROOT}/src/tools/rbtree.c"
    "${BOX86_ROOT}/src/tools/wine_tools.c"
    "${BOX86_ROOT}/src/elfs/elfloader.c"
    "${BOX86_ROOT}/src/elfs/elfparser.c"
    "${BOX86_ROOT}/src/elfs/elfload_dump.c"
    "${BOX86_ROOT}/src/librarian/library.c"
    "${BOX86_ROOT}/src/librarian/librarian.c"
    "${BOX86_ROOT}/src/librarian/dictionnary.c"
    "${BOX86_ROOT}/src/librarian/symbols.c"
    "${BOX86_ROOT}/src/librarian/globalsymbols.c"
    "${BOX86_ROOT}/src/emu/x86compstrings.c"
    "${BOX86_ROOT}/src/emu/x86emu.c"
    "${BOX86_ROOT}/src/emu/x86run_private.c"
    "${BOX86_ROOT}/src/emu/x86syscall.c"
    "${BOX86_ROOT}/src/emu/x86primop.c"
    "${BOX86_ROOT}/src/emu/x86trace.c"
    "${BOX86_ROOT}/src/emu/x86int3.c"
    "${BOX86_ROOT}/src/emu/x86tls.c"
    "${BOX86_ROOT}/src/emu/x87emu_private.c"
    "${BOX86_ROOT}/src/libtools/auxval.c"
    "${BOX86_ROOT}/src/libtools/myalign.c"
    "${BOX86_ROOT}/src/libtools/myalign64.c"
    "${BOX86_ROOT}/src/libtools/myalignsys64.c"
    "${BOX86_ROOT}/src/libtools/stat64_helper.c"
    "${BOX86_ROOT}/src/libtools/myfts.c"
    "${BOX86_ROOT}/src/libtools/sdl1rwops.c"
    "${BOX86_ROOT}/src/libtools/sdl2rwops.c"
    "${BOX86_ROOT}/src/libtools/signals.c"
    "${BOX86_ROOT}/src/libtools/threads.c"
    "${BOX86_ROOT}/src/libtools/vkalign.c"
    "${BOX86_ROOT}/src/dynarec/dynarec.c"
    "${BOX86_ROOT}/src/wrapped/generated/wrapper.c"
)
if(NOT ANDROID)
    list(APPEND ELFLOADER_SRC "${BOX86_ROOT}/src/libtools/obstack.c")
endif()

set(INTERPRETER
    "${BOX86_ROOT}/src/emu/x86run.c"
    "${BOX86_ROOT}/src/emu/x86run0f.c"
    "${BOX86_ROOT}/src/emu/x86run64.c"
    "${BOX86_ROOT}/src/emu/x86run640f.c"
    "${BOX86_ROOT}/src/emu/x86run6466.c"
    "${BOX86_ROOT}/src/emu/x86run6467.c"
    "${BOX86_ROOT}/src/emu/x86run66.c"
    "${BOX86_ROOT}/src/emu/x86run660f.c"
    "${BOX86_ROOT}/src/emu/x86run66d9.c"
    "${BOX86_ROOT}/src/emu/x86run66dd.c"
    "${BOX86_ROOT}/src/emu/x86run66f20f.c"
    "${BOX86_ROOT}/src/emu/x86run67.c"
    "${BOX86_ROOT}/src/emu/x86run6766.c"
    "${BOX86_ROOT}/src/emu/x86rund8.c"
    "${BOX86_ROOT}/src/emu/x86rund9.c"
    "${BOX86_ROOT}/src/emu/x86runda.c"
    "${BOX86_ROOT}/src/emu/x86rundb.c"
    "${BOX86_ROOT}/src/emu/x86rundc.c"
    "${BOX86_ROOT}/src/emu/x86rundd.c"
    "${BOX86_ROOT}/src/emu/x86runde.c"
    "${BOX86_ROOT}/src/emu/x86rundf.c"
    "${BOX86_ROOT}/src/emu/x86runf0.c"
    "${BOX86_ROOT}/src/emu/x86runf066.c"
    "${BOX86_ROOT}/src/emu/x86runf20f.c"
    "${BOX86_ROOT}/src/emu/x86runf30f.c"
)

set(WRAPPEDS
    "${BOX86_ROOT}/src/wrapped/wrappedlibc.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibpthread.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibrt.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibbsd.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibgl.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibglu.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibglx.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibx11.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibasound.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibdl.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibm.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsdl1.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsdl1mixer.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsdl1net.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsdl1image.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsdl1sound.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsdl1ttf.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsdl2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsdl2mixer.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsdl2net.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsdl2image.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsdl2ttf.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsmpeg.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsmpeg2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedvorbisfile.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibvorbis.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibogg.c"
    "${BOX86_ROOT}/src/wrapped/wrappedpng12.c"
    "${BOX86_ROOT}/src/wrapped/wrappedpng16.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxxf86vm.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxrandr.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxpresent.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxext.c"
    "${BOX86_ROOT}/src/wrapped/wrappedxinerama.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxfixes.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcursor.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxrender.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxft.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxi.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxss.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxpm.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxau.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxdmcp.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibx11xcb.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcb.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcbxfixes.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcbshape.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcbshm.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcbrandr.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcbimage.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcbkeysyms.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcbxtest.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcbdri2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcbdri3.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcbpresent.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxtst.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxt.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxcomposite.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxdamage.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibxmu.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibz.c"
    "${BOX86_ROOT}/src/wrapped/wrappedopenal.c"
    "${BOX86_ROOT}/src/wrapped/wrappedalure.c"
    "${BOX86_ROOT}/src/wrapped/wrappedalut.c"
    "${BOX86_ROOT}/src/wrapped/wrappedopencl.c"
    "${BOX86_ROOT}/src/wrapped/wrappedcurl.c"
    "${BOX86_ROOT}/src/wrapped/wrappedudev0.c"
    "${BOX86_ROOT}/src/wrapped/wrappedudev1.c"
    "${BOX86_ROOT}/src/wrapped/wrappeddbus.c"
    "${BOX86_ROOT}/src/wrapped/wrappediconv.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibdrm.c"
    "${BOX86_ROOT}/src/wrapped/wrappedcrypto.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibssl.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibcrypt.c"
    "${BOX86_ROOT}/src/wrapped/wrappedutil.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibuuid.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibresolv.c"
    "${BOX86_ROOT}/src/wrapped/wrappedpulsesimple.c"
    "${BOX86_ROOT}/src/wrapped/wrappedpulse.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibsndfile.c"
    "${BOX86_ROOT}/src/wrapped/wrappedfontconfig.c"
    "${BOX86_ROOT}/src/wrapped/wrappedfreetype.c"
    "${BOX86_ROOT}/src/wrapped/wrappedbz2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedexpat.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibsm.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibice.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibusb1.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibncursesw.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibformw.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibncurses.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibtinfo.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibpanel.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibncurses6.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibtinfo6.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgmp.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgcrypt.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgtkx112.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgdkx112.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgtk3.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgdk3.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgdkpixbuf2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgio2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgmodule2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgobject2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedglib2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgconf2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgstreamer010.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgstinterfaces010.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgstreamer.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgstaudio.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgstvideo.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgsttag.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgstbase.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgstapp.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibibus.c"
    "${BOX86_ROOT}/src/wrapped/wrappeddbusmenugtk.c"
    "${BOX86_ROOT}/src/wrapped/wrappeddbusmenuglib.c"
    "${BOX86_ROOT}/src/wrapped/wrappedappindicator.c"
    "${BOX86_ROOT}/src/wrapped/wrappedappindicator3.c"
    "${BOX86_ROOT}/src/wrapped/wrappedayatanaappindicator3.c"
    "${BOX86_ROOT}/src/wrapped/wrappedatk.c"
    "${BOX86_ROOT}/src/wrapped/wrappedatkbridge.c"
    "${BOX86_ROOT}/src/wrapped/wrappedpangoft2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedpangocairo.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgthread2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedcairo.c"
    "${BOX86_ROOT}/src/wrapped/wrappedcairogobject.c"
    "${BOX86_ROOT}/src/wrapped/wrappedpango.c"
    "${BOX86_ROOT}/src/wrapped/wrappeddbusglib1.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgudev1.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibnm.c"
    "${BOX86_ROOT}/src/wrapped/wrappedtcmallocminimal.c"
    "${BOX86_ROOT}/src/wrapped/wrappedmpg123.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgnutls.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibcups.c"
    "${BOX86_ROOT}/src/wrapped/wrappedd3dadapter9.c"
    "${BOX86_ROOT}/src/wrapped/wrappedvulkan.c"
    "${BOX86_ROOT}/src/wrapped/wrappedwaylandclient.c"
    "${BOX86_ROOT}/src/wrapped/wrappedxml2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedxslt.c"
    "${BOX86_ROOT}/src/wrapped/wrappedldapr.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlber.c"
    "${BOX86_ROOT}/src/wrapped/wrappednsl.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlcms2.c"
    "${BOX86_ROOT}/src/wrapped/wrappedkrb5.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgssapikrb5.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibtiff.c"
    "${BOX86_ROOT}/src/wrapped/wrappedldlinux.c"
    "${BOX86_ROOT}/src/wrapped/wrappedcrashhandler.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibv4l2.c"
    "${BOX86_ROOT}/src/wrapped/wrappednspr4.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsmime3.c"
    "${BOX86_ROOT}/src/wrapped/wrappednssutil3.c"
    "${BOX86_ROOT}/src/wrapped/wrappednss3.c"
    "${BOX86_ROOT}/src/wrapped/wrappedplds4.c"
    "${BOX86_ROOT}/src/wrapped/wrappedplc4.c"
    "${BOX86_ROOT}/src/wrapped/wrappedssl3.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsoftokn3.c"
    "${BOX86_ROOT}/src/wrapped/wrappedfreebl3.c"
    "${BOX86_ROOT}/src/wrapped/wrappedsecret1.c"
    "${BOX86_ROOT}/src/wrapped/wrappedtbbmalloc.c"
    "${BOX86_ROOT}/src/wrapped/wrappedmimalloc.c"
    "${BOX86_ROOT}/src/wrapped/wrappedselinux.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibharfbuzz.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibcairogobject.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibvkd3d.c"
    "${BOX86_ROOT}/src/wrapped/wrappedgomp.c"
    "${BOX86_ROOT}/src/wrapped/wrappedcap.c"
    "${BOX86_ROOT}/src/wrapped/wrappedpcap.c"
    "${BOX86_ROOT}/src/wrapped/wrappedfaudio.c"
    "${BOX86_ROOT}/src/wrapped/wrappedtbbmallocproxy.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibfuse.c"
    "${BOX86_ROOT}/src/wrapped/wrappedatomic.c"
    "${BOX86_ROOT}/src/wrapped/wrappedlibpcre.c"
)
if(ANDROID)
    list(APPEND WRAPPEDS
        "${BOX86_ROOT}/src/wrapped/wrappedandroidshmem.c"
    )
else()
    list(APPEND WRAPPEDS
        "${BOX86_ROOT}/src/wrapped/wrappedlibjpeg.c"
        "${BOX86_ROOT}/src/wrapped/wrappedlibjpeg62.c"
        "${BOX86_ROOT}/src/wrapped/wrappedturbojpeg.c"
    )
endif()

# If BOX86_ROOT contains a ".c", the build breaks...
string(REPLACE ".c" "_private.h" MODROOT ${BOX86_ROOT})
set(WRAPPEDS_HEAD "${BOX86_ROOT}/src/wrapped/wrappedd3dadapter9_genvate.h")
foreach(A ${WRAPPEDS})
    string(REPLACE ".c" "_private.h" C ${A})
    string(REPLACE "${MODROOT}" "${BOX86_ROOT}" B ${C})
    set(WRAPPEDS_HEAD ${WRAPPEDS_HEAD} ${B})
    set_source_files_properties(${A} PROPERTIES OBJECT_DEPENDS ${B})
endforeach()

set(WRAPPER "${BOX86_ROOT}/src/wrapped/generated/wrapper.c" "${BOX86_ROOT}/src/wrapped/generated/wrapper.h")

add_custom_command(
    OUTPUT "${BOX86_ROOT}/src/wrapped/generated/functions_list.txt"
    COMMAND "${PYTHON_EXECUTABLE}" "${BOX86_ROOT}/rebuild_wrappers.py" 
    "${BOX86_ROOT}" 
    "PANDORA" "HAVE_LD80BITS" "NOALIGN" "HAVE_TRACE" "POWERPCLE" "ANDROID" "TERMUX" "--" 
    ${WRAPPEDS_HEAD}
    MAIN_DEPENDENCY "${BOX86_ROOT}/rebuild_wrappers.py"
    DEPENDS ${WRAPPEDS} ${WRAPPEDS_HEAD}
    BYPRODUCTS ${WRAPPER}
)

add_custom_command(
    OUTPUT "${BOX86_ROOT}/src/dynarec/last_run.txt"
    COMMAND "${PYTHON_EXECUTABLE}" "${BOX86_ROOT}/rebuild_printer.py" "${BOX86_ROOT}"
    MAIN_DEPENDENCY "${BOX86_ROOT}/rebuild_printer.py"
    DEPENDS "${BOX86_ROOT}/src/dynarec/arm_instructions.txt"
    BYPRODUCTS "${BOX86_ROOT}/src/dynarec/arm_printer.c"
)

add_custom_target(WRAPPERS DEPENDS "${BOX86_ROOT}/src/wrapped/generated/functions_list.txt")
add_custom_target(PRINTER DEPENDS "${BOX86_ROOT}/src/dynarec/last_run.txt")

if(ARM_DYNAREC)
    set(DYNAREC_SRC
        "${BOX86_ROOT}/src/dynarec/dynablock.c"

        "${BOX86_ROOT}/src/dynarec/dynarec_arm.c"
        "${BOX86_ROOT}/src/dynarec/dynarec_arm_functions.c"
        "${BOX86_ROOT}/src/dynarec/dynarec_arm_jmpnext.c"
        "${BOX86_ROOT}/src/dynarec/arm_printer.c"
        "${BOX86_ROOT}/src/emu/x86test.c"

        "${BOX86_ROOT}/src/dynarec/arm_prolog.S"
        "${BOX86_ROOT}/src/dynarec/arm_epilog.S"
        "${BOX86_ROOT}/src/dynarec/arm_next.S"
        "${BOX86_ROOT}/src/dynarec/arm_lock_helper.S"
    )

    set(DYNAREC_PASS
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_helper.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_emit_tests.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_emit_math.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_emit_logic.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_emit_shift.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_pass.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_00.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_0f.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_64.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_65.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_66.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_67.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_d8.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_d9.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_da.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_db.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_dc.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_dd.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_de.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_df.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_f0.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_660f.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_66f0.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_f20f.c"
    "${BOX86_ROOT}/src/dynarec/dynarec_arm_f30f.c"
    )

    set(DYNAREC_PASS "${BOX86_ROOT}/src/wrapped/generated/wrapper.h" ${DYNAREC_PASS})

    add_library(dynarec_arm OBJECT ${DYNAREC_SRC})

    add_library(arm_pass0 OBJECT ${DYNAREC_PASS})
    set_target_properties(arm_pass0 PROPERTIES COMPILE_FLAGS "-DSTEP=0")
    add_library(arm_pass1 OBJECT ${DYNAREC_PASS})
    set_target_properties(arm_pass1 PROPERTIES COMPILE_FLAGS "-DSTEP=1")
    add_library(arm_pass2 OBJECT ${DYNAREC_PASS})
    set_target_properties(arm_pass2 PROPERTIES COMPILE_FLAGS "-DSTEP=2")
    add_library(arm_pass3 OBJECT ${DYNAREC_PASS})
    set_target_properties(arm_pass3 PROPERTIES COMPILE_FLAGS "-DSTEP=3")
    add_library(test_interpreter OBJECT ${INTERPRETER})
    set_target_properties(test_interpreter PROPERTIES COMPILE_FLAGS "-DTEST_INTERPRETER")
    add_dependencies(arm_pass0 WRAPPERS)
    add_dependencies(arm_pass1 WRAPPERS)
    add_dependencies(arm_pass2 WRAPPERS)
    add_dependencies(arm_pass3 WRAPPERS)

    add_library(dynarec STATIC 
        $<TARGET_OBJECTS:dynarec_arm> 
        $<TARGET_OBJECTS:arm_pass0> 
        $<TARGET_OBJECTS:arm_pass1>
        $<TARGET_OBJECTS:arm_pass2>
        $<TARGET_OBJECTS:arm_pass3>
        $<TARGET_OBJECTS:test_interpreter>
    )

endif()

# creates git_head.h
if(ARM_DYNAREC)
    add_custom_command(
        OUTPUT "${BOX86_ROOT}/src/git_head.h"
        COMMAND bash -c "echo \\\#define GITREV \\\"$(git rev-parse --short HEAD)\\\">\"${BOX86_ROOT}/src/git_head.h\""
        DEPENDS dynarec ${ELFLOADER_SRC} ${INTERPRETER} ${WRAPPEDS}
        VERBATIM)
else()
    add_custom_command(
        OUTPUT "${BOX86_ROOT}/src/git_head.h"
        COMMAND bash -c "echo \\\#define GITREV \\\"$(git rev-parse --short HEAD)\\\">\"${BOX86_ROOT}/src/git_head.h\""
        DEPENDS ${ELFLOADER_SRC} ${INTERPRETER} ${WRAPPEDS}
        VERBATIM)
endif()

add_library(interpreter OBJECT ${INTERPRETER})

if(BOX86LIB)
#build a static or dynamic lib...
if(BOX86DYNAMIC)
    add_library(box86 SHARED ${ELFLOADER_SRC} ${WRAPPEDS} "${BOX86_ROOT}/src/git_head.h")
else()
    add_library(${BOX86} OBJECT ${ELFLOADER_SRC} ${WRAPPEDS} "${BOX86_ROOT}/src/git_head.h")
endif()
if(ARM_DYNAREC)
    if(ANDROID)
        if(TERMUX)
        target_link_libraries(${BOX86} dynarec interpreter m dl android-sysv-semaphore android-spawn)
        else()
        target_link_libraries(${BOX86} dynarec interpreter m dl)
        endif()
    else()
        target_link_libraries(${BOX86} dynarec interpreter m dl rt pthread resolv)
    endif()
endif()
if(${CMAKE_VERSION} VERSION_LESS "3.13")
    set_target_properties(${BOX86} PROPERTIES LINK_FLAGS -rdynamic)
else()
    target_link_options(${BOX86} PUBLIC -rdynamic)
endif()
add_dependencies(${BOX86} WRAPPERS)
add_dependencies(${BOX86} PRINTER)

add_custom_target(buildinfo ${BOX86}
    DEPENDS dynarec ${BOX86} )

else(BOX86LIB)

add_executable(${BOX86} ${ELFLOADER_SRC} ${WRAPPEDS} "${BOX86_ROOT}/src/git_head.h")
add_dependencies(${BOX86} WRAPPERS)
add_dependencies(${BOX86} PRINTER)
if(ANDROID)
    if(TERMUX)
    target_link_libraries(${BOX86} dynarec interpreter m dl android-complex-math android-sysv-semaphore android-spawn )
    else()
    target_link_libraries(${BOX86} dynarec interpreter m dl)
    endif()
    target_link_libraries(${BOX86} c m dl)
else()
    #target_link_libraries(${BOX86} c m dl rt pthread resolv)
    set_target_properties(${BOX86} PROPERTIES LINK_FLAGS "-Wl,--no-as-needed -lc -lm -ldl -lrt -lpthread -lresolv -Wl,--as-needed -pthread")
endif()
if(ARM_DYNAREC)
    target_link_libraries(${BOX86} dynarec interpreter)
else()
    target_link_libraries(${BOX86} interpreter)
endif()

if(${CMAKE_VERSION} VERSION_LESS "3.13")
    if(NOT NOLOADADDR)
        if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
            set_target_properties(${BOX86} PROPERTIES LINK_FLAGS "-rdynamic -Wl,--image-base,${BOX86_ELF_ADDRESS}")
        else()
            set_target_properties(${BOX86} PROPERTIES LINK_FLAGS "-rdynamic -Wl,-Ttext-segment,${BOX86_ELF_ADDRESS}")
        endif()
    else()
        set_target_properties(${BOX86} PROPERTIES LINK_FLAGS -rdynamic)
    endif()
else()
    target_link_options(${BOX86} PUBLIC -rdynamic)
    if(NOT NOLOADADDR)
        if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
            target_link_options(${BOX86} PUBLIC -Wl,--image-base,${BOX86_ELF_ADDRESS})
        else()
            target_link_options(${BOX86} PUBLIC -Wl,-Ttext-segment,${BOX86_ELF_ADDRESS})
        endif()
    endif()
endif()

if(NOT _x86 AND NOT _x86_64)
  if(NOT TERMUX)
   install(TARGETS ${BOX86}
    RUNTIME DESTINATION bin)
  else()
   install(TARGETS ${BOX86}
    RUNTIME DESTINATION ${TERMUX_PATH}/usr/bin)
  endif()
  if(NOT NO_CONF_INSTALL)
    configure_file(system/box86.conf.cmake system/box86.conf)
    	if(NOT TERMUX)
    		install(FILES ${CMAKE_BINARY_DIR}/system/box86.conf DESTINATION /etc/binfmt.d/)
		install(FILES ${CMAKE_SOURCE_DIR}/system/box86.box86rc DESTINATION /etc/)
	else()
		#install(FILES ${CMAKE_BINARY_DIR}/system/box86.conf DESTINATION ${TERMUX_PATH}/usr/etc/binfmt.d/)
		install(FILES ${CMAKE_SOURCE_DIR}/system/box86.box86rc DESTINATION ${TERMUX_PATH}/usr/etc/)
	endif()
  endif()
  set(INSTALL_PATH "/usr/lib/box86-i386-linux-gnu/")
  if(NOT NO_LIB_INSTALL)
    if(NOT TERMUX)
    	install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libstdc++.so.6 DESTINATION ${INSTALL_PATH})
    	install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libstdc++.so.5 DESTINATION ${INSTALL_PATH})
    	install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libgcc_s.so.1 DESTINATION ${INSTALL_PATH})
    	install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libpng12.so.0 DESTINATION ${INSTALL_PATH})
    	install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libunwind.so.8 DESTINATION ${INSTALL_PATH})
    else()

	    install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libstdc++.so.6 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
	    install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libstdc++.so.5 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
	    install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libgcc_s.so.1 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
	    install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libpng12.so.0 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
	    install(FILES ${CMAKE_SOURCE_DIR}/x86lib/libunwind.so.8 DESTINATION ${TERMUX_PATH}${INSTALL_PATH})
    endif()
  endif()
endif()

if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ptitSeb")
set(CPACK_PACKAGE_CONTACT "ptitSeb@box86.org")
set(CPACK_PACKAGE_DESCRIPTION "Box86 - Linux Userspace x86 Emulator with a twist")
if(TERMUX)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libandroid-sysv-semaphore, libandroid-spawn")
else()
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")
endif()
set(CPACK_PACKAGE_HOMEPAGE_URL, "https://box86.org")
file(STRINGS "${BOX86_ROOT}/src/box86version.h" TEMP_STRING REGEX "BOX86_MAJOR\\s*")
string(REGEX REPLACE "BOX86_MAJOR" "" TEMP_STRING ${TEMP_STRING})
string(REGEX MATCH "[0-9]" BOX86_MAJOR ${TEMP_STRING})
file(STRINGS "${BOX86_ROOT}/src/box86version.h" TEMP_STRING REGEX "BOX86_MINOR\\s*")
string(REGEX REPLACE "BOX86_MINOR" "" TEMP_STRING ${TEMP_STRING})
string(REGEX MATCH "[0-9]" BOX86_MINOR ${TEMP_STRING})
file(STRINGS "${BOX86_ROOT}/src/box86version.h" TEMP_STRING REGEX "BOX86_REVISION\\s*")
string(REGEX REPLACE "BOX86_REVISION" "" TEMP_STRING ${TEMP_STRING})
string(REGEX MATCH "[0-9]" BOX86_REVISION ${TEMP_STRING})
set(CPACK_PACKAGE_VERSION_MAJOR ${BOX86_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${BOX86_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${BOX86_REVISION})
if(NOT TERMUX)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/postinst")
endif()
if(_aarch64 OR _armhf OR _arm OR _armv8l)
    if(TERMUX)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm")
    else()
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
    endif()
elseif(_x86_64 OR _x86)
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
else()
    #probably wrong...
    execute_process(COMMAND "dpkg --print-architecture" OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
endif()
if(TERMUX)
set(CPACK_PACKAGING_INSTALL_PREFIX "${TERMUX_PATH}/usr")
endif()
if(NOT TERMUX)
set(CPACK_DEBIAN_FILE_NAME "${BOX86}-${BOX86_MAJOR}.${BOX86_MINOR}.${BOX86_REVISION}_${CMAKE_SYSTEM_NAME}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")
else()
set(CPACK_DEBIAN_FILE_NAME "${BOX86}-${BOX86_MAJOR}.${BOX86_MINOR}.${BOX86_REVISION}_Termux-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")
endif()
INCLUDE(CPack)

add_test(NAME bootSyscall COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test01 -D TEST_OUTPUT=tmpfile01.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref01.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME bootSyscallC COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test02 -D TEST_OUTPUT=tmpfile02.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref02.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME printf COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test03 -D TEST_OUTPUT=tmpfile03.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref03.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME args COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test04 -D TEST_ARGS2=yeah -D TEST_OUTPUT=tmpfile04.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref04.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME maths1 COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test05 -D TEST_ARGS2=7 -D TEST_OUTPUT=tmpfile05.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref05.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME threadsStart COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test06 -D TEST_OUTPUT=tmpfile06.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref06.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME trig COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test07 -D TEST_OUTPUT=tmpfile07.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref07.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME pi COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test08 -D TEST_OUTPUT=tmpfile08.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref08.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME fork COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test09 -D TEST_OUTPUT=tmpfile09.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref09.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME cppThreads COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test10 -D TEST_OUTPUT=tmpfile10.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref10.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME tlsData COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test11 -D TEST_OUTPUT=tmpfile11.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref11.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME fpu COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test12 -D TEST_OUTPUT=tmpfile12.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref12.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME contexts COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test13 -D TEST_OUTPUT=tmpfile13.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref13.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

if(NOT LD80BITS)
    add_test(NAME conditionalThreads COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test14 -D TEST_OUTPUT=tmpfile14.txt 
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref14.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )
endif()

add_test(NAME linkingIndirectNoversion COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test15 -D TEST_OUTPUT=tmpfile15.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref15.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME linkingIndirectVersion COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test16 -D TEST_OUTPUT=tmpfile16.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref16.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME sse COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test17 -D TEST_OUTPUT=tmpfile17.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref17.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

set_tests_properties(sse PROPERTIES ENVIRONMENT "BOX86_DYNAREC_FASTNAN=0;BOX86_DYNAREC_FASTROUND=0")

add_test(NAME longjumpInSignals COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test18 -D TEST_OUTPUT=tmpfile18.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref18.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME x87 COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test19 -D TEST_OUTPUT=tmpfile19.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref19.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME idiv COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test20 -D TEST_OUTPUT=tmpfile20.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref20.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME multiple_dlopen COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test21 -D TEST_OUTPUT=tmpfile21.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref21.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(NAME aes COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test22 -D TEST_OUTPUT=tmpfile22.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref22.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

file(GLOB extension_tests "${CMAKE_SOURCE_DIR}/tests/extensions/*.c")    
foreach(file ${extension_tests})
    get_filename_component(testname "${file}" NAME_WE)
    add_test(NAME "${testname}" COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/extensions/${testname} -D TEST_OUTPUT=tmpfile-${testname}.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/extensions/${testname}.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake)
endforeach()        

add_test(NAME sse_optimized COMMAND ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86} 
    -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test17_o2 -D TEST_OUTPUT=tmpfile17_o2.txt 
    -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref17_o2.txt
    -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

set_tests_properties(sse_optimized PROPERTIES ENVIRONMENT "BOX86_DYNAREC_FASTNAN=0;BOX86_DYNAREC_FASTROUND=0")    
    
add_test(bswap ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test23 -D TEST_OUTPUT=tmpfile23.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref23.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )
    
add_test(feround ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test24 -D TEST_OUTPUT=tmpfile24.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref24.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(sse4_2 ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test25 -D TEST_OUTPUT=tmpfile25.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref25.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

add_test(fpu_rounding ${CMAKE_COMMAND} -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/${BOX86}
        -D TEST_ARGS=${CMAKE_SOURCE_DIR}/tests/test26 -D TEST_OUTPUT=tmpfile26.txt
        -D TEST_REFERENCE=${CMAKE_SOURCE_DIR}/tests/ref26.txt
        -P ${CMAKE_SOURCE_DIR}/runTest.cmake )

set_tests_properties(fpu_rounding PROPERTIES ENVIRONMENT "BOX86_DYNAREC_FASTROUND=0")
    
endif(BOX86LIB)
