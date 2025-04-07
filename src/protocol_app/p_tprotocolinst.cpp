// p_tprotocolinst.cpp
//

#include "protocol_app/p_tprotocolinst.h"


//*****************************************************************************
//                           TPacketHandlersInst
//*****************************************************************************

// начало обработки принятого пакета
void TPacketHandlersInst::onBeginRxPacketProcess(void)		
{
	// при завершении приема пакета удалить имеющиеся данные в выходных буферах - при
	// использующейся концепции обмена это непереданный ответ на предыдущие запросы,
	// после прихода очредного пакета теряет актуальность
	protocol->startTxPacket(txBuffer);
}
	
// завершение обработки принятого пакета;
// arg1: адрес источника входящих сообщений
void TPacketHandlersInst::onEndRxPacketProcess(uint16_t src)
{
	// безусловная отправка пакета при наличии данных
	if(layer3->getTxDataSize()) protocolInst->sendPacket();
}

// вызывается из addMessage при нехватке места в вых. буфере
// arg1: адрес источника входящих сообщений
void TPacketHandlersInst::onTxBufferFull(uint16_t src)
{
	if(layer3->getTxDataSize()) protocolInst->sendPacket();
}


//*****************************************************************************
//                             TProtocolInstance
//*****************************************************************************

// Передача исходящего пакета из layer3 в канал связи
// destAddr  - адрес получателя, =-1 - используется адрес отправителя из принятого 
//             ранее сообщения
// Возвращает кол-во переданных байт.
uint16_t TProtocolInstance::sendPacket(int16_t dest)
{                    
	uint16_t n;
	if(dest == -1) n = protocol.finishTxPacket();		// ответ на адрес источника ранее принятого сообщения
	else					 n = protocol.finishTxPacket(dest);
		
	if(n  &&  iLayer1Tx->getTxFree(layer1Number) > n)
		return iLayer1Tx->write(layer1Number, txBuffer, n, true);
	else
		return 0;
}

//
void TProtocolInstance::configure(uint16_t aselfAddress, uint16_t abroadcastAddress, uint16_t arxTimeout)
{
	protocol.configure(aselfAddress, abroadcastAddress, arxTimeout);
}    

// Обработка входящих данных от интерфейса связи с контролем таймаута
//
// buffer	- буфер, содержащий входные данные
// n			- количество байт для чтения из буфера  
// preambleRecieved  - преамбула пакета принята внешним обработчиком
// Возврат:
//	0  - идет прием
//  1  - резерв
//  2  - был принят пакет
//  3  - принят заголовок пакета DataIOLt
int16_t TProtocolInstance::processInputData(TBuffer* buffer, uint16_t n)
{
	return protocol.processInputData(buffer, n, false);
}                


// Добавить сообщение в пакет для передачи.
//
// msgPtr - указатель на структуру одного из типов:
//          MsgParameterWrite, MsgParameterRead, MsgParameterValue, MsgAppType
//          Для сообщений MsgParameter.. структура содержит всё сообщение,
//          для MsgUserType - заголовок сообщения
// data   - указатель на блок данных для UserType; может располагаться в памяти
//          независимо от заголовка сообщения
//
// возврат:  0 - сообщение добавлено в пакет
//          -1 - нет места в буфере
//          -2 - неизвестный тип сообщения
int16_t TProtocolInstance::addMessage(uint8_t* msgPtr, uint8_t* data)
{
	return protocol.addMessage(msgPtr, data);
}
