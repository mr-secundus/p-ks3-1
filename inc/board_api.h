/*
 * @brief Common board API functions
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#ifndef __BOARD_API_H_
#define __BOARD_API_H_

#include "lpc_types.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Function prototype for a MS delay function. Board layers or example code may
 *        define this function as needed.
 */
typedef void (*p_msDelay_func_t)(uint32_t);

/* The DEBUG* functions are selected based on system configuration.
   Code that uses the DEBUG* functions will have their I/O routed to
   the UART, semihosting, or nowhere. */
#if defined(DEBUG_ENABLE)
#if defined(DEBUG_SEMIHOSTING)
#define DEBUGINIT()
#define DEBUGOUT(...) printf(__VA_ARGS__)
#define DEBUGSTR(str) printf(str)
#define DEBUGIN() (int) EOF

#else
//#define DEBUGINIT() Board_Debug_Init()
//#define DEBUGOUT(...) printf(__VA_ARGS__)
//#define DEBUGSTR(str) Board_UARTPutSTR(str)
//#define DEBUGIN() Board_UARTGetChar()
#define DEBUGINIT() 
#define DEBUGOUT(...) printf(__VA_ARGS__)
#define DEBUGSTR(str) puts(str)
#define DEBUGIN() 
#endif /* defined(DEBUG_SEMIHOSTING) */

#else
#define DEBUGINIT()
#define DEBUGOUT(...)
#define DEBUGSTR(str)
#define DEBUGIN() (int) EOF
#endif /* defined(DEBUG_ENABLE) */


#ifdef __cplusplus
}
#endif

#endif /* __BOARD_API_H_ */
