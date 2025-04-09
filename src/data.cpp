// data.cpp

#include "data.h"
#include "cfg\default_ip_parameters.h"


TSetup1		  setup1;
TSetup2			setup2;
TSetup3			setup3;

TServiceFlags  	serviceFlags;		  // флаги состояния для внутреннего использования



uint32_t	R14 = 0,
					R15 = 0,
					R19 = 0,								// управление отладочными режимами
					R99 = 0;								// состояние операции с сохраняемыми параметрами


//
// Инициализация конфигурации default значениями
//
void initSetup2DefaultValues(TSetup2 *setup2)
{
	setup2->IP_HostAddress 		= DEFAULT_IP;
	setup2->IP_GatewayAddress = DEFAULT_GW;
	setup2->IP_SubnetMask 		= DEFAULT_SMASK;
	setup2->localPort 				= DEFAULT_LOCAL_PORT;
	setup2->destPort 					= DEFAULT_DEST_PORT;
}
