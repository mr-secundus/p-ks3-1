/*
 * @brief NGX Xplorer 4330 board file
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

#ifndef __BOARD_H_
#define __BOARD_H_

#include "chip.h"
/* board_api.h is included at the bottom of this file after DEBUG setup */

#ifdef __cplusplus
extern "C" {
#endif
	
//-----------------------------------------------------------------------------	
//                                 Board type
//-----------------------------------------------------------------------------	

// Опытный образец платы ks3 - схема c0
#define BOARD_REVISION_C0			
	

//-----------------------------------------------------------------------------	
//                                Misc. settings
//-----------------------------------------------------------------------------	

/** Define DEBUG_ENABLE to enable IO via the DEBUGSTR, DEBUGOUT, and
		DEBUGIN macros. If not defined, DEBUG* functions will be optimized
	out of the code at build time.
 */
#define DEBUG_ENABLE


/* Build for RMII interface */
#define USE_RMII
#define BOARD_ENET_PHY_ADDR	0x00

	
//-----------------------------------------------------------------------------	
//                                  GPIO pins
//-----------------------------------------------------------------------------	

#ifdef BOARD_REVISION_C0
	// Опытный образец платы ks3
	
	// PHY reset
	#define PHY_RST_PORT				0x9
	#define PHY_RST_BIT			  		6
	#define PHY_RST_GPIO_PORT		  4
	#define PHY_RST_GPIO_BIT		 11

	// External WDT
	#define WDI_GPIO_PORT		  		3			// P7.6
	#define WDI_GPIO_BIT		 		 14
	
	// Выход управления драйвером RS485-1
	// На схеме подключен к PF.4 без GPIO. Напаяна перемычка на X1:3 - P7.0.
	#define U1_DIR_GPIO_PORT			3			// P7.0			
	#define U1_DIR_GPIO_BIT		  	13

	#define LED0_GPIO_PORT    		2			// P4.4
	#define LED0_GPIO_BIT     		4
	#define LED1_GPIO_PORT				2			// P4.5 
	#define LED1_GPIO_BIT     		5
	#define LED2_GPIO_PORT    		2			// P4.6
	#define LED2_GPIO_BIT     		6
	// На опытном образце LED3 подключен к P4.7, который не имеет GPIO.
	// Назначен на P3.1 (RD0) - выведен на X1::13.
	#define LED3_GPIO_PORT    		5			// P3.1
	#define LED3_GPIO_BIT     		8
#else 																			// Рабочий
	// PHY reset
	#define PHY_RST_PORT				0x1
	#define PHY_RST_BIT			  		3
	#define PHY_RST_GPIO_PORT		  0
	#define PHY_RST_GPIO_BIT		 10

	// External WDT
	#define WDI_GPIO_PORT		  		3			// P7.6
	#define WDI_GPIO_BIT		 		 14

	// Выход управления драйвером RS485-1
	#define U1_DIR_GPIO_PORT			3			// P7.5			
	#define U1_DIR_GPIO_BIT		  	8

	#define LED0_GPIO_PORT    		2			// P4.4
	#define LED0_GPIO_BIT     		4
	#define LED1_GPIO_PORT				2			// P4.5 
	#define LED1_GPIO_BIT     		5
	#define LED2_GPIO_PORT    		2			// P4.6
	#define LED2_GPIO_BIT     		6
	#define LED3_GPIO_PORT    		5			// P4.8
	#define LED3_GPIO_BIT     		12
#endif


#include "board_api.h"

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H_ */
