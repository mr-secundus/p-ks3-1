// p_tprotocol.h

#ifndef __P_TPROTOCOL_H
#define __P_TPROTOCOL_H

#include <tbuffer.h>    
//#include <dataio3.h>
#include <dataio/dataio3.h>
#include "p_thandlers.h"

//
//
//
class TProtocol
{       
	public:     
		static const uint16_t	MaxPacketSize 		= sizeof(DataIO::LR2_PACKET);
		static const uint16_t	MaxPacketPayload 	= DataIO::MaxLayer2DataSize;
		
	protected:                                                 
  	TPacketProcessHandlers	*packetHandlers;		// обработчтки событий разбора пакета
	
		TRxMessageHandler				*rxMessageHandler;	// обработчик входящих сообщений
		
		DataIO::DataIOLayer2 		*layer2;
		DataIO::DataIOLayer3 		*layer3;                                  
		
		uint32_t		rxTimeout,					// значение таймаута приема данных в тиках
							rxTicks;						// содержит значение счетчика тиков CPU на момент
																	// предыдущего прихода данных от интерфейса связи;
																	// используется для обнаружения таймаута приема данных
		uint16_t 		broadcastAddress,		// широковещательный адрес
							srcAddress;					// адрес источника входящего сообщения - последний зафиксированный
		uint8_t 		*rxDataBuffer;			// указатель на принятый пакет в буфере парсера
	
	public:    
		
		TProtocol(DataIO::DataIOLayer2 *l2, DataIO::DataIOLayer3 *l3, TPacketProcessHandlers *ph, TRxMessageHandler *rxh) :
			packetHandlers(ph),
			rxMessageHandler(rxh),
			layer2(l2),
			layer3(l3),
			rxTicks(0),
			srcAddress(0)
			{
			};

		TProtocol() :
			packetHandlers(0),
			rxMessageHandler(0),
			layer2(0),
			layer3(0),
			rxTicks(0),
			srcAddress(0)
			{
			};

		void install(DataIO::DataIOLayer2 *l2, DataIO::DataIOLayer3 *l3, TPacketProcessHandlers *ph, TRxMessageHandler *rxh)
		{
			packetHandlers		= ph;
			rxMessageHandler	= rxh;
			layer2						= l2;
			layer3						= l3;
		}

		void configure(uint16_t aselfAddress, uint16_t abroadcastAddress, uint16_t arxTimeout);

		// обработка входящих данных от интерфейса связи с контролем таймаута
		// buffer	- буфер, содержащий входные данные
		// n			- количество байт для чтения из буфера  
		// preambleRecieved  - преамбула пакета принята внешним обработчиком
		// Возврат:
		//	0  - идет прием
		//  1  - резерв
		//  2  - был принят пакет
		//  3  - принят заголовок пакета DataIOLt
		int16_t processInputData(TBuffer* buffer, uint16_t n, bool preambleRecieved);
		
		//
		// Обработка принятого пакета прикладного уровня - без заголовка пакета.
		//
		// buffer					буфер, содержащий входные данные
		// n							количество байт для чтения из буфера        
		// destAddress		адрес получателя
		// srcAddress			адрес отправителя                                   
		//
		// Возврат:  количество разобранных из buffer прикладных сообщений
		//
		int16_t processInputAppData(uint8_t* buffer, uint16_t n, uint16_t dest, uint16_t src);
		
			
		// Начать формирование выходного пакета.
		// Сбрасывает автомат заполнения выходного пакета в layer3 и фиксирует адрес 
		// буфера пакета. При неизменном адресе buffer может вызываться только один раз,
		// затем сброс состояния автомата выполняется в finishTxPacket.
		void startTxPacket(uint8_t* buffer)
		{
			layer3->startTxPacket((DataIO::LR2_PACKET*)buffer);
		}

		// Добавить сообщение в пакет для передачи.
		// При нехватке места в выходном буфере вызывает onTxBufferFull, после этого
		// пытается добавить сообщение еще раз.
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
		int16_t addMessage(uint8_t* msgPtr, uint8_t* data);
		
		//
		// Возвращает текущий размер выходного пакета
		//
		uint16_t getTxDataSize(void)
		{ 
			return layer3->getTxDataSize();
		}

		// Завершение формирования пакета.
		// Заносит адрес получателя и вычисляет CRC. Сбрасывает автомат заполнения пакета.
		// Возвращает размер пакета.
		uint16_t finishTxPacket(uint16_t destAddress)
		{
			return layer3->finishTxPacket(destAddress);
		}
		
		// Используется srcAddress из ранее пинятого пакета
		uint16_t finishTxPacket(void)
		{
			return layer3->finishTxPacket(srcAddress);
		}

		// Сброс счетчиков ошибок Layer2
		void resetErrors(void)
		{
			layer2->clearErrors();
		}
		
		// Возвращает счетчик ошибок - см. DataIO::TErrors
		uint32_t getErrors(DataIO::TErrors code)
		{    
			return layer2->getErrors((DataIO::TErrors)code);
		}   
};		// class TProtocol

#endif
