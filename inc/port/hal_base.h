// hal_base.h

#ifndef HAL_BASE_H
#define HAL_BASE_H

#include <port.h>

#define now   									(hal::ticks)   
//#define TIMEOUT(timer, msec)		(hal::difftime((timer), now) >= MS_TO_TICKS(msec))
#define TIMEOUT(timer, msec)		(hal::difftime((timer), now) >= (msec))
	
namespace hal
{                                             
	extern volatile uint32_t ticks;				// system ticks counter

	// ticks difference    
	inline uint32_t difftime(uint32_t ticks1, uint32_t ticks2)
	{
		if(ticks2 >= ticks1) return ticks2 - ticks1;
		else return ticks2 + (0xffffffff - ticks1);
	}
	
};

#endif
