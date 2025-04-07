// p_tapprotocol.h                        
//
// Связь с ВУ по протоколу DataIO.
// Привязанная к приложению реализация интерфейса между компонентами:
//   - нижний уровень протокола: layer2 layer3;
//   - обработчик пакетов protocol;
//   - прикладной обработчик сообщений RxMessagesHandler;
//   - реализация передачи исходящих пакетов в канал связи.


#ifndef __T_APPPROTOCOL_H
#define __T_APPPROTOCOL_H

#include <tbuffer.h>
#include "protocol_app/p_tprotocolinst.h"
#include "p_slavemsghandlers.h"


class TAppProtocol
{              
	TLayer1TxInterface	*layer1TxInterface;     // интерфейс передачи в канал связи
	TRxMessageHandler		*rxMsgHandler;					// обработчик принятых сообщений
	TProtocolInstance		*protocol;							// 

	public:
		
	TAppProtocol(TLayer1TxInterface	*p1, TRxMessageHandler *p2, TProtocolInstance *p3) :
		layer1TxInterface(p1),
		rxMsgHandler(p2),
		protocol(p3)
		{
			protocol->install(rxMsgHandler, layer1TxInterface);
		}
		
	// Конфигурирование
	// selfAddress				- адрес устройства
	// broadcastAddress		- широковещательный адрес
	// rxTimeout					- таймаут приема пакета, мс
	// payload						- макс. размер блока прикладных данных в пакете
	void configure(uint16_t selfAddress, uint16_t broadcastAddress, uint16_t rxTimeout)
	{  
		// трансляция rxTimeout в тики
		protocol->configure(selfAddress, broadcastAddress, rxTimeout);
	}    
	
	
	// обработка входящих данных от интерфейса связи с контролем таймаута
	// buffer	- буфер, содержащий входные данные
	// n			- количество байт для чтения из буфера  
	int16_t processInputData(TBuffer* buffer, uint16_t n)
	{
		return protocol->processInputData(buffer, n);
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
	int16_t addMessage(uint8_t* msgPtr, uint8_t* data)
	{
		return protocol->addMessage(msgPtr, data);
	}
	
	
	// передача исходящего пакета в канал связи
	int16_t sendPacket(int16_t src=-1)
	{
		return protocol->sendPacket(src);
	}
	
	
	// Сброс счетчиков ошибок
	void resetErrors(void)
	{
		protocol->resetErrors();
	}
	
	// Возвращает счетчик ошибок - см. DataIO::TErrors
	uint32_t getErrors(int16_t code)
	{    
		return protocol->getErrors((DataIO::TErrors)code);
	}
	
};

#endif

