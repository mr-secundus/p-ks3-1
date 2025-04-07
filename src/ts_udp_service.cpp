// ts_udp_service2.cpp

#include <string.h>

#include "taskdef.h"
#include "cfg/defines.h"
#include "hal.h"
#include "debug.h"
//#include "types.h"
#include "protocol/protocol.h"
#include "cfg/default_ip_parameters.h"
#include "udp_io/udp_io_if.h"


// упаковка октетов в uint32_t
#if !defined(X4TOU32)
#define X4TOU32(a3, a2, a1, a0)		(((a3) << 24) | ((a2) << 16) | ((a1) << 8) | (a0))
#endif


namespace ts_udp_service
{
// Состояние задачи
enum
{
	kStIdle=0,
	kStInit,
	kStInitOk,
	kStProcess,
	kStProcessRx,
	kStProcessTx
};


// Состояние пеодключения и модуля UDP
struct
{
	bool	UDP_initOk,				// выполнена иниициализация UDP
				connected;				// сетевое подключение 
} netState;


//*****************************************************************************
//
// Обработка входящих данных UDP. 
// Вызывается из обработчика очереди входящих пакетов UDP.
// Сигнатура должна соответствовать UDP_IO::T_v_TUDPDataPtr
// 
void processRxData(UdpData* data)
{            
#ifdef DEBUG_UDP_RX_APP
	putlog(" Rx "); putd(data->size); putc('\n'); 
#endif
	Protocol::processInputAppData(data->appData(), data->size, 1, 2);
}
                                           
//                    
// Передача выходных данных по UDP
// data		указатель на буфер
// n			количество байт для передачи
//
int16_t processTxData(uint8_t* data, uint16_t n)
{                               
	if(netState.connected)
	{
		int16_t rc = udp_io::addTxData(data, n);
#ifdef DEBUG_UDP_TX_APP
		putlog(" #Tx "); putd(n); putc(' '); putd(rc); putc('\n'); 
#endif
		return rc;
	}
	else
		return 0;
}

};		// namespace


//*****************************************************************************
//
void TTaskUDP_Service::setState(uint16_t newState)
{
	state = newState;

#ifdef DEBUG_UDP_SERVICE
	static const char* s[] = 
	{
		"Idle",
		"Init",
		"InitOk",
		"Process",
		"ProcessRx",
		"ProcessTx"
	};
		
	putdw(now);
	puts(" UDPService:");
	if(state < sizeof(s)/sizeof(char*)) puts(s[state]);
	else																putw(state);
	putc('\n');
#endif
}


//
void TTaskUDP_Service::activate(void)
{
	using namespace ts_udp_service;

	TBaseTask::activate();        

	netState.UDP_initOk = false;
	netState.connected = false;
	state = kStIdle;         

	Protocol::setAppTxHandler(processTxData); 
	Protocol::hi_io::setHandler_getTxFree(udp_io::getTxFree); 
}                       

//
void TTaskUDP_Service::deactivate(void)
{
	using namespace ts_udp_service;

	state = kStIdle;
	Protocol::setAppTxHandler(0);
	TBaseTask::deactivate();                      
}                       

//
void TTaskUDP_Service::handleEvent(TCMessage* msg)
{        
	using namespace ts_udp_service;

	switch(msg->msgNo)
	{
		default: break;    

		case msgNetInitStart:
			setState(kStIdle);
      break;

		case msgNetInitOk:
			setState(kStInit);
      break;

		case msgNetConnected:
			netState.connected = true;

			if(netState.UDP_initOk)
				setState(kStProcess);
			else if(state != kStInit)
				setState(kStInit);
      break;

		case msgNetDisconnected:
			netState.connected = false;
			setState(kStIdle);
      break;

		case msgNetError:
			setState(kStIdle);
      break;
	}
}

//*****************************************************************************
//
void TTaskUDP_Service::process(void)
{      
	using namespace ts_udp_service;

	switch(state)
	{
		default: break;
			
		case kStInit:                           
//			if(UDP_IO::init(DEFAULT_MAC_HI, DEFAULT_MAC_LO | (DEFAULT_SERIAL_NUMBER & 0xffff), 
//					setup1.IP_HostAddress, setup1.IP_GatewayAddress, setup1.IP_SubnetMask, setup1.localPort, setup1.destPort) == 0)
//					DEFAULT_IP, DEFAULT_GW, DEFAULT_SMASK, DEFAULT_LOCAL_PORT, DEFAULT_DEST_PORT) == 0)
			if(udp_io::init(DEFAULT_LOCAL_PORT, DEFAULT_DEST_PORT) == udp_io::kRcOk)
			{
				netState.UDP_initOk = true;
				state = kStInitOk;
				sendMessage(grpNet, msgUDP_InitOk); 
			}
			else 
			{
				netState.UDP_initOk = false;
				state = kStIdle;
				sendMessage(grpNet, msgUDP_InitError); 
			}
			break;
			
		case kStInitOk:
			if(netState.connected)
				setState(kStProcess);
			else
				setState(kStIdle);
			break;
			
		case kStProcess:
			udp_io::process();     
			state = kStProcessRx;
			break;                           

		case kStProcessRx:
			udp_io::process();     
			udp_io::processRxQueue(processRxData, 1);
			state = kStProcessTx;
			break;

		case kStProcessTx:   
			udp_io::process();     
			udp_io::processTxQueue(1);
			state = kStProcess;
			break;
	}
}

