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

# The Solist-AI accelerator library (solistAi_library) and Solist-AI Scope
# are distributed by ROHM as Arm Compiler (armclang) archives and are not
# bundled here. Obtain them from ROHM if your application needs them, then
# link the GCC-compatible build yourself.

# Add ML63Q25x7 include path
target_include_directories(driver PUBLIC
    ${CMAKE_SOURCE_DIR}/ml63q25x7/include
    ${CMAKE_SOURCE_DIR}/external/CMSIS/CMSIS/Core/Include
    ${CMAKE_SOURCE_DIR}/src
)

# Driver-specific compile definitions
target_compile_definitions(driver PRIVATE
    ${MCU_VARIANT}
)
