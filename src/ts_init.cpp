// ts_init.cpp

#include <stdio.h>

#include "taskdef.h"
#include "cfg/defines.h"
#include "debug.h"
#include "hal.h"
//#include "types.h"
#include "protocol.h"
#include "data_buffers.h"
//#include "tparameters.h"
//#include "uip_app\udp_io.h"


// Инициализация полей конфигурации setup1 default значениями
//void initSetup1DefaultValues(TDeviceSetup1 *setup1);

void loadFromNVMemory(void);
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
			
//			initParameters();
//			loadFromNVMemory();      
				
			// if(!NV_BLOCK_RD_ERROR(2))
			// 	HAL::setUART_Baud(LINK_ID_HI_IO, setup2.baud);
				
			// Protocol::init(setup1.R3, NET_ADDRESS_LOCAL, NET_ADDRESS_EXT);
			Protocol::init(DEFAULT_NET_ADDRESS, 1, 1);
			timer = now;
			state = 2;
			break;                                                           

		case 2:
			data_buffers::init();
			state = 3;
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
//
// Инициализация ЭНП, загрузка сохраняемых блоков параметров.
// При ошибках выполнения операций устанавливаются соотв. биты в R50 R99.
//
void loadFromNVMemory(void)
{
/*	initSetup1DefaultValues(&setup1);

	// загрузка блоков параметров из ЭНП
	for(uint16_t i=1; i<=NUM_NV_BLOCKS; i++)
	{
		uint32_t t=now;
		while(HAL::difftime(t, now) < MS_TO_TICKS(1));
		int16_t rc = loadNVBlock(i);
#ifdef DEBUG_INIT  	
		puts("#loadNVBlock("); putd(i); puts(") :"); putd(rc); putc('\n');
#endif
  } */
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
