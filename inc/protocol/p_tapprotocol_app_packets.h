// p_tapprotocol_app_packets.h
//
// Расширение TAppProtocol для работы с пакетами прикладного уровня - без
// заголовка пакета, для работы по UDP.

// Связь с ВУ по протоколу DataIO.
// Привязанная к приложению реализация интерфейса между компонентами:
//   - нижний уровень протокола: layer2 layer3;
//   - обработчик пакетов protocol;
//   - прикладной обработчик сообщений RxMessagesHandler;
//   - реализация передачи исходящих пакетов в канал связи.
//
// 24.06.22
// Из iss4-s2 перенесены дополнения для работы с пакетами прикладного уровня.
// Добавлен TProtocolInstance_AppPackets - поддерживает дополнительные интерфейсы
// приема и передачи пакетов. Обработчики устанавливаются из модуля работы с UDP.


#ifndef __T_APPPROTOCOL_APP_PACKETS_H
#define __T_APPPROTOCOL_APP_PACKETS_H

#include "p_tapprotocol.h"


// callback для передачи пакетов прикладного уровня
// arg0		буфер с выходными данными
// arg1		число байт данных в буфере                                           
// Возвращает число обработанных байт либо код ошибки <0
typedef int16_t (*T_I16_U8ptr_U16)(uint8_t*, uint16_t);

class TProtocolInstance_AppPackets;


//-----------------------------------------------------------------------------
//
// Реализация обработчиков ситуаций при разборе/отправке пакетов в TProtocol
//
// Введение обусловлено необходимостью обращения к TProtocolInstance_AppPackets::sendPacket().
// TPacketHandlersInst обращается к TProtocolInstance::sendPacket().
//
// Содержит отдельный конструктор для задания внешнего txBuffer, т.к. по умолчанию 
// при вызове install() передается буфер, используемый в TProtocolInstance - размер
// соответствует макс. размеру пакета DataIO. А при использовании UDP макс. размер
// пакета - 1536 байт.
//
class TPacketHandlersInst_AppPackets : public TPacketHandlersInst
{            
	private:
		TProtocolInstance_AppPackets	*protocolInst;

	public:
		TPacketHandlersInst_AppPackets(uint8_t *ptxBuffer, DataIO::DataIOLayer3 *player3,
			                  TProtocol *pprotocol, TProtocolInstance_AppPackets *pprotocolInst) :
			TPacketHandlersInst(ptxBuffer, player3, pprotocol, (TProtocolInstance*)pprotocolInst),
			protocolInst(pprotocolInst) 
			{} 

		TPacketHandlersInst_AppPackets() : 
			TPacketHandlersInst(0, 0, 0, 0),
			protocolInst(0)
			{} 

		void install(uint8_t *ptxBuffer, DataIO::DataIOLayer3 *player3,
			           TProtocol *pprotocol, TProtocolInstance_AppPackets *pprotocolInst) 
		{                    
			TPacketHandlersInst::install(ptxBuffer, player3, pprotocol, (TProtocolInstance*)pprotocolInst);
			protocolInst	= pprotocolInst;
		}                                             
		
		inline uint8_t* getTxBuffer(void) { return txBuffer; }     
		
		
		virtual void onBeginRxPacketProcess(void);
		virtual void onEndRxPacketProcess(uint16_t src);
		virtual void onTxBufferFull(uint16_t src);
};                       



//-----------------------------------------------------------------------------
//                                                                             
// TProtocolInstance с поддержкой работы с пакетами сообщений без заголовка - обмен по UDP.
// Добавлено определение типа входящяго пакета и дополнительные внешние обработчики для
// передачи пакетов без заголовка.
// При приеме определяется тип входящего пакета, при отправке выходного пакета соответственно
// выбирается интерфейс передачи.
//
class TProtocolInstance_AppPackets : public TProtocolInstance
{                   
	private:		     
		TPacketHandlersInst_AppPackets	*packetHandlersInst;        
		
		uint16_t linkID;     										// id типа входящего сообщения - полный пакет с заголовком,
																					// или только блок данных с сообщениями.
																					// Используется для диспетчеризации ответного пакета в канал связи.
																					
		T_I16_U8ptr_U16 	processTxAppData;
																				

	public:
		// идентификация каналов связи для специфичной обработки и перенаправления выходных данных
		static const uint16_t	LINK_ID_0	= 0;			// UART/RF
		static const uint16_t	LINK_ID_1	= 1;			// пакеты прикладного уровня без заголовка - обмен по UDP
	
		TProtocolInstance_AppPackets(TPacketHandlersInst_AppPackets *apacketHandlersInst, uint16_t alayer1Number,
											uint8_t* atxBuffer, uint16_t apayload) :
			TProtocolInstance(apacketHandlersInst, alayer1Number, atxBuffer, apayload),
			packetHandlersInst(apacketHandlersInst),
			linkID(LINK_ID_0),
			processTxAppData(0)
			{}                              
			
