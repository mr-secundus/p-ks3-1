// p_slavemsghandlers.h
//
// Обработчики прикладных сообщений
//
#include "protocol_app/p_tprotocolinst.h"

namespace P_SlaveMsgHandlers_HiIO
{
	void RxMessageHandler(uint8_t* msg, uint16_t destAddress, uint16_t srcAddress, TProtocolInstance* p);
};

namespace P_SlaveMsgHandlers_Ext
{
	void RxMessageHandler(uint8_t* msg, uint16_t destAddress, uint16_t srcAddress/*, TProtocolInstance* p*/);
};

namespace P_SlaveMsgHandlers_Local
{
	void RxMessageHandler(uint8_t* msg, uint16_t destAddress, uint16_t srcAddress/*, TProtocolInstance* p*/);
};
