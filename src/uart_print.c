/*
 * Copyright (c) 2026 p-ban.com Corp.
 * SPDX-License-Identifier: LicenseRef-p-ban-solist-ai-gcc-template
 * See LICENSE (LICENSE.ja is the authoritative Japanese text).
 */

/*****************************************************************************
 uart_print.c

 UART Debug Interface Implementation for ML63Q2537
 - Uses UARTF0 peripheral
 - Non-blocking operation
 - Supports printf-style formatting

******************************************************************************/

#include "uart_print.h"
#include "mcu.h"
#include "uartf0.h"
#include "rdwr_reg.h"
#include "irq.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/*############################################################################*/
/*#                                 Defines                                  #*/
/*############################################################################*/

/* Buffer for printf formatting */
#define PRINTF_BUFFER_SIZE      256

/* UART configuration parameters - matching sample code */
/* For 115200 bps at SYSCLK 48MHz */
#define UARTF_PARAM_MODE        (UARTF_LG_8BIT | UARTF_STP_1BIT | UARTF_PT_NON | \
                                 UARTF_BC_DIS | UARTF_DLAB_RBR_THR | UARTF_FEN_ENA | \
                                 UARTF_RFR_KEEP | UARTF_TFR_KEEP | UARTF_FTL_2BYTE)
#define UARTF_PARAM_CAJ         (UARTF_RMV_ENA | 0x0019U)
#define UARTF_PARAM_DLR         (0x0019U)  /* baudrate : 115200bps at SYSCLK48MHz */

/* #define UARTF_PARAM_CAJ         (UARTF_RMV_ENA | 0x000DU) */
/* #define UARTF_PARAM_DLR         (0x000AU)  /\* baudrate : 115200bps at SYSCLK20MHz *\/ */

/* P3MOD0 holds one 6-bit mode field per pin on an 8-bit stride: P30 at bit 0,
 * P31 at bit 8, P32 at bit 16, P33 at bit 24. This is the layout ROHM's own
 * smpl_initLED1..3() use for P5MOD0 in utility/board/smpl_common_led.c. */
#define P3MOD_FIELD_MASK        (0x3FU)
#define P3MOD_SHIFT(port3_pin)  ((uint32_t)(port3_pin) * 8U)

/* Mode codes written into that field. 0x0A is what ROHM calls "general-purpose
 * mode"; it is the same value for every pin. */
#define P3MOD_MODE_GPIO         (0x0AU)
#define P3MOD_MODE_UARTF0_RX    (0x21U)
#define P3MOD_MODE_UARTF0_TX    (0x23U)

/* Pins are numbered port * 10 + index, so P33 is index 3 of PORT3. */
#define PORT3_PIN(pin)          ((uint32_t)(pin) - 30U)

/* Bits of UAF0MOD that hold their written value after uartf0_init(). The FIFO
 * reset bits UFnRFR/UFnTFR are excluded because uartf0_init() pulses them, and
 * the reserved bits are excluded because their read value is not specified. */
#define UARTF_MODE_VERIFY_MASK  (uint16_t)(UAFnMOD_UFnLG1  | UAFnMOD_UFnLG0 | \
                                           UAFnMOD_UFnSTP  | UAFnMOD_UFnPT2 | \
                                           UAFnMOD_UFnPT1  | UAFnMOD_UFnPT0 | \
                                           UAFnMOD_UFnBC   | UAFnMOD_UFnDLAB | \
                                           UAFnMOD_UFnFEN  | UAFnMOD_UFnFTL1 | \
                                           UAFnMOD_UFnFTL0)

/*############################################################################*/
/*#                                Variables                                 #*/
/*############################################################################*/

static bool s_uart_initialized = false;
static char s_printf_buffer[PRINTF_BUFFER_SIZE];

/*############################################################################*/
/*#                            Private Functions                             #*/
/*############################################################################*/

/**
 * Write one pin's mode field in P3MOD0, leaving the other pins alone.
 *
 * Both the UART bring-up and the teardown go through here, so the field
 * positions are stated once instead of being open-coded at each site.
 *
 * @param[in]   port3_pin   Pin index within PORT3 (0-3)
 * @param[in]   mode        Mode code for that pin
 * @return      -
 */
static void s_set_port3_pin_mode(uint32_t port3_pin, uint32_t mode)
{
  write_bit(PORT3->P3MOD0,
            P3MOD_FIELD_MASK << P3MOD_SHIFT(port3_pin),
            mode << P3MOD_SHIFT(port3_pin));
}

/**
 * Hand P32/P33 to UARTF0
 */
static void s_configure_gpio(void)
{
  s_set_port3_pin_mode(PORT3_PIN(UART_PRINT_RX_PIN), P3MOD_MODE_UARTF0_RX);
  s_set_port3_pin_mode(PORT3_PIN(UART_PRINT_TX_PIN), P3MOD_MODE_UARTF0_TX);

  /* Drive TX high: the idle level for a UART line. */
  set_bit(PORT3->P3DO, 1U << PORT3_PIN(UART_PRINT_TX_PIN));
}

