// port.h

#ifndef PORT_H
#define PORT_H

#include "chip.h"

//-----------------------------------------------------------------------------
//                           Compiler specific
//-----------------------------------------------------------------------------

#define enable()	__enable_irq();
#define disable()	__disable_irq();


#endif		// PORT_H


