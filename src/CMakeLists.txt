# Copyright (c) 2026 p-ban.com Corp.
# SPDX-License-Identifier: LicenseRef-p-ban-solist-ai-gcc-template
# See LICENSE (LICENSE.ja is the authoritative Japanese text).

# Application source files

set(APP_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/syscalls.c
  ${CMAKE_CURRENT_SOURCE_DIR}/app_irq.c
  ${CMAKE_CURRENT_SOURCE_DIR}/device.c
  ${CMAKE_CURRENT_SOURCE_DIR}/uart_print.c
  ${CMAKE_CURRENT_SOURCE_DIR}/uartf_int.c
  ${CMAKE_CURRENT_SOURCE_DIR}/codeoption.c
  PARENT_SCOPE
)
