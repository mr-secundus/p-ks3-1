// p_protocol.cpp
//                    
// Унифицированный обработчик входящих пакетов. 
// Реализует интерфейс между парсером нижнего уровня протокола и
// обработчиком прикладных сообщений. 
// Отправка ответных сообщений реализуется в прикладном модуле через 
// обратные вызовы.      
//
//-----------------------------------------------------------------------------
//
// 24.06.22  Перенсено из iss4-s2 при реализации обмена по UDP
// - добавлено processInputAppData - обработка принятого пакета прикладного уровня
//   без заголовка.
//
// 19.09.2015  ai16-s1
// - добавлен вызов startRx после завершения обработки пакета - ранее при приеме
//   более одного пакета без временнОго промежутка перед началом пакета не начиналась
//   обработка нового пакета.

//#define DEBUG_RX_PACKET

#ifdef DEBUG_RX_PACKET
extern "C" {
#include <putx.h>
}            

namespace HAL
{
void wait_conout(void)
{
}
};                    
#endif

 
#include <hal_base.h>
#include "protocol_app/p_tprotocol.h"

//
void TProtocol::configure(uint16_t aselfAddress, uint16_t abroadcastAddress, uint16_t arxTimeout)
{
	broadcastAddress = abroadcastAddress;
//	rxTimeout = MS_TO_TICKS(arxTimeout);
	rxTimeout = arxTimeout;
	layer2->configure(aselfAddress, abroadcastAddress);
}

// обработка входящих данных от интерфейса связи с контролем таймаута
// buffer	- буфер, содержащий входные данные
// n			- количество байт для чтения из буфера  
// preambleRecieved  - преамбула пакета принята внешним обработчиком
// Возврат:
//	0  - идет прием
//  1  - резерв
//  2  - был принят пакет
//  3  - принят заголовок пакета DataIOLt
int16_t TProtocol::processInputData(TBuffer* buffer, uint16_t n, bool preambleRecieved)
{             
	int16_t rc=0;
		                     
	if(hal::difftime(rxTicks, now) >= rxTimeout)		// таймаут приема
	{
		layer2->startRx();
#ifdef DEBUG_RX_PACKET
		putc('['); putdw(rxTimeout-rxTicks); puts("]\n");
#endif
	}
	rxTicks = now;                                                 
	
	if(preambleRecieved) layer2->setPreambleRecieved();

#ifndef DEBUG_RX_PACKET
	while(n--)
	{                
		//	0  - ожидание начала пакета
		//  1  - прием пакета
		//  2  - принят пакет
		//  3  - принят заголовок пакета DataIOLt  - если определено USE_DATAIOLT
		int16_t r = layer2->rxHandler(buffer->read_wo_check());
		if(r == 2) 
		{
			uint16_t dest, src;
			layer3->readPacket(&dest, &src);   
			
			srcAddress = src;
	
			packetHandlers->onBeginRxPacketProcess();
			
			uint8_t msgId;
			while(layer3->getMessagePtr(&msgId, &rxDataBuffer) == 0)
				rxMessageHandler->processRxMessage(rxDataBuffer, dest, src);
			
			packetHandlers->onEndRxPacketProcess(src);
				
			layer2->startRx();
	
			rc = 2;
		}         
		else if(r == 3) return 3;
	}                          
	return rc;
#else
	while(n--)
	{                
		//	0  - ожидание начала пакета
		//  1  - прием пакета
		//  2  - принят пакет
		//  3  - принят заголовок пакета DataIOLt  - если определено USE_DATAIOLT
//		int16_t r = layer2->rxHandler(buffer->read_wo_check());

		uint8_t _b = buffer->read_wo_check();
		int16_t r = layer2->rxHandler(_b);
		
putc(':'); putbx(_b); putc(' '); putd(r); putc(' '); 		

		if(r == 2)
		{
			puts(">>P:"); putd(layer2->getRxDataSize()); putc('\n');
//			HAL::wait_conout();
		}                                                     
		else if(r < 0  ||  r > 1000)
		{
			puts(">>RC:"); putd(r); putc('\n');
			HAL::wait_conout();
		}

		if(r == 2) 
		{
			uint16_t dest, src;
			layer3->readPacket(&dest, &src);   
			
			srcAddress = src;
	
			packetHandlers->onBeginRxPacketProcess();
			
//			puts("ri:"); putd(layer3->getRxIndex()); puts(" rds:"); putd(layer3->rxDataSize()); putc('\n');
			HAL::wait_conout();

			uint8_t msgId;
			while(layer3->getMessagePtr(&msgId, &rxDataBuffer) == 0)
			{
//				puts("rxi:"); putd(layer3->getRxIndex()); puts(" rds:"); putd(layer3->rxDataSize());
				puts(" m:"); putd(msgId); puts("..");
				HAL::wait_conout();
				rxMessageHandler->processRxMessage(rxDataBuffer, dest, src);
				HAL::wait_conout();
				puts("****\n");
				HAL::wait_conout();
			}
			
			puts("txsz:"); putd(layer3->getTxDataSize()); putc(' ');
 			HAL::wait_conout();

			packetHandlers->onEndRxPacketProcess(src);

			putd(layer3->getTxDataSize()); putc('\n');
			HAL::wait_conout();
				
			layer2->startRx();
	
			rc = 2;
		}         
		else if(r == 3) return 3;
	}                          
	if(rc != 0) { puts("rc:"); putd(rc); putc('\n'); HAL::wait_conout(); }
	return rc;
#endif
}     


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
int16_t TProtocol::processInputAppData(uint8_t* buffer, uint16_t n, uint16_t dest, uint16_t src)
{             
	srcAddress = src;		// запомнить адрес отправителя для формирования ответного пакета
	
	packetHandlers->onBeginRxPacketProcess();
		
	layer3->setRxAppData(buffer, n);
			
	uint8_t msgId;
	int16_t msgCounter = 0;
	while(layer3->getMessagePtr(&msgId, &rxDataBuffer) == 0)
	{
		rxMessageHandler->processRxMessage(rxDataBuffer, dest, src);
		msgCounter++;
	}
			
	packetHandlers->onEndRxPacketProcess(src);
				
//	layer2->startRx();			// ??  processInputAppData используется отдельно от layer2

	return msgCounter;
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
int16_t TProtocol::addMessage(uint8_t* msgPtr, uint8_t* data)
{                               
	int16_t rc = layer3->addMessage(msgPtr, data);
	
	// код, специфичный для ведомого устройства
	if(rc == -1)			// недостаточно места в вых. буфере
	{
		packetHandlers->onTxBufferFull(srcAddress);
		rc = layer3->addMessage(msgPtr, data);
	}
	return rc;
} 