		//
		void install(TRxMessageHandler *prxMsgHandler, TLayer1TxInterface *piLayer1Tx)
		{                                         
			TProtocolInstance::install(prxMsgHandler, piLayer1Tx);
			packetHandlersInst->install(getTxBuffer(), getLayer3(), getProtocol(), this);
		}
			
			
		// Установка обработчика передачи пакетов прикладного уровня - без заголовка пакета
		void setAppTxHandler(T_I16_U8ptr_U16 h)
		{
			processTxAppData = h;         
		}
			
			                     
		// Установка ID канала связи в соответствии с типом входящего пакета		                     
		void setLinkID(uint16_t id)
		{
			linkID = id;
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
		int16_t processInputData(TBuffer* buffer, uint16_t n)
		{
			// Передача ответного пакета происходит в процессе работы processInputData(),
			// поэтому linkID необходимо устанавливать до вызова
			linkID = LINK_ID_0;		// диспетчеризации ответного сообщения
			int16_t r = TProtocolInstance::processInputData(buffer, n);
			return r;
		}             
		
		// Обработка принятого пакета прикладного уровня - без заголовка пакета.
		//
		// buffer					буфер, содержащий входные данные
		// n							количество байт для чтения из буфера        
		// destAddress		адрес получателя
		// srcAddress			адрес отправителя                                   
		//
		// Возврат:  количество разобранных из buffer прикладных сообщений
		int16_t processInputAppData(uint8_t* buffer, uint16_t n, uint16_t dest, uint16_t src)
		{
			// Передача ответного пакета происходит в процессе работы processInputData(),
			// поэтому linkID необходимо устанавливать до вызова
			linkID = LINK_ID_1;		// диспетчеризации ответного сообщения
			int16_t r = getProtocol()->processInputAppData(buffer, n, dest, src);
			return r;
		}               
		
		
		// Передача исходящего пакета из layer3 в канал связи
		// destAddr  - адрес получателя, =-1 - используется адрес отправителя из принятого 
		//             ранее сообщения
		// Возвращает кол-во переданных байт.
		uint16_t sendPacket(int16_t dest)
		{                       
			if(linkID == LINK_ID_0)
				return TProtocolInstance::sendPacket(dest);
			else   
			{
				if(getTxDataSize()  &&  processTxAppData)
				{                                 
					
	//				int16_t rc2 = processTxAppData(((DataIO::LR2_PACKET*)getTxBuffer())->Packet.data, getTxDataSize());
					uint8_t *b = ((DataIO::LR2_PACKET*)getTxBuffer())->Packet.data;
					uint16_t sz = getTxDataSize();
					int16_t rc2 = processTxAppData(b, sz);
					
					startTxPacket();							// сброс выходного буфера для исключения повторной 
																				// передачи данных в onEndRxPacketProcess
					return rc2 > 0 ? rc2 : 0;
				}
			}
	    return 0;
		}
};


//-----------------------------------------------------------------------------
//
// TAppProtocol с поддержкой прикладных пакетов.
// Расширенные операции выполняются через указатель на объект с поддержкой
// AppPackets.
//
class TAppProtocol_AppPackets : public TAppProtocol
{                                                                                                          
	TProtocolInstance_AppPackets		*protocol_app;	
	
	public:
		
	TAppProtocol_AppPackets(TLayer1TxInterface	*p1, TRxMessageHandler *p2, TProtocolInstance_AppPackets *p3) :
		TAppProtocol(p1, p2, p3),
		protocol_app(p3)
		{
		}              

	// Установка обработчика передачи пакетов прикладного уровня - без заголовка пакета
	void setAppTxHandler(T_I16_U8ptr_U16 h)
	{
		protocol_app->setAppTxHandler(h);         
	}
		
	
	// Обработка входящих данных от интерфейса связи с контролем таймаута
	// buffer	- буфер, содержащий входные данные
	// n			- количество байт для чтения из буфера  
	int16_t processInputData(TBuffer* buffer, uint16_t n)
	{
		return protocol_app->processInputData(buffer, n);
	}
	

	// Обработка принятого пакета прикладного уровня - без заголовка пакета
	//
	// buffer					буфер, содержащий входные данные
	// n							количество байт для чтения из буфера        
	// destAddress		адрес получателя
	// srcAddress			адрес отправителя                                   
	//
	// Возврат:  количество разобранных из buffer прикладных сообщений
	int16_t processInputAppData(uint8_t* buffer, uint16_t n, uint16_t dest, uint16_t src)
	{
		return protocol_app->processInputAppData(buffer, n, dest, src);
	}               

	
	// Передача исходящего пакета в канал связи
	int16_t sendPacket(int16_t src=-1)
	{
		return protocol_app->sendPacket(src);
	}
};

#endif

