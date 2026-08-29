# Copyright (c) 2026 p-ban.com Corp.
# SPDX-License-Identifier: LicenseRef-p-ban-solist-ai-gcc-template
# See LICENSE (LICENSE.ja is the authoritative Japanese text).

# Utility library CMakeLists.txt

# Collect utility source files
set(UTILITY_SOURCES
    board/smpl_common.c
    board/smpl_common_led.c
)

# Create utility library
add_library(utility STATIC ${UTILITY_SOURCES})

# Include directories
target_include_directories(utility PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/board
)

# Link with driver library for access to peripheral headers
target_link_libraries(utility PUBLIC
    driver
)

# Utility-specific compile definitions
target_compile_definitions(utility PRIVATE
    ${MCU_VARIANT}
)