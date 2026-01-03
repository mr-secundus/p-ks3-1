// ts_monitor.cpp

#include "taskdef.h"
#include "hal.h"
#include "debug.h"
#include "protocol.h"
#include "p_slave_data_io.h"
#include "indication.h"
#include "soft_timer.h"

//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------

// Таймеры контроля таймаута связи с ВУ и с устройствами на RS485
SoftTimer timerHiIo(5000), timerRs485Io(5000); 

//-----------------------------------------------------------------------------
void TTaskMonitor::setState(uint16_t newState)
{
	state = newState;
}

//
void TTaskMonitor::activate(void)
{
	TBaseTask::activate(); 
	
	timerHiIo.reset();
	timerRs485Io.reset();
		
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
			p_slave_data_io::start();
			break;

		case msgNetConnected:
//			p_slave_data_io::start();
			indication::setState(indication::kEth_Link, true);
puts(" NetConnect\n");
      break;

		case msgNetInitStart:
		case msgNetDisconnected:
//			p_slave_data_io::stop();
			indication::setState(indication::kEth_Link, false);
			indication::setState(indication::kEth_Io, false);
puts(" NetDisconnect\n");
      break;
      
		case msgNetIoHiRequest:
			indication::setState(indication::kEth_Io, true);
			timerHiIo.reset();
puts(" H0");
			break;

		case msgSlaveMsg:
			indication::setState(indication::kRs485_Io, true);
			timerRs485Io.reset();
puts(" R0");
			break;
	}
}

//                 
void TTaskMonitor::process(void)
{   
	const uint32_t kPeriod = 20;
	
	// Периодическая обработка автомата передачи запросов по локальному интерфейсу связи
	Protocol::MasterTxLocal.process();
	
	p_slave_data_io::process();
	
  if(TIMEOUT(timer, kPeriod))
  {
    timer = now;
    
    // Инкремент таймеров контроля сообщений от ВУ и устройств на RS485. 
    // По таймауту установить соответствующее состояние индикации.
    
    timerHiIo.increment(kPeriod);
    if(timerHiIo.active())
    {
    	puts(" H1");
			indication::setState(indication::kEth_Io, false);
    }

    timerRs485Io.increment(kPeriod);
    if(timerRs485Io.active())
    {
    	puts(" R1");
			indication::setState(indication::kRs485_Io, false);
    }
    
    indication::process();

    // применение нового сетевого адреса
/*		if(serviceFlags.cmdAcceptSAddress)
		{
			Protocol::configureHiIO(setup1.R3);
			serviceFlags.cmdAcceptSAddress = 0;
		} */
  }
}
