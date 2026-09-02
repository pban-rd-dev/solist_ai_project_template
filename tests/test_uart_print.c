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
#include "ML63Q25x7.h"
#include "rdwr_reg.h"

/* P3MOD0 layout, restated here rather than shared with uart_print.c so that a
 * wrong constant in the driver cannot make the test agree with it. One 6-bit
 * mode field per pin, on an 8-bit stride. */
#define P3MOD_FIELD(reg, pin)   (((reg) >> ((uint32_t)(pin) * 8U)) & 0x3FU)
#define P3_PIN_RX               2U      /* P32 */
#define P3_PIN_TX               3U      /* P33 */
#define P3_MODE_GPIO            0x0AU
#define P3_MODE_UARTF0_RX       0x21U
#define P3_MODE_UARTF0_TX       0x23U

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
 * @brief Test deinit hands P32/P33 back without disturbing P30/P31
 *
 * uart_print_deinit() used to clear the wrong bit range, which left P33 in
 * UARTF0 mode and corrupted the mode fields of the two pins below it.
 */
DEFINE_TEST(test_uart_print_deinit_restores_port3)
{
    uint32_t before = read_reg32(PORT3->P3MOD0);

    uart_print_deinit();
    uint32_t deinited = read_reg32(PORT3->P3MOD0);

    bool reinit = uart_print_init();
    uint32_t reinited = read_reg32(PORT3->P3MOD0);

    /* UART is back up from here, so assertions can report. */
    TEST_ASSERT(reinit, "uart_print_init succeeds after deinit");

    TEST_ASSERT_EQUAL(P3_MODE_GPIO, P3MOD_FIELD(deinited, P3_PIN_RX),
                      "deinit returns P32 to general-purpose mode");
    TEST_ASSERT_EQUAL(P3_MODE_GPIO, P3MOD_FIELD(deinited, P3_PIN_TX),
                      "deinit returns P33 to general-purpose mode");

    TEST_ASSERT_EQUAL(P3MOD_FIELD(before, 0), P3MOD_FIELD(deinited, 0),
                      "deinit leaves P30 mode untouched");
    TEST_ASSERT_EQUAL(P3MOD_FIELD(before, 1), P3MOD_FIELD(deinited, 1),
                      "deinit leaves P31 mode untouched");

    TEST_ASSERT_EQUAL(P3_MODE_UARTF0_RX, P3MOD_FIELD(reinited, P3_PIN_RX),
                      "re-init puts P32 back on UARTF0_RX");
    TEST_ASSERT_EQUAL(P3_MODE_UARTF0_TX, P3MOD_FIELD(reinited, P3_PIN_TX),
                      "re-init puts P33 back on UARTF0_TX");
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
    REGISTER_TEST("UartPrint", test_uart_print_deinit_restores_port3);
}
