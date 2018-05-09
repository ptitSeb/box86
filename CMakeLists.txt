cmake_minimum_required(VERSION 2.6)

project(box86)

option(PANDORA "Set to ON if targeting an OpenPandora device" ${PANDORA})

# Pandora
if(PANDORA)
    add_definitions(-DPANDORA)
    add_definitions(-mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp -ftree-vectorize -fsingle-precision-constant -ffast-math)
endif()

add_definitions(-g -std=gnu99 -funwind-tables -O3 -fvisibility=hidden)

#include_directories(include)

SET(ELFLOADER_SRC
    src/main.c
)

add_executable(box86 ${ELFLOADER_SRC})
target_link_libraries(box86 m dl rt)
