// p_slavemsghandlers_local.cpp
//
// Обработчики прикладных сообщений от ведомых устройств на внутренней шине.
 
#include "port.h"
#include "dataio/dataio_types.h"
#include "dataio/dataio_app_types.h"

namespace P_SlaveMsgHandlers_Local
{                                              
void handleMsgPValue(DataIO::TMsgParameterValue* msg, uint16_t destAddress, uint16_t srcAddress);
void handleMsgAppType(DataIO::TMsgAppTypeHeader* msg, uint16_t destAddress, uint16_t srcAddress);

//
// прием сообщений из полученного пакета
//
void RxMessageHandler(uint8_t* msg, uint16_t destAddress, uint16_t srcAddress)
{          
  switch(*msg)
  {
  	default:
  		break;

		case DataIO::MsgParameterValue:
			handleMsgPValue((DataIO::TMsgParameterValue*)msg, destAddress, srcAddress);
			break;
			
		case DataIO::MsgAppType:
			handleMsgAppType((DataIO::TMsgAppTypeHeader*)msg, destAddress, srcAddress);
			break; 
  }
}    
};	// namespace P_SlaveMsgHandlers_Local
