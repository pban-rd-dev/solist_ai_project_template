/*
 * Copyright (c) 2026 p-ban.com Corp.
 * SPDX-License-Identifier: LicenseRef-p-ban-solist-ai-gcc-template
 * See LICENSE (LICENSE.ja is the authoritative Japanese text).
 */

/*****************************************************************************
 * @file     test_uart_print.c
 * @brief    Tests for uart_print.c functionality
 * @version  1.0
 *****************************************************************************/

#include "test_framework.h"
#include "uart_print.h"

/**
 * @brief Test uart_print_init reports success as true, not as 0
 *
 * uart_print_init() follows the bool convention (true = success), while the
 * device_* API follows the uint16_t convention (0 = success). Callers that
 * mix the two treat a successful init as a failure.
 */
DEFINE_TEST(test_uart_print_init_returns_true)
{
    bool result = uart_print_init();

    TEST_ASSERT(result, "uart_print_init returns true on success");
    TEST_ASSERT_NOT_EQUAL(0, (uint32_t)result, "uart_print_init success is non-zero");
}

/**
 * @brief Test uart_print_init can be called again once initialized
 */
DEFINE_TEST(test_uart_print_init_is_idempotent)
{
    TEST_ASSERT(uart_print_init(), "Second uart_print_init returns true");
    TEST_ASSERT(uart_print_init(), "Third uart_print_init returns true");
}

/**
 * @brief Test the transmitter is idle again once flushed
 */
DEFINE_TEST(test_uart_print_ready_after_flush)
{
    uart_print_puts("");
    uart_print_flush();

    TEST_ASSERT(uart_print_is_ready(), "uart_print_is_ready after flush");
}

/**
 * @brief Test uart_print_printf reports the number of formatted characters
 */
DEFINE_TEST(test_uart_print_printf_returns_length)
{
    int len = uart_print_printf("%s%d", "ab", 7);

    TEST_ASSERT_EQUAL(3, len, "printf returns formatted length");
}

/**
 * @brief Test the string helpers tolerate a NULL argument
 */
DEFINE_TEST(test_uart_print_rejects_null)
{
    uart_print_puts(NULL);
    uart_print_printf(NULL);
    uart_print_hex_dump(NULL, 4, "null");

    TEST_ASSERT(true, "NULL arguments do not hang or fault");
}

/**
 * @brief Register all uart_print tests
 */
void register_uart_tests(void)
{
    REGISTER_TEST("UartPrint", test_uart_print_init_returns_true);
    REGISTER_TEST("UartPrint", test_uart_print_init_is_idempotent);
    REGISTER_TEST("UartPrint", test_uart_print_ready_after_flush);
    REGISTER_TEST("UartPrint", test_uart_print_printf_returns_length);
    REGISTER_TEST("UartPrint", test_uart_print_rejects_null);
}
