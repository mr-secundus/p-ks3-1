// data.cpp

#include "data.h"


TSetup1		  setup1;
TSetup2			setup2;
TSetup3			setup3;

TServiceFlags  	serviceFlags;		  // флаги состояния для внутреннего использования



uint32_t	R14 = 0,
					R15 = 0,
					R19 = 0,								// управление отладочными режимами
					R99 = 0;								// состояние операции с сохраняемыми параметрами
