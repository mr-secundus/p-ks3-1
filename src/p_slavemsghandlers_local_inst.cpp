// p_slavemsghandlers_local_inst.cpp
//
// Обработчики сообщений от устройств на локальной шине

#include "hal.h"
#include "debug.h" 
#include "cfg/defines.h"
#include "cfg/slave_io.h"
#include "protocol.h"
#include "p_slave_data_io.h"


namespace P_SlaveMsgHandlers_Local
{             
// Запоминаем номер последнего принятого пакета для исключения двойной отправки
// данных на ВУ. Ситуация возможна, если до ведомого устройства не дошло подтверждение.
//uint16_t packetNumber = 0xFFFF;
	
	
//
// Обработчики сообщений от ведомого устройства. 
// Вызываются из P_SlaveMsgHandlers_Local::RxMessageHandler
//
	
//	
void handleMsgPValue(DataIO::TMsgParameterValue* msg, uint16_t destAddress, uint16_t srcAddress)
{                                                                                  
#ifdef DEBUG_PROTOCOL_MESSAGES_LOC
	puts("msgVal sq:"); putd(msg->sequenceNumber);
	puts(" p:"); putl(msg->parameterNumber);
	puts(" v:"); putl(msg->parameterValue);
	putc('\n');
#endif

#ifdef DEBUG_ADC1_CONTROL
	puts("AI4R rx "); putl(msg->parameterNumber);
	putc(' '); putl(msg->parameterValue);
	if(msg->result != DataIO::ResultOk)	
		puts(" error");
	putc('\n');
#endif

//	if(msg->result == DataIO::ResultOk)	
////		adc1::AIRegs.rxParameterValue(msg->parameterNumber, msg->parameterValue);
//		adc1::rxParameterValue(msg->parameterNumber, msg->parameterValue);
}

//
void handleMsgAppType(DataIO::TMsgAppTypeHeader* msg, uint16_t destAddress, uint16_t srcAddress)
{          
	using namespace DataIO;

#ifdef DEBUG_PROTOCOL_MESSAGES_LOC
	puts("msgApp sq:"); putd(msg->sequenceNumber); puts(" t:"); putl(msg->type); putc('\n');
#endif
	                                                                      
	// Блок данных 
	if(msg->type == kDataId)
	{
		TDataBuffer* p = (TDataBuffer*)((uint8_t*)msg + sizeof(DataIO::TMsgAppTypeHeader));

#ifdef DEBUG_PROTOCOL_MESSAGES_LOC
		puts("MsgApp_Data:"); 	putw(msg->type);
		puts(" pn:"); 					putl(p->packet);
		puts(" ds:"); 					putl(p->size); 
		puts(" s:"); 						putl(p->sync); putc('\n');   
#endif

		// Контроль повторного прёма пакетов выполняется в handleSlaveMessageData(), т.к.
		// возможна работа с несколькими ведомыми устройствами
/*		if(packetNumber != p->packet)		// если не повторный пакет
		{ 
			// Занести блок в выходной буфер для ВУ и послать подтверждение
			if(handleSlaveMessageData(srcAddress, p, msg->type) > 0)
			{
				packetNumber = p->packet;
				Protocol::addTxMsgApp(srcAddress, MsgApp_DataAcknowledge, sizeof(p->packet), p->packet, 0, LINK_ID_LOCAL);
			}
		}
		else
			Protocol::addTxMsgApp(srcAddress, MsgApp_DataAcknowledge, sizeof(p->packet), p->packet, 0, LINK_ID_LOCAL); */
		
		if(p_slave_data_io::handleMessageData(srcAddress, p, msg->type) > 0)
			Protocol::addTxMsgApp(srcAddress, MsgApp_DataAcknowledge, sizeof(p->packet), p->packet, 0, LINK_ID_LOCAL);
	}
	// Отсутствие данных
	else if(msg->type == MsgApp_DataAbsence)	
	{   
			p_slave_data_io::handleMessageNoData(srcAddress);
	}                                                                 
}
};		// namespace P_SlaveMsgHandlers_Local

