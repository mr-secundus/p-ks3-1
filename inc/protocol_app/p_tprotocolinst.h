// p_tprotocolinst.h                        
//
// 28.06.22 iss53c-p_cp_test1
// В TPacketHandlersInst изменена видимость членов private -> protected для
// использования в производных классах.
// В TProtocolInstance добавлены protected методы для доступа к данным из
// производных классов.


#ifndef __P_TPROTOCOLINST_H
#define __P_TPROTOCOLINST_H

#include "p_tprotocol.h"

class TProtocolInstance;

//
// Реализация обработчиков ситуаций при разборе/отправке пакетов в TProtocol
//
class TPacketHandlersInst : public TPacketProcessHandlers
{            
	protected:
		uint8_t 								*txBuffer;
		DataIO::DataIOLayer3 	*layer3;
		TProtocol							*protocol;

	private:
		TProtocolInstance			*protocolInst;

	public:
		TPacketHandlersInst(uint8_t *ptxBuffer, DataIO::DataIOLayer3 *player3,
			                  TProtocol *pprotocol, TProtocolInstance *pprotocolInst) :
			txBuffer(ptxBuffer), layer3(player3), protocol(pprotocol), protocolInst(pprotocolInst) {} 

		TPacketHandlersInst() : txBuffer(0), layer3(0), protocol(0), protocolInst(0) {} 
			
		void install(uint8_t *ptxBuffer, DataIO::DataIOLayer3 *player3,
			           TProtocol *pprotocol, TProtocolInstance *pprotocolInst) 
		{                              
			txBuffer			= ptxBuffer;
			layer3 				= player3;
			protocol 			= pprotocol;
			protocolInst	= pprotocolInst;
		}
		
		virtual void onBeginRxPacketProcess(void);
		virtual void onEndRxPacketProcess(uint16_t src);
		virtual void onTxBufferFull(uint16_t src);
};                       


//
// Привязка TProtocol к прикладному уровню.
// В TProtocolInstance передаются интерфейсные объекты:
//   - TPacketHandlersInst	- обработчики разбора пакетов;
//   - TRxMessageHandler		- прикладной обработчик принятых сообщений;
//   - TLayer1TxInterface		- интерфейс передачи в канал связи.
//
class TProtocolInstance
{    
	private:
		DataIO::DataIOLayer2 	layer2;
		DataIO::DataIOLayer3 	layer3;
		TPacketHandlersInst		*packetHandlersInst;        
		TProtocol							protocol;                           
		
		TRxMessageHandler			*rxMsgHandler;
		
		TLayer1TxInterface		*iLayer1Tx;
		
		uint16_t layer1Number;		// номер канала связи, используемого с данным обработчиком
		
		uint8_t *txBuffer;

		uint16_t payload;
 
	public:               
		TProtocolInstance(TPacketHandlersInst *apacketHandlersInst, uint16_t alayer1Number,
											uint8_t* atxBuffer, uint16_t apayload=TProtocol::MaxPacketPayload) : 
			packetHandlersInst(apacketHandlersInst),
			layer1Number(alayer1Number),
			txBuffer(atxBuffer),
			payload(apayload)
			{
				layer3.setTxAppSize(payload);
			}   
			
	protected:
		inline DataIO::DataIOLayer3* 	getLayer3(void) 	{ return &layer3; }
		
	public:               
		inline TProtocol*		getProtocol(void) 	{ return &protocol; }
		inline uint8_t* 			getTxBuffer(void) 	{ return txBuffer; }     
		
		// Возвращает размер данных в выходном пакете
		inline uint16_t 			getTxDataSize(void) { return layer3.getTxDataSize(); }
		
		// Возвращает макс. допустимый размер прикладных данных в выходном пакете
		inline uint16_t 			getPayload(void) 		{ return payload; }		
		         
		//
		void install(TRxMessageHandler *prxMsgHandler, TLayer1TxInterface *piLayer1Tx)
		{
			rxMsgHandler	= prxMsgHandler;
			iLayer1Tx			= piLayer1Tx;                                        

			layer3.install(&layer2);
			packetHandlersInst->install(txBuffer, &layer3, &protocol, this);
			protocol.install(&layer2, &layer3, packetHandlersInst, rxMsgHandler);
		}
		
		// Установка параметров протокола
		void configure(uint16_t aselfAddress, uint16_t abroadcastAddress, uint16_t arxTimeout);    
		
		// Установка макс. размера прикладных данных в выходном пакете.
		// Значение зависит от используемого канала связи и типа протокола.
		// При обмене по последовательным каналам связи используются пакеты DataIO - 
		//   size = 254 - sizeof(LR2_HEADER)
		// При обмене по UDP заголовок DataIO не используется - 
		//   size = UDP_DATA_SIZE.
		void setPayload(uint16_t size)
		{
			layer3.setTxAppSize(size);
		}    
		
		
		// обработка входящих данных от интерфейса связи с контролем таймаута
		// buffer	- буфер, содержащий входные данные
		// n			- количество байт для чтения из буфера  
		int16_t processInputData(TBuffer* buffer, uint16_t n);     
		
		// Начать формирование выходного пакета.
		void startTxPacket(void)
		{
			protocol.startTxPacket(txBuffer);
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
		int16_t addMessage(uint8_t* msgPtr, uint8_t* data);   
		
		// Передача исходящего пакета из layer3 в канал связи.
		// destAddr  - адрес получателя, =-1 - используется адрес отправителя из принятого 
		//             ранее сообщения                        
		// Возвращает кол-во переданных байт.
		uint16_t sendPacket(int16_t destAddr=-1);
		
		// Сброс счетчиков ошибок
		void resetErrors(void) { layer2.clearErrors(); }

		// Возвращает счетчик ошибок - см. DataIO::TErrors
		uint32_t getErrors(DataIO::TErrors code) { return layer2.getErrors((DataIO::TErrors)code); }
};

#endif

