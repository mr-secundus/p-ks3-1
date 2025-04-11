// ts_emac_service.cpp
//
// ќбслуживание состо€ни€ EMAC
// - инициализаци€
// - контроль наличи€ подключени€
// ѕри отключении/подключении отправл€ет сообщени€ дл€ управлени€ прикладными
// процессами, использующими сеть.


#include <string.h>

#include "taskdef.h"
#include "hal.h"
#include "cfg/defines.h"
#include "debug.h"
#include "emac_app.h"
#include "cfg/default_ip_parameters.h"
#include "data.h"

#define TM_MAC_INIT_TIMEOUT		 8000			// таймаут ожидани€ инициализации MAC
#define TM_MAC_CONTROL_LINK		   20			// период контрол€ состо€ни€ линка
#define TM_MAC_TIMERS		   			 10			// период обработки таймеров сетевого стека


// —осто€ние задачи
enum
{
	kStIdle = 0,							// idle
	kStError,									// ошибка выполнени€ операции модул€ EMAC
	kStInitStart,							// запуск инициализации EMAC
	kStInitWait,							// ожидание выполнени€ иницализации
	kStInitWaitTimeout,				// таймаут выполнени€ инициализации
	kStInitComplete,					// иницализаци€ выполнена успешно
	kStControlLinkStatus,			// текущий контроль подключени€
	kStLinkOff,								// обнаружено отсутствие подключени€
	kStLinkWait								// ожидание подключени€
};


//*****************************************************************************
//
void TTaskMAC_Service::setState(uint16_t newState)
{
	state = newState;

#ifdef DEBUG_EMAC_SERVICE	
	static const char* s[] = 
	{
		"Idle",
		"Error",
		"InitStart",
		"InitWait",
		"InitWaitTimeout",
		"InitComplete",
		"ControlLinkStatus",
		"LinkFault",
		"LinkWait",
	};
		
	putdw(now);
	puts(" MACService:");
	
	if(state < sizeof(s)/sizeof(char*))
		puts(s[state]);
	else
		putd(state);
	
	putc('\n');
#endif
}


//
void TTaskMAC_Service::activate(void)
{
	TBaseTask::activate();        
	connected = false;
	timer = timer2 = timer3 = now;
	state = kStIdle;   
}                       


//
void TTaskMAC_Service::deactivate(void)
{
	state = kStIdle;         
	TBaseTask::deactivate();                      
}                       


//
void TTaskMAC_Service::handleEvent(TCMessage* msg)
{        
	switch(msg->msgNo)
	{
		default: break;    

		case msgSysInit:			
			setState(kStInitStart);
			break;
	}
}

//*****************************************************************************
//
void TTaskMAC_Service::process(void)
{              
	// 
	if(state == kStControlLinkStatus)
	{
		emac_app::process();

		if(TIMEOUT(timer3, TM_MAC_TIMERS))
		{
			emac_app::processTimers();
			timer3 = now;
		}
	}

	if(timer2 == now) return;

	timer2 = now;

	// ќбработка автомата контрол€ состо€ни€ подключени€.
	// ¬ыполн€етс€ только дл€ состо€ний, в которых выполн€етс€ контроль link status.
	switch(state)
	{
		case kStInitWait:
		case kStControlLinkStatus:
		case kStLinkWait:
			if(TIMEOUT(timer, TM_MAC_CONTROL_LINK))
			{
				emac_app::processLinkStatus();
				timer = now;
			}
			else
				return;
			break;

		default: break;
	}

	switch(state)
	{
		default: break;
			
		case kStIdle:
			break;

		case kStInitStart:						// запуск инициализации
			sendMessage(grpNet, msgNetInitStart);
			
			hal::setEthPhyReset(true);
			hal::delay(2);
			hal::setEthPhyReset(false);

			emac_app::setMacAddress(DEFAULT_MAC_HI, DEFAULT_MAC_LO | (setup1.serialNumber & 0xFFFF));
			emac_app::setIpAddress(setup2.IP_HostAddress, setup2.IP_GatewayAddress, setup2.IP_SubnetMask);
			emac_app::init();
			
			setState(kStInitWait);
			timer = now;
			break;

		case kStInitWait:							// ожидание выполнени€ иницализации
			if(emac_app::getLinkStatus() != 0)
				setState(kStInitComplete);
			else
				if(TIMEOUT(timer, TM_MAC_INIT_TIMEOUT))
					setState(kStInitWaitTimeout);
			break;

		case kStInitWaitTimeout:			// таймаут выполнени€ инициализации
			setState(kStInitStart);
			break;

		case kStInitComplete:					// иницализаци€ выполнена успешно
			sendMessage(grpNet, msgNetInitOk); 
			sendMessage(grpNet, msgNetConnected); 
			connected = true;
			setState(kStControlLinkStatus);
			timer = now;
			break;

		case kStControlLinkStatus:		// текущий контроль подключени€
			if(emac_app::getLinkStatus() == 0)
				setState(kStLinkOff);
			break;

		case kStLinkOff:							// обнаружено отсутствие подключени€
			sendMessage(grpNet, msgNetDisconnected); 
			connected = false;
			state = kStLinkWait;
			timer = now;
#ifdef DEBUG_EMAC_SERVICE	
			putdw(now); puts(": link off\n");
#endif						
			break;

		case kStLinkWait:							// ожидание подключени€
			if(emac_app::getLinkStatus() != 0)
			{
#ifdef DEBUG_EMAC_SERVICE	
				putdw(now); puts(": link on\n");
#endif						
				sendMessage(grpNet, msgNetConnected); 
				connected = true;
				setState(kStControlLinkStatus);
			}
			break;

		case kStError:
#ifdef DEBUG_EMAC_SERVICE	
			putdw(now); puts(": EMAC error\n");
#endif						
			sendMessage(grpNet, msgNetError); 
			setState(kStIdle);
			break;
	}
}
