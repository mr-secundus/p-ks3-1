// p_thandlers.h
// Объявление интерфейса к обработчикам, реализуемым ближе к прикладному уровню.

#ifndef __P_THANDLERS_H
#define __P_THANDLERS_H

#include <tbuffer.h>    

//
// Интерфейс к обработчикам ситуаций при разборе/формировании пакета
//
class TPacketProcessHandlers
{
	public:
		// начало обработки принятого пакета
		virtual void onBeginRxPacketProcess(void) {};
		
		// завершение обработки принятого пакета                                                   
		// src - адрес источника входящих сообщений
		virtual void onEndRxPacketProcess(uint16_t src) {};
																								
		// вызывается из addMessage при нехватке места в вых. буфере												
		// src - адрес источника входящих сообщений
		virtual void onTxBufferFull(uint16_t src) {};
};


//
// Интерфейс к обработчику входящих сообщений
//
class TRxMessageHandler
{
	public:                                                                                    
		virtual void processRxMessage(uint8_t* message, uint16_t destAddress, uint16_t srcAddress)=0;
};                


//
// Интерфейс передачи в канал связи
//
class TLayer1TxInterface
{
	public:                                   
		// num - номер канала связи
		
/*		virtual uint16_t	write(uint16_t num, uint8_t *buffer, uint16_t size, bool forceTx=false);
		
		// Возвращает кол-во свободных байт в выходном буфере
		virtual uint16_t	getTxFree(uint16_t num);

		// Сброс выходного буфера
		virtual void		resetTx(uint16_t num);*/
		
		// Передача данных из buffer в канал связи
		// size     - кол-во байт для передачи
		// forceTx  - немедленная передача 
		// Возвращает кол-во переданных байт
		uint16_t	write(uint16_t num, uint8_t *buffer, uint16_t size, bool forceTx=false);
		
		// Возвращает кол-во свободных байт в выходном буфере
		uint16_t	getTxFree(uint16_t num);

		// Сброс выходного буфера
		void		resetTx(uint16_t num);
		
};

#endif
