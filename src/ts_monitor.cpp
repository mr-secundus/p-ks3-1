// ts_monitor.cpp

#include "taskdef.h"
#include "hal.h"
#include "debug.h"
#include "protocol.h"
#include "p_slave_data_io.h"


// opcode:  0 - отработка индикации
//          1 - выполнить синхронизацию
extern void processIndication(uint16_t opcode);

void TTaskMonitor::setState(uint16_t newState)
{
	state = newState;
}

//
void TTaskMonitor::activate(void)
{
	TBaseTask::activate(); 
	
	sendMessage(grpSystem, msgSysInit); 
}                       

//

void TTaskMonitor::deactivate(void)
{                     
	TBaseTask::deactivate();                      
}                       

//
void TTaskMonitor::handleEvent(TCMessage* msg)
{                      
	switch(msg->msgNo)
	{
		default: break;
		
		case msgSysInit:
			p_slave_data_io::init();
			break;

		case msgNetConnected:
			p_slave_data_io::start();
      break;

		case msgNetDisconnected:
			p_slave_data_io::stop();
      break;
	}
}

//                 
void TTaskMonitor::process(void)
{   
	// Периодическая обработка автомата передачи запросов по локальному интерфейсу связи
	Protocol::MasterTxLocal.process();
	
	p_slave_data_io::process();
	
  if(TIMEOUT(timer, 5))
  {
    timer = now;
//    processIndication(0);

    // применение нового сетевого адреса
/*		if(serviceFlags.cmdAcceptSAddress)
		{
			Protocol::configureHiIO(setup1.R3);
			serviceFlags.cmdAcceptSAddress = 0;
		} */
  }
}
