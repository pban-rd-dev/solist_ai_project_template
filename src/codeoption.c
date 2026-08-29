/*****************************************************************************
 codeoption.c

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
    202.12.20 Ver 1.1.0

******************************************************************************/
/*
 * Derived from codeoption.c of the ML63Q2500 Reference Software
 * (SourceCode/samples/1kHzTimer/1kHzTimer/src/codeoption.c).
 *
 * Modifications by p-ban.com Corp. (株式会社ピーバンドットコム), 2026:
 *   - reformatted and replaced the descriptive comments
 *
 * The contents of codeop_area, including the word 4 bit expression, are
 * unchanged from the ROHM original.
 */
/**
 * @file     codeoption.c
 * @brief    Code option block placed at 0x1003FFC0
 *
 * The silicon reads this 64-byte block at power-on. Word 4 (offset 0x10)
 * holds the WDT-related bits; other words are kept in the erased
 * (0xFFFFFFFF) state. See codeoption.h for the bit layout and
 * codeoption_config.h for the configurable values.
 */

#include "codeoption_config.h"

const uint32_t codeop_area[CODEOPTION_AREA_SIZE] __attribute__((section(".codeoption"))) = {
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    ((0x1FFFFFFFUL << 3UL) | (CODEOPTION0_WDTPWMD0 << 2UL) | (0x1UL << 1UL) | (CODEOPTION0_WDTMD)),
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
};
