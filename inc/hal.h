// hal.h

#ifndef HAL_H
#define HAL_H

#include <hal_base.h>


#define FCCLK		SystemCoreClock

#define  now      (hal::ticks)

/*
#define TICK_US							 1000			  // tick period, us
#define CLCK_T							 1000				// ticks per second
#define CLCK_T_MS						   1				// ticks per millisecond

#define MS_TO_TICKS(ms)   ((uint32_t)(ms))
#define TICKS_TO_MS(tck)  (((uint32_t)(tck)))
*/

// #define TIMEOUT(timer, msec)    (hal::difftime((timer), now) >= MS_TO_TICKS(msec))

//*****************************************************************************

namespace hal
{
// Индикаторы LED0 .. LED3
static const uint8_t 
	kLed0 = 0,
	kLed1 = 1,
	kLed2 = 2,
	kLed3 = 3;
	
extern volatile uint32_t ticks;

// ticks difference    
/* inline uint32_t difftime(uint32_t ticks1, uint32_t ticks2)
{
	if(ticks2 >= ticks1) 
		return ticks2 - ticks1;
	else 
		return ticks2 + (0xFFFFFFFF - ticks1);
} */


// Устанавливает состояние сигнала RESET для ETH PHY
//
// st		true/false - Reset on/off
void setEthPhyReset(bool st);

// Установить состояние выхода управления индикатором
//
// LEDNumber		kLed0 .. kLed3
// st						true/false
void setLed(uint8_t LEDNumber, bool st);

void reset(void);
void delay(uint32_t ms);
void init(void);
void process(void);
	
};		// namespace hal

#endif		// HAL_H
