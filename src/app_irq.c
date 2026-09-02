/*
 * Copyright (c) 2026 p-ban.com Corp.
 * SPDX-License-Identifier: LicenseRef-p-ban-solist-ai-gcc-template
 * See LICENSE (LICENSE.ja is the authoritative Japanese text).
 */

#include <stdio.h>

#include "uart_print.h"
#include "uartf_int.h"
#include "irq.h"
#include "ssiof0.h"

/*############################################################################*/
/*#                               Prototype                                  #*/
/*############################################################################*/
void EXI_IRQHandler( void );
void NMI_Handler( void );
void UAF0_IRQHandler( void );

void EXI_IRQHandler( void )
{
}

void NMI_Handler( void )
{
  /* No process */
}

void UAF0_IRQHandler( void )
{
  uart_procUartfInt();
}

void SIOF0_IRQHandler(void)
{
  // ssiof0_continue();
}

