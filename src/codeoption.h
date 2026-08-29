/*****************************************************************************
 codeoption.h

 Copyright (C) 2024 ROHM Co., Ltd.
 All rights reserved.

 This software is provided "as is" and any expressed or implied
 warranties, including, but not limited to, the implied warranties of
 merchantability and fitness for a particular purpose are disclaimed.
 ROHM shall not be liable for any direct, indirect, consequential or
 incidental damages arising from using or modifying this software.
 You (customer) can modify and use this software in whole or part on
 your own responsibility, only for the purpose of developing the software
 for use with microcontroller manufactured by ROHM.

 History
    2024.07.31 Ver 1.0.0

******************************************************************************/
/*
 * Derived from codeoption.h of the ML63Q2500 Reference Software
 * (SourceCode/samples/1kHzTimer/1kHzTimer/src/codeoption.h).
 *
 * Modifications by p-ban.com Corp. (株式会社ピーバンドットコム), 2026:
 *   - include <stdint.h> instead of "mcu.h" so the header builds
 *     standalone under arm-none-eabi-gcc
 *   - reformatted and expanded the descriptive comments
 *
 * The code option macro names, their values, and the bit layout they
 * describe are unchanged from the ROHM original.
 */
/**
 * @file     codeoption.h
 * @brief    Code option area definition for ML63Q25x7
 *
 * The ML63Q25x7 reserves the top 64 bytes of flash (0x1003FFC0–0x1003FFFF)
 * for a hardware-level configuration block read by silicon at power-on.
 * It is analogous to AVR fuses, STM32 option bytes, or NXP FOPT.
 */

#ifndef CODEOPTION_H__
#define CODEOPTION_H__

#include <stdint.h>

#define CODEOPTION_AREA_SIZE                    (16U)

/* CODE OPTION 0 — bit 0: WDTMD, bit 2: WDTPWMD0 */
#define CODEOPTION0_WDTMD_DISABLED              (0UL)
#define CODEOPTION0_WDTMD_ENABLED               (1UL)

#define CODEOPTION0_WDTPWMD0_DISABLED           (0UL)
#define CODEOPTION0_WDTPWMD0_ENABLED            (1UL)

extern const uint32_t codeop_area[CODEOPTION_AREA_SIZE];

#endif /* CODEOPTION_H__ */
