/*****************************************************************************
 codeoption_config.h

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
 * Derived from codeoption_config.h of the ML63Q2500 Reference Software
 * (SourceCode/samples/1kHzTimer/1kHzTimer/src/codeoption_config.h).
 *
 * Modifications by p-ban.com Corp. (株式会社ピーバンドットコム), 2026:
 *   - reformatted and replaced the descriptive comments
 *
 * The configured values are unchanged from the ROHM original: both the
 * WDTMD and WDTPWMD0 options are enabled.
 */
/**
 * @file     codeoption_config.h
 * @brief    Code option values written into the .codeoption flash sector
 *
 * Override these to change the hardware-level startup configuration.
 */

#ifndef CODEOPTION_CONFIG_H__
#define CODEOPTION_CONFIG_H__

#include "codeoption.h"

/* WDT enabled at power-on so device_initialize()'s wdt_init() sees a
 * consistent hardware state regardless of the flash erase pattern. */
#define CODEOPTION0_WDTMD                       (CODEOPTION0_WDTMD_ENABLED)
#define CODEOPTION0_WDTPWMD0                    (CODEOPTION0_WDTPWMD0_ENABLED)

#endif /* CODEOPTION_CONFIG_H__ */