/**
 * Read back the mode register and compare it with what was configured.
 *
 * A UARTF0 whose peripheral clock is not running accepts writes but does not
 * retain them, so the transmitter would stay silent while every uart_print_*
 * call reported success. Comparing the readback turns that into an init
 * failure the caller can act on.
 */
static bool s_config_applied(void)
{
  uint16_t mode = (uint16_t)read_reg32(UARTF0->UAF0MOD);

  return (mode & UARTF_MODE_VERIFY_MASK) ==
         ((uint16_t)UARTF_PARAM_MODE & UARTF_MODE_VERIFY_MASK);
}

/**
 * Simple itoa for hex conversion
 */
static void s_hex_to_str(uint8_t value, char *str)
{
  const char hex_chars[] = "0123456789ABCDEF";
  str[0] = hex_chars[(value >> 4) & 0x0F];
  str[1] = hex_chars[value & 0x0F];
  str[2] = '\0';
}

/*############################################################################*/
/*#                              Public Functions                            #*/
/*############################################################################*/

/**
 * Initialize debug UART
 */
bool uart_print_init(void)
{
  if (s_uart_initialized) {
    return true;
  }

  /* Configure GPIO pins */
  s_configure_gpio();

  /* Initialize UARTF0 */
  uartf0_init((uint16_t)UARTF_PARAM_MODE,(uint16_t)UARTF_PARAM_CAJ, (uint16_t)UARTF_PARAM_DLR);
  if (!s_config_applied()) {
    return false;
  }
  s_uart_initialized = true;
  /* enable interrupt */
  irq_uaf0_clearIRQ();
  irq_uaf0_ena();

  return true;
}

/**
 * Deinitialize debug UART
 */
void uart_print_deinit(void)
{
  if (!s_uart_initialized) {
    return;
  }

  /* Wait for any pending transmission */
  uart_print_flush();

  /* Give P32/P33 back to general-purpose mode. */
  s_set_port3_pin_mode(PORT3_PIN(UART_PRINT_RX_PIN), P3MOD_MODE_GPIO);
  s_set_port3_pin_mode(PORT3_PIN(UART_PRINT_TX_PIN), P3MOD_MODE_GPIO);

  s_uart_initialized = false;
}

/**
 * Send a single character
 */
void uart_print_putc(char c)
{
  if (!s_uart_initialized) {
    return;
  }

  /* Wait until transmit buffer is ready */
  while (uartf0_checkWriteBusy()) {
    /* Busy wait */
  }

  /* Send character */
  uartf0_putc((uint8_t)c);
}

/**
 * Send a string
 */
void uart_print_puts(const char *str)
{
  if (!s_uart_initialized || !str) {
    return;
  }

  while (*str) {
    uart_print_putc(*str++);
  }
}

/**
 * Send formatted string
 */
int uart_print_printf(const char *format, ...)
{
  if (!s_uart_initialized || !format) {
    return 0;
  }

  va_list args;
  int len;

  /* Format string into buffer */
  va_start(args, format);
  len = vsnprintf(s_printf_buffer, PRINTF_BUFFER_SIZE, format, args);
  va_end(args);

  /* Ensure null termination */
  if (len >= PRINTF_BUFFER_SIZE) {
    len = PRINTF_BUFFER_SIZE - 1;
  }
  s_printf_buffer[len] = '\0';

  /* Send formatted string */
  uart_print_puts(s_printf_buffer);

  return len;
}

/**
 * Send hex dump of data
 */
void uart_print_hex_dump(const uint8_t *data, uint32_t len, const char *prefix)
{
  if (!s_uart_initialized || !data || len == 0) {
    UART_PRINT_INFO("hex dump returns soon");
    return;
  }

  char hex_str[3];
  uint32_t i;

  if (prefix) {
    uart_print_puts(prefix);
    uart_print_puts(": ");
  }

  for (i = 0; i < len; i++) {
    if (i > 0 && (i % 16) == 0) {
      uart_print_puts("\r\n");
      if (prefix) {
        uart_print_puts(prefix);
        uart_print_puts(": ");
      }
    } else if (i > 0 && (i % 8) == 0) {
      uart_print_puts("  ");
    } else if (i > 0) {
      uart_print_putc(' ');
    }

    s_hex_to_str(data[i], hex_str);
    uart_print_puts(hex_str);
  }

  uart_print_puts("\r\n");
}

/**
 * Check if UART is ready
 */
bool uart_print_is_ready(void)
{
  if (!s_uart_initialized) {
    return false;
  }

  return !uartf0_checkWriteBusy();
}

/**
 * Flush UART transmit buffer
 */
void uart_print_flush(void)
{
  if (!s_uart_initialized) {
    return;
  }

  /* Wait until transmit complete */
  while (uartf0_checkWriteBusy()) {
    /* Busy wait */
  }

  /* Wait for UART transmit empty flag */
  while (!(uartf0_getStatus() & UAFnLSR_UFnTEMT)) {
    /* Busy wait */
  }
}
