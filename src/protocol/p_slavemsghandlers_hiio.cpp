// p_slavemsghandlers_hiio.cpp
//
// Обработчики прикладных сообщений для связи с ВУ
 
#include "hal.h"
#include <supervisor/supervisor.h>
#include <dataio/dataio_app_types.h>
#include "tparameters.h"
#include "events.h"        
#include "debug.h"  
#include "cfg/defines.h"  
#include "protocol.h"
#include "protocol_app/p_tprotocolinst.h"


// Настройки обрабатываемых типов сообщений
#include "cfg/p_slavemsghandlers_hiio_config.h"


namespace P_SlaveMsgHandlers_HiIO
{   
#ifdef UseExternalAppTypeHandler
	void extMsgAppTypeHandler(DataIO::TMsgAppTypeHeader* msg, uint16_t destAddress, uint16_t srcAddress, TProtocolInstance* p);
#endif

struct 
{
	DataIO::TMsgParameterBlockHeader h;
	int32_t data[DataIO::MaxParametersBlockSize/sizeof(uint32_t)];
}msgParameterBlock;

#ifdef Handle_MsgPWrite
//
void handleMsgPWrite(DataIO::TMsgParameterWrite* msg, uint16_t destAddress, TProtocolInstance* pi)
{                                           
	// В общем случае msg может быть не выровнена на границу слова,
	// это необходимо учитывать при доступе к msg->parameterValue
//	int32_t v = msg->parameterValue;		!!!
	uint16_t *p = (uint16_t*)(&msg->parameterValue);
	int32_t v = p[1];
	v <<= 16;
	v |= p[0];

#ifdef DEBUG_PROTOCOL_MESSAGES
	puts("msgWr sq:"); putd(msg->sequenceNumber); puts(" da:"); putd(destAddress);
	puts(" p:"); putl(msg->parameterNumber);
	puts(" v:"); putl(v); 
//	puts(" v:"); putl(msg->parameterValue); 
	putc('\n');
#endif              
	int16_t result = Parameters.write(msg->parameterNumber, v);
	if(destAddress != PROTOCOL_BROADCAST_ADDRESS  &&  msg->sequenceNumber != 0xFF)
	{
		DataIO::TMsgParameterValue m;
		m.id = DataIO::MsgParameterValue;
		m.sequenceNumber	= msg->sequenceNumber;
		m.parameterNumber = msg->parameterNumber;
		m.parameterValue 	= v;

		if(result == 0)				m.result = DataIO::ResultOk;
		else if(result == -1) m.result = DataIO::ResultBadParameter;
		else									m.result = DataIO::ResultOperationError;

		pi->addMessage((uint8_t*)&m, 0);
	}
}
#endif

#ifdef Handle_MsgPRead
//
void handleMsgPRead(DataIO::TMsgParameterRead* msg, uint16_t destAddress, TProtocolInstance* p)
{                                           
#ifdef DEBUG_PROTOCOL_MESSAGES
	puts("msgRd sq:"); putd(msg->sequenceNumber); puts(" da:"); putd(destAddress); puts(" p:"); putl(msg->parameterNumber); putc('\n');
#endif
	int32_t v;
	int16_t result = Parameters.read(msg->parameterNumber, &v);
	DataIO::TMsgParameterValue m;
	m.id = DataIO::MsgParameterValue;
	m.sequenceNumber	= msg->sequenceNumber;
	m.parameterNumber = msg->parameterNumber;
	m.parameterValue	= v;

	if(result == 0)				m.result = DataIO::ResultOk;
	else if(result == -1) m.result = DataIO::ResultBadParameter;
	else									m.result = DataIO::ResultOperationError;

	p->addMessage((uint8_t*)&m, 0);
}     

//
void handleMsgPReadBlock(DataIO::TMsgReadBlock* msg, uint16_t destAddress, TProtocolInstance* p)
{                                           
#ifdef DEBUG_PROTOCOL_MESSAGES
	puts("msgRdBlock sq:"); putd(msg->sequenceNumber); puts(" da:"); putd(destAddress); putc('\n');
#endif                       
	
	uint16_t size = msg->size;
//	if(size > DataIO::MaxParametersBlockSize/sizeof(uint32_t))
//		size = DataIO::MaxParametersBlockSize/sizeof(uint32_t);
	if(size > p->getPayload()/sizeof(uint32_t))
		size = p->getPayload()/sizeof(uint32_t);
			
	msgParameterBlock.h.header.sequenceNumber	= msg->sequenceNumber;
	msgParameterBlock.h.setParameters(msg->base, size);
	
	for(uint16_t i=0; i<size; i++)
	{
/*		int32_t v;
		if(Parameters.read(msg->base + i, &v) != 0)
		{
			v = -1;
			msgParameterBlock.h.errCount++;
		}
		msgParameterBlock.data[i] = v; */
		
		Parameters.read(msg->base + i, &msgParameterBlock.data[i]);
	} 
	
	p->addMessage((uint8_t*)&msgParameterBlock, msgParameterBlock.h.getAppDataPtr());
}     
#endif

#ifdef Handle_MsgPValue
//
void handleMsgPValue(DataIO::TMsgParameterValue* msg, uint16_t destAddress, uint16_t srcAddress)
{                                                                                  
#ifdef DEBUG_PROTOCOL2_MESSAGES
	puts("msgVal sq:"); putd(msg->sequenceNumber);
	puts(" p:"); putl(msg->parameterNumber);
	puts(" v:"); putl(msg->parameterValue);
	putc('\n');
#endif
	
	deviceIO.rxParameterValue(srcAddress, msg->parameterNumber, msg->parameterValue);
	
//	AI16Reg.rxParameterValue(srcAddress, msg->parameterNumber, msg->parameterValue); */
}
#endif

#ifdef Handle_DateTime
//
void handleMsgRqDateTime(DataIO::TMsgAppTypeHeader* msg, uint16_t destAddress, TProtocolInstance* p)
{       
	TMsgAppDateTime m;
	TDateTime *dt = (TDateTime*)((uint8_t*)&m + sizeof(DataIO::TMsgAppTypeHeader));
//	getDateTime(dt);
	HAL::getRTC_DateTime(dt);
	m.header.sequenceNumber	= msg->sequenceNumber;
	p->addMessage((uint8_t*)&m, (uint8_t*)dt);
}

//
void handleMsgDateTime(TMsgAppDateTime* m, uint16_t destAddress)
{                              
#ifdef DEBUG_PROTOCOL_MESSAGES
	puts("msgDateTime\n");
#endif
	TDateTime *dt = (TDateTime*)((uint8_t*)m + sizeof(DataIO::TMsgAppTypeHeader));
//	setDateTime(dt);
	HAL::setRTC_DateTime(dt);
//	internalSetParameter(16, 1);		// установить R16.0 - информация о произведенной установке времени
}      
#endif		// Handle_DateTime

//***************************************************************************
//
void handleMsgAppType(DataIO::TMsgAppTypeHeader* msg, uint16_t destAddress, uint16_t srcAddress, TProtocolInstance* p)
{
	switch(msg->type)
	{     
		default:
#ifdef UseExternalAppTypeHandler
			extMsgAppTypeHandler(msg, destAddress, srcAddress, p);
#endif			
			break;
		
		//---- обрабатываются в данном файле ----------------
#ifdef Handle_DateTime
		case MsgApp_RequestDateTime:
			handleMsgRqDateTime(msg, srcAddress, p);
			break;

		case MsgApp_DateTime:
			handleMsgDateTime((TMsgAppDateTime*)msg, srcAddress);
			break;        
#endif
  }
}

// прием сообщений из полученного пакета
void RxMessageHandler(uint8_t* msg, uint16_t destAddress, uint16_t srcAddress, TProtocolInstance* p)
{          
  switch(*msg)
  {
  	default:
  		break;

#ifdef Handle_MsgPWrite
		case DataIO::MsgParameterWrite:
      handleMsgPWrite((DataIO::TMsgParameterWrite*)msg, destAddress, p);
      break;
#endif
		
#ifdef Handle_MsgPRead
		case DataIO::MsgParameterRead:
			handleMsgPRead((DataIO::TMsgParameterRead*)msg, destAddress, p);
			break;

		case DataIO::MsgReadBlock:
			handleMsgPReadBlock((DataIO::TMsgReadBlock*)msg, srcAddress, p);
			break;
#endif
			
#ifdef Handle_MsgPValue
			case DataIO::MsgParameterValue:
			handleMsgPValue((DataIO::TMsgParameterValue*)msg, destAddress, srcAddress);
			break;                   
#endif
			
		case DataIO::MsgAppType:
			handleMsgAppType((DataIO::TMsgAppTypeHeader*)msg, destAddress, srcAddress, p);
			break; 
  }
}    

};			// namespace
