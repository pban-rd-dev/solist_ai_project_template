# Copyright (c) 2026 p-ban.com Corp.
# SPDX-License-Identifier: LicenseRef-p-ban-solist-ai-gcc-template
# See LICENSE (LICENSE.ja is the authoritative Japanese text).

# Driver library CMakeLists.txt

# Collect driver source files
set(DRIVER_SOURCES
    src/irq.c
    src/ssiof0.c
    src/wdt.c
    src/uartf0.c
)

# Create driver library
add_library(driver STATIC ${DRIVER_SOURCES})

# Include directories
target_include_directories(driver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# ROHM Solist-AI precompiled library selection
# Two library variants available:
#   SOLIST_AI_LIB_512: 1 model, 512 inputs, 64 hidden (default)
#   SOLIST_AI_LIB_256: 2 models, 256 inputs, 64 hidden
option(SOLIST_AI_LIB_512 "Use 512-input library (1 model)" ON)

if(SOLIST_AI_LIB_512)
    set(SOLIST_AI_LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/SolistAi_Library_1_512_64.a")
    set(SOLIST_AI_MAX_MODELS 1)
    set(SOLIST_AI_MAX_INPUTS 512)
    set(SOLIST_AI_MAX_HIDDEN 64)
else()
    set(SOLIST_AI_LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/SolistAi_Library_2_256_64.a")
    set(SOLIST_AI_MAX_MODELS 2)
    set(SOLIST_AI_MAX_INPUTS 256)
    set(SOLIST_AI_MAX_HIDDEN 64)
endif()

message(STATUS "Solist-AI library: ${SOLIST_AI_LIB}")
message(STATUS "  Max models: ${SOLIST_AI_MAX_MODELS}, Max inputs: ${SOLIST_AI_MAX_INPUTS}, Max hidden: ${SOLIST_AI_MAX_HIDDEN}")

target_link_libraries(driver PUBLIC
    ${SOLIST_AI_LIB}
)

# Add ML63Q25x7 include path
target_include_directories(driver PUBLIC
    ${CMAKE_SOURCE_DIR}/ml63q25x7/include
    ${CMAKE_SOURCE_DIR}/external/CMSIS/CMSIS/Core/Include
)

# Driver-specific compile definitions
target_compile_definitions(driver PUBLIC
    ${MCU_VARIANT}
    ODL_MAX_INST_NUM=${SOLIST_AI_MAX_MODELS}
    ODL_MAX_INPUTS=${SOLIST_AI_MAX_INPUTS}
    ODL_MAX_UNITS=${SOLIST_AI_MAX_HIDDEN}
)
