// ts_init.cpp

#include <stdio.h>

#include "taskdef.h"
#include "cfg/defines.h"
#include "debug.h"
#include "hal.h"
#include "data.h"
#include "protocol.h"
#include "data_buffers.h"
#include "nv_parameters.h"


// Инициализация конфигурации default значениями
void initSetup2DefaultValues(TSetup2 *setup2);

void loadNvData(void);
void printMemoryInfo(void);					// debug

extern "C"
{
	extern unsigned long
	__start_data_RamLoc32,
	__end_data_RamLoc32,
	__start_bss_RamLoc32,
	__end_bss_RamLoc32,
	__start_data_RamLoc40,		
	__end_data_RamLoc40,
	__start_bss_RamLoc40,		
	__end_bss_RamLoc40,
//		__start_data_RamAHB32,
//		__end_data_RamAHB32,
	_pvHeapStart,
	__user_heap_base,
	__end_of_heap,		
	_vStackTop;
}


//*****************************************************************************
//
void TTaskInit::setState(uint16_t st)
{                
	state = st; 
}

//
void TTaskInit::activate(void)
{
	TBaseTask::activate();        
	state = 0;  
	timer = now;

//	hal::setLed1(true);

#ifdef DEBUG_INIT  	
	putdw(now); puts(": tsInit start\n");
#endif 
}                       

//
void TTaskInit::deactivate(void)
{
	TBaseTask::deactivate();                      
#ifdef DEBUG_INIT  	
	putdw(now); puts(": tsInit end\n");
#endif
}                       

//
void TTaskInit::handleEvent(TCMessage* msg)
{                 
//	uint16_t address, pn;
//	int32_t pv;
	switch(msg->msgNo)
	{
		default: break;    

		//-----------------------------------------------------
		//  P1:   b31..b24    резерв
		//        b23..b16    адрес устройства
		//        b15..b0     номер параметра
		//  P2:   значение параметра
/*		case msgRxParameterValue:
			address = (msg->Param1 >> 16) & 0x00FF;
			pn = msg->Param1 & 0xFFFF;
			pv = msg->Param2;           
			
			{
				char s[128];
				sprintf(s, "%d tsInit state:%d %d %d\n", MS_TO_TICKS(now), state, pn, pv);
				puts(s);
			} 

			switch(state)
			{
				default : break;
			}
			break; */
	}
	return;
}

//
void TTaskInit::process(void)
{ 
	switch(state)
	{     
		default: break;              
			
		case 0:      
#ifdef DEBUG_INIT  	
//			printMemoryInfo();
#endif
			loadNvData();
			
			hal::initUart1DrvControl(setup2.baudRs485_1, LINK1_EXTRA_DELAY);
			
			Protocol::init();
			Protocol::configure(LINK_ID_HI_IO, setup2.netAddrHi, PROTOCOL_RX_TIMEOUT);
			Protocol::configure(LINK_ID_LOCAL, setup2.netAddrRs485_1, PROTOCOL_RX_TIMEOUT);
			timer = now;
			state = 2;
			break;                                                           

		case 2:
			data_buffers::init();
			hal::setLed(hal::kLed0, true);
			hal::setLed(hal::kLed1, false);
			hal::setLed(hal::kLed2, false);
			hal::setLed(hal::kLed3, false);
			timer = now;
			state = 21;
			break;    
			
		case 21:
			if(TIMEOUT(timer, 1000))
			{
				hal::setLed(hal::kLed0, false);
				hal::setLed(hal::kLed1, true);
				timer = now;
				state = 22;
			}
			break;

		case 22:
			if(TIMEOUT(timer, 1000))
			{
				hal::setLed(hal::kLed1, false);
				hal::setLed(hal::kLed2, true);
				hal::setLed(hal::kLed3, false);
				timer = now;
				state = 23;
			}
			break;

		case 23:
			if(TIMEOUT(timer, 1000))
			{
				hal::setLed(hal::kLed2, false);
				hal::setLed(hal::kLed3, true);
				timer = now;
				state = 24;
			}
			break;

		case 24:
			if(TIMEOUT(timer, 1000))
			{
				hal::setLed(hal::kLed3, false);
				state = 3;
			}
			break;
			
		case 3:
		 	supervisor.activateTask(task_MAC_Service);
		 	supervisor.activateTask(task_UDP_Service);
    	supervisor.activateTask(task_Monitor); 
      deactivate();       
			break;               
	}
}

