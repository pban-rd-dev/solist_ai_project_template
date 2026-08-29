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
 * Declares uart_procUartfInt(), which is derived from smpl_procUartfInt() of
 * the ML63Q2500 Reference Software
 * (SourceCode/samples/UARTF/UartFCommunicate/src/main.c). The ROHM notice
 * above is reproduced from that file; only the function name differs.
 * See uartf_int.c for the implementation.
 */
/**
 * @file     uartf_int.h
 * @brief    UARTF0 interrupt dispatch
 */

#ifndef UARTF_INT_H__
#define UARTF_INT_H__

void uart_procUartfInt( void );

#endif /* UARTF_INT_H__ */
