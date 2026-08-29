/*****************************************************************************
 main.c

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
 * Derived from smpl_procUartfInt() of the ML63Q2500 Reference Software
 * (SourceCode/samples/UARTF/UartFCommunicate/src/main.c). The ROHM notice
 * above is reproduced from that file.
 *
 * Modifications by p-ban.com Corp. (株式会社ピーバンドットコム), 2026:
 *   - renamed smpl_procUartfInt() to uart_procUartfInt()
 *   - moved into its own translation unit
 *
 * The function body is otherwise unchanged from the ROHM original.
 */
/**
 * @file     uartf_int.c
 * @brief    UARTF0 interrupt dispatch
 */

#include "uartf_int.h"

#include "uartf0.h"

/**
 * UARTF interrupt routine
 *
 * @param           -
 * @return          None
 */
void uart_procUartfInt( void )
{
	uint32_t intStat;

	intStat = uartf0_getIntCause() & UARTF_IRID_MASK;
	switch( intStat ) {
	case UARTF_IRID_READ_REQ:
	case UARTF_IRID_CHAR_TIMEOUT:
	case UARTF_IRID_DATA_ERR:
		uartf0_continueRead();
		break;
	case UARTF_IRID_WRITE_REQ:
	case UARTF_IRID_TRANS_COMP:
		uartf0_continueWrite( (uint16_t)intStat );
		break;
	default:
		break;
	}
}