//*****************************************************************************

#ifdef DEBUG_INIT  	
#define IP4_ADDR(ipaddr, a,b,c,d) \
        (ipaddr)->addr = ((u32_t)((d) & 0xff) << 24) | \
                         ((u32_t)((c) & 0xff) << 16) | \
                         ((u32_t)((b) & 0xff) << 8)  | \
                          (u32_t)((a) & 0xff)
//
void printIp(char *s, char const *msg, uint32_t ip)
{
	sprintf(s, "%s %ld.%ld.%ld.%ld\n", msg, ip & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);
}
#endif


//
// Инициализация ЭНП, загрузка сохраняемых блоков параметров.
// При ошибке загрузки устанавливаются соотв. биты в R99.
//
void loadNvData(void)
{
	initSetup2DefaultValues(&setup2);

	nv_parameters::init();

	for(uint16_t i=1; i<=NUM_NV_BLOCKS; i++)
	{
		nv_parameters::rc_t rc = nv_parameters::load(i, &R99);
		if(rc != nv_parameters::rcOk)
		{
			puts(" nv load "); putd(i); puts(" error rc : "); putd(rc); puts("\n");				
		}
  }
	
#ifdef DEBUG_INIT  	
	char s[64];
	sprintf(s, " s/n : %ld\n", setup1.serialNumber);
	puts(s);
	sprintf(s, " NetAddr : %d %d %d\n", setup2.netAddrHi, setup2.netAddrRs485_1, setup2.netAddrRs485_2);
	puts(s);
	sprintf(s, " Baud : %ld %ld %ld\n", setup2.baudRs232, setup2.baudRs485_1, setup2.baudRs485_2);
	puts(s);
	printIp(s, " IP host :", setup2.IP_HostAddress);
	puts(s);
	printIp(s, " IP gw   :", setup2.IP_GatewayAddress);
	puts(s);
	printIp(s, " IP mask :", setup2.IP_SubnetMask);
	puts(s);
	sprintf(s, " LocalPort : %d  DestPort : %d\n", setup2.localPort, setup2.destPort);
	puts(s);
#endif	
}                          


#ifdef DEBUG_INIT  	
void printMemoryInfo(void)
{
	static const int SZ = 11;  

	char const *names[SZ] = 
	{
		"start_data_RamLoc32 ",
		"  end_data_RamLoc32 ",
		" start_bss_RamLoc32 ",
		"   end_bss_RamLoc32 ",
		"start_data_RamLoc40 ",		
		"  end_data_RamLoc40 ",
		" start_bss_RamLoc40 ",	
		"   end_bss_RamLoc40 ",
		"pvHeapStart    ",
		"end_of_heap    ",
		"vStackTop      "
	};
	
	unsigned long v[SZ] = 
	{
		__start_data_RamLoc32,
		__end_data_RamLoc32,
		__start_bss_RamLoc32,
		__end_bss_RamLoc32,
		__start_data_RamLoc40,		
		__end_data_RamLoc40,
		__start_bss_RamLoc40,		
		__end_bss_RamLoc40,
		_pvHeapStart,
		__end_of_heap,		
		_vStackTop
	};

	for(int i = 0; i < SZ; i++)
	{
		puts(names[i]);
		putdwx(v[i]);
		puts("\n");
		hal::delay(20);
	}
}
#endif
