/*
 * Copyright (c) 2026 p-ban.com Corp.
 * SPDX-License-Identifier: LicenseRef-p-ban-solist-ai-gcc-template
 * See LICENSE (LICENSE.ja is the authoritative Japanese text).
 */

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <stdint.h>
#include <stdbool.h>

/* Definitions */

#define USR_PERI                    (UAF0_PERI)


/* Functions */

uint16_t device_initialize(void);
uint16_t device_deinitialize(void);
uint32_t device_get_sysclk(void);
uint32_t device_get_tick_ms(void);
uint16_t device_delay_ms(uint16_t ms);
uint16_t device_delay_us(uint16_t us);
void device_irq_ssiof(bool enable);

#endif // __DEVICE_H__
