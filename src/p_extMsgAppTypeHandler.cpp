// p_extMsgAppTypeHandler.cpp
//
// Обработчик специфичных для проекта подтипов MsgAppType при обмене с ВУ
 
#include "hal.h"
#include "dataio/dataio_app_types.h"
#include "protocol.h"
#include "protocol_app/p_tprotocolinst.h"
#include "data_buffers.h"
#include "debug.h"  

namespace P_SlaveMsgHandlers_HiIO
{
//
// Формирует и добавляет в выходной пакет сообщение с состоянием устройства
//
void addMsgStateInfo(uint8_t seq)
{
/*
	TMsgAppStateInfo_5430 msg;
	int32_t v1, v2;

	msg.header.sequenceNumber = seq;
	msg.r17  = deviceState.getU32();
	msg.r150 = R150;

	// Читаем число потерянных отсчетов
	ADC1_Buffers::handleParameters_DataIO(ADC1_Buffers::kOpCodeRead, 12, &v1);
	ADC2_Buffers::handleParameters_DataIO(ADC1_Buffers::kOpCodeRead, 12, &v2);

	if(v1)		msg.flags |= TMsgAppStateInfo_5430::Mask_loss1;
	if(v2)		msg.flags |= TMsgAppStateInfo_5430::Mask_loss2; 

	// Флаг потери данных от AI4R
	if(ADC1::state.dataInputState & ADC1::TAI4R_State::Mask_loss)
		msg.flags |= TMsgAppStateInfo_5430::Mask_loss1;

	ADC1_Buffers::handleParameters_DataIO(ADC1_Buffers::kOpCodeRead, 10, &v1);
	ADC2_Buffers::handleParameters_DataIO(ADC1_Buffers::kOpCodeRead, 10, &v2);
	
	msg.dataBlocksNumber1 = v1;
	msg.dataBlocksNumber2 = v2;

	Protocol::addMessage((uint8_t*)&msg, (uint8_t*)&msg+sizeof(msg.header));
*/	
}


//
// Обработчик MsgAppType
// Обрабатывает:
//   - запросы на передачу блоков данных
//   - пеодтверждения приема блоков данных
//
void extMsgAppTypeHandler(DataIO::TMsgAppTypeHeader *msg, uint16_t destAddress, uint16_t srcAddress, TProtocolInstance *p)
{
	switch (msg->type)
	{
		default:
			break;
	
		case DataIO::MsgApp_RequestData:
			// при отсутствии данных послать MsgApp_DataAbsence
			if (data_buffers::getDataReadyInfo() == data_buffers::kNoData)
			{
				DataIO::TMsgAppTypeHeader m;
				m.id = DataIO::MsgAppType;
				m.sequenceNumber = msg->sequenceNumber;
				m.dataSize = 0;
				m.type = DataIO::MsgApp_DataAbsence;

				Protocol::addMessage((uint8_t *)&m, 0);
				Protocol::sendPacket(srcAddress);

#ifdef DEBUG_HIIO_APP_MSG
				putlog(" RqData  ->  DataAbsence\n");
#endif	
			}
			else
				{
					data_buffers::handleMsgRequestData(srcAddress, msg);
#ifdef DEBUG_HIIO_APP_MSG
//					putlog(" RqData  ->  Data\n");
#endif	
				}
			break;
	
		case DataIO::MsgApp_DataAcknowledge:
			data_buffers::handleMsgDataAcknowledge(srcAddress, (DataIO::TMsgAppDataAcknowledge *)msg);
#ifdef DEBUG_HIIO_APP_MSG
			if (data_buffers::getDataReadyInfo() == data_buffers::kNoData)
			{
				putlog(" DataAck(");
				putd(((DataIO::TMsgAppDataAcknowledge *)msg)->packetNumber);
				puts(") : buffer free\n");
			}
#endif	
		break;
	}
}

};			// namespace
