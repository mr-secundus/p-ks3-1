// version.h

#ifndef VERSION_H
#define VERSION_H

#include "board.h"
#include "debug.h"

#ifdef	NDEBUG
	#define V_FEATURE_D				0			
#else
	#define V_FEATURE_D				0x80	// DEBUG - отладочная версия
#endif

#ifdef BOARD_REVISION_C0					// Опытный образец платы			
	#define V_FEATURE_0       1
#else
	#define V_FEATURE_0       0
#endif

#define V_FEATURE_1       0
#define V_FEATURE_2       0


#define V_MAJOR				0
#define V_MINOR				0   
#define V_BUILD				1
#define V_FEATURE			((V_FEATURE_D) | (V_FEATURE_2) | (V_FEATURE_1) | (V_FEATURE_0))

#define VERSION_N		(((V_MAJOR) << 24) | ((V_MINOR) << 16) | ((V_BUILD) << 8) | (V_FEATURE))


#endif /* VERSION_H */
