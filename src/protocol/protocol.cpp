// protocol.cpp                 
//
// Реализация обмена по каналам связи:
// - обмен с ВУ как slave устройство;
// - опрос локальных устройств по внутренней шине;
// - опрос устройств по внешней шине.
//            
// 1) Экземпляры объектов конечных классов протокола, реализующие отправку и прием сообщений. 
// 2) Объекты, реализующие отдельные функции и уровни протокола - связюваются с объектами 1).
// 3) Функции отправки запросов к ведомым устройствам.   
//
// TODO:
//	- реализация в TRxMessageHandlerInst установки локального адреса для обработки
//    сообщений от ВУ - сейчас используется DEFAULT_NET_ADDRESS;
//
//-----------------------------------------------------------------------------
// 17.11.22  iss54-3-control
// Реализовано динамическое изменение размера прикладных данных в выходном пакете
// при наличии определений MAX_TX_APP_SIZE_DATAIO и MAX_TX_APP_SIZE_DATAIO_APP - 
// см. вызовы setPayload() в йункциях обработки входных данных.
//
//
// 27.06.22  iss53c-p_cp_test1
//
// Из iss4-s2 перенесены дополнения для работы с пакетами прикладного уровня - без
// заголовка пакета, для работы по UDP:
//
// - В TProtocolInstance внутренний txBuffer заменен на передачу указателя на внешний
//   буфер для возможности использования буфера разных размеров.
//
// - Добавлен TProtocolInstance_AppPackets (p_tapprotocol_app_packets.h) - производный
//   класс от TProtocolInstance с поддержкой приема прикладных пакетов и установкой
//   интерфейса передачи выходных пакетов.
//   Обработчики устанавливаются из модуля работы с UDP.   
//   Соответственно в модуле protocol.cpp изменен тип объекта protocolInst_HiIO.
//
// - Добавлен метод TProtocol::processInputAppData() - разбор пакетов без заголовка.
//-----------------------------------------------------------------------------



#include <stdio.h>
//#include "hal.h"
#include "cfg/protocol.h"
#include "protocol/protocol.h"
#include "protocol/p_slavemsghandlers.h"

// Идентификаторы каналов связи LINK_ID_.. - см. в defines.h

//-----------------------------------------------------------------------------
//
// Обработчики событий обработки пакетов для прикладных пакетов при работе по UDP
//                                                                               
// начало обработки принятого пакета
void TPacketHandlersInst_AppPackets::onBeginRxPacketProcess(void)		
{
	// при завершении приема пакета удалить имеющиеся данные в выходных буферах - при
	// использующейся концепции обмена это непереданный ответ на предыдущие запросы,
	// после прихода очредного пакета теряет актуальность
	protocol->startTxPacket(txBuffer);
}

void TPacketHandlersInst_AppPackets::onEndRxPacketProcess(uint16_t src)
{
	if(layer3->getTxDataSize()) protocolInst->sendPacket(-1);
}

void TPacketHandlersInst_AppPackets::onTxBufferFull(uint16_t src)
{
	if(layer3->getTxDataSize()) protocolInst->sendPacket(-1);
}
//-----------------------------------------------------------------------------

namespace Protocol
{
extern TMasterTx	MasterTxLocal;							// опрос локальных узлов

//
// Специфический обработчик для интерфейса Local - при приеме пакета 
// от slave делается уведомление автомата опроса для завершения 
// ожидания ответа.
//
class TPacketHandlersInstLocal : public TPacketHandlersInst
{            
	public:
		TPacketHandlersInstLocal() : TPacketHandlersInst() {} 
			
		virtual void onEndRxPacketProcess(uint16_t src)
		{                                                       
			MasterTxLocal.onRxSlavePacket(src);
			TPacketHandlersInst::onEndRxPacketProcess(src);
		}
};                                  


// Буферы для выходных пакетов
static uint8_t 	txBuffer1[TProtocol::MaxPacketSize],
#ifdef LINK_ID_EXTERNAL
							txBuffer2[TProtocol::MaxPacketSize],
#endif
							txBuffer3[1536];


//TPacketHandlersInst								packetHandlersHiIO;
TPacketHandlersInst_AppPackets			packetHandlersHiIO;
TPacketHandlersInstLocal						packetHandlersLocal;
#ifdef LINK_ID_EXTERNAL
TPacketHandlersInst									packetHandlersExternal;
#endif
	
//-----------------------------------------------------------------------------
//
// Обработчики протокола - отдельные для каждого канала связи
//

//static TProtocolInstance	protocolInst_HiIO(    &packetHandlersHiIO, 			LINK_ID_HI_IO);     
TProtocolInstance_AppPackets	protocolInst_HiIO(&packetHandlersHiIO,	LINK_ID_HI_IO, txBuffer3, TProtocol::MaxPacketPayload);

TProtocolInstance	protocolInst_Local(           &packetHandlersLocal, LINK_ID_LOCAL, txBuffer1);
#ifdef LINK_ID_EXTERNAL
TProtocolInstance	protocolInst_External(&packetHandlersExternal,	LINK_ID_EXTERNAL, txBuffer2);
#endif

//-----------------------------------------------------------------------------
//
// Реализация интерфейса к обработчикам входящих сообщений
// Один обработчик для разных каналов связи, экземпляру объекта присваивается
// индивидуальный идентификатор, по которому определяется вызываемый обработчик.
//
class TRxMessageHandlerInst : public TRxMessageHandler
{                                                   
	uint8_t id;
	
	public:                                                                         
		TRxMessageHandlerInst(uint8_t aid) : id(aid) {}
		           
		virtual void processRxMessage(uint8_t* message, uint16_t destAddress, uint16_t srcAddress)
		{     
/*			char s[128];                                                
			sprintf(s, "%d id:%d msg:%d da:%d sa:%d\n", now, id, (uint32_t)(*message), destAddress, srcAddress);
			puts(s); */

			if(id == LINK_ID_HI_IO)
				P_SlaveMsgHandlers_HiIO::RxMessageHandler(message, destAddress, srcAddress, &protocolInst_HiIO);
			else if(id == LINK_ID_LOCAL)
			{
				P_SlaveMsgHandlers_Local::RxMessageHandler(message, destAddress, srcAddress);
			}
#ifdef LINK_ID_EXTERNAL
			else if(id == LINK_ID_EXTERNAL  &&  destAddress == DEFAULT_NET_ADDRESS)
			{
				P_SlaveMsgHandlers_Ext::RxMessageHandler(message, destAddress, srcAddress);
			} 
#endif
		}
};                


TRxMessageHandlerInst		rxMsgHandler_HiIO(LINK_ID_HI_IO),
												rxMsgHandler_Local(LINK_ID_LOCAL);
#ifdef LINK_ID_EXTERNAL
TRxMessageHandlerInst		rxMsgHandler_External(LINK_ID_EXTERNAL);
#endif

//-----------------------------------------------------------------------------
//
// Интерфейс передачи в канал связи; реализация в hal_uarts.cpp
//
static TLayer1TxInterface		layer1TxInterface;     

//-----------------------------------------------------------------------------
//
// Обмен с ВУ
//

TAppProtocol_AppPackets		protocol_HiIO(&layer1TxInterface, &rxMsgHandler_HiIO, &protocolInst_HiIO);

//-----------------------------------------------------------------------------
//
// Обмен с ведомыми устройствами - узлы на локальной шине и внешние датчики
//                        

static TTxRequest	txRqBuffers1[TX_RQ_N_LOC];
#ifdef LINK_ID_EXTERNAL
static TTxRequest	txRqBuffers2[TX_RQ_N_EXT]; 
#endif

// очереди исходящих запросов
static TQueue<TTxRequest>	txQueueLoc((uint8_t*)txRqBuffers1, TX_RQ_N_LOC);
#ifdef LINK_ID_EXTERNAL
static TQueue<TTxRequest>	txQueueExt((uint8_t*)txRqBuffers2, TX_RQ_N_EXT);
#endif

/*
//
// Реализация callback для уведомления о событиях в автомате опроса ведомых устройств
//													
static class TMsTxCallback : public TMasterTx::TCallback
{              
	uint32_t	group, eventId;
	
	public:                
		TMsTxCallback(uint32_t g, uint32_t e) : group(g), eventId(e) {}
		
		// eventId
		// 0		Таймаут ответа от устройства; p1: адрес устройства
		virtual void proc(uint16_t eventId, uint32_t p1, uint32_t p2)
		{                                            
			sendMessage(group, eventId, p1); 
		}
};

static TMsTxCallback	callbackLoc(grpDataIO, msgAnswerTimeout),
											callbackExt(grpDataIO, msgAnswerTimeout); */

//
TMasterTx		MasterTxLocal(
							LINK_ID_LOCAL,
							&protocolInst_Local,
							&layer1TxInterface, 
							&txQueueLoc,
//							&callbackLoc,
							0,
							TIMEOUT_SLAVE_ANSWER_LOC,
							TIMEOUT_END_PACKET_LOC);
//
#ifdef LINK_ID_EXTERNAL
TMasterTx		MasterTxExternal(
							LINK_ID_EXTERNAL,
							&protocolInst_External,
							&layer1TxInterface, 
							&txQueueExt,
//							&callbackExt,
							0,
							TIMEOUT_SLAVE_ANSWER_EXT,
							TIMEOUT_END_PACKET_EXT);
#endif    


//-----------------------------------------------------------------------------
//    Специфичные для Hi_IO функции - обслуживание динамического изменения
//    типа протокола и соотвтетсвующего измеения размера данных в 
//    выходном пакете.
//-----------------------------------------------------------------------------

namespace hi_io
{
enum {kProtocolDataIO=0, kProtocolApp};					// тип протокола

static uint16_t protocolType = kProtocolDataIO;		// текущий тип протокола

T_U32_void	getTxFreeHandler = 0;


//
// Установка обработчика получения размера свободного места в выходном буфере 
// при работе по протоколу App
//
void setHandler_getTxFree(T_U32_void h)
{     
	getTxFreeHandler = h;
}

//
// Возвращает макс. размер выходного пакета, включая служебную информацию.
//
uint32_t getMaxPacketSize(void)
{
	if(protocolType == kProtocolDataIO)
		return TProtocol::MaxPacketSize;
	else
		return MAX_TX_APP_SIZE_DATAIO_APP;
} 


//
// Возвращает размер свободного места в выходном буфере используемого канала связи.
//
uint32_t getTxFree(void)
{
	if(protocolType == kProtocolDataIO)
		return layer1TxInterface.getTxFree(LINK_ID_HI_IO);
	else
		return getTxFreeHandler ? getTxFreeHandler() : 0;
} 

};		// namespace info



//-----------------------------------------------------------------------------
//                    Операции конфигурирования модуля
//-----------------------------------------------------------------------------

//
// Инициализация модуля протокола
// 
void init(void)
{
	protocolInst_HiIO.install(    &rxMsgHandler_HiIO,     &layer1TxInterface);
	protocolInst_Local.install(   &rxMsgHandler_Local,    &layer1TxInterface);
#ifdef LINK_ID_EXTERNAL
	protocolInst_External.install(&rxMsgHandler_External, &layer1TxInterface);
#endif
}     


//
// Установка параметров интерфейса связи - сетевой адрес и таймаут приема пакета
//
// id						идентификатор канала свзяи - см. LINK_ID_..
// addr					сетевой адрес интерфейса
// rxTimeout		таймаут приема пакета, мс
// 
//
void configure(uint8_t id, uint16_t addr, uint16_t rxTimeout)
{
	if(id == LINK_ID_HI_IO)
		protocol_HiIO.configure(addr, PROTOCOL_BROADCAST_ADDRESS, rxTimeout);
	else if(id == LINK_ID_LOCAL)
		protocolInst_Local.configure(addr, PROTOCOL_BROADCAST_ADDRESS, rxTimeout);
#ifdef LINK_ID_EXTERNAL
	else if(id == LINK_ID_EXTERNAL)
		protocolInst_External.configure(addr, PROTOCOL_BROADCAST_ADDRESS, rxTimeout);
#endif
}                  

//
// Установка макс. размера прикладных данных в выходном пакете.
// id		идентификатор канала свзяи - см. LINK_ID_..
//
/*void setTxAppSize(uint16_t size, uint8_t id)
{
	if(     id == LINK_ID_HI_IO)		protocolInst_HiIO.setTxAppSize(size);
	else if(id == LINK_ID_LOCAL)		protocolInst_Local.setTxAppSize(size);
#ifdef LINK_ID_EXTERNAL
	else if(id == LINK_ID_EXTERNAL)	protocolInst_External.setTxAppSize(size);
#endif
} */


//
// Установка макс. размера прикладных данных в выходном пакете.
// Используется только для протокола HI_IO.
//
void setTxAppSize(uint16_t size)
{
	protocolInst_HiIO.setPayload(size);
}


//
// Установка обработчика передачи пакетов прикладного уровня - без заголовка DataIO
//
void setAppTxHandler(T_I16_U8ptr_U16 h)
{                 
	protocol_HiIO.setAppTxHandler(h);
}


//-----------------------------------------------------------------------------
//          Опрос ведомых устройств по интерфейсам Local и External
//-----------------------------------------------------------------------------
	
//
// Проверка доступности очереди исходящих сообщений
// id		идентификатор канала свзяи - см. LINK_ID_..
//
// rc		true	- есть место в очереди исходящих
//			false - очередь заполнена
//
bool isTxBufferFree(uint8_t id)
{
	if(     id == LINK_ID_LOCAL)			return txQueueLoc.full() ? false : true;
#ifdef LINK_ID_EXTERNAL
	else if(id == LINK_ID_EXTERNAL)		return txQueueExt.full() ? false : true;
#endif
	else return false;
}

//
// Проверка отсутствия исходящих сообщений в очереди
// id		идентификатор канала свзяи - см. LINK_ID_..
//
// rc		true	- есть место в очереди исходящих
//			false - очередь заполнена
//
bool isTxBufferEmpty(uint8_t id)
{
	if(     id == LINK_ID_LOCAL)			return txQueueLoc.empty();
#ifdef LINK_ID_EXTERNAL
	else if(id == LINK_ID_EXTERNAL)		return txQueueExt.empty();
#endif
	else return false;
}


//
// Очистка очереди исходящих сообщений
// id		идентификатор канала свзяи - см. LINK_ID_..
//
void flushTxBuffer(uint8_t id)
{
	if(     id == LINK_ID_LOCAL)			txQueueLoc.clear();
#ifdef LINK_ID_EXTERNAL
	else if(id == LINK_ID_EXTERNAL)		txQueueExt.clear();
#endif
}

//
// Добавляет в выходную очередь запрос на запись параметра
// id				id канала связи
//
void addTxMsgWrite(uint16_t a, uint32_t p0, uint32_t p1, uint32_t f, uint8_t id)
{
	TTxRequest txRq(DataIO::MsgParameterWrite, a, p0, p1, f);          
	if(id == LINK_ID_LOCAL)
		txQueueLoc.push((const TTxRequest*)&txRq);
#ifdef LINK_ID_EXTERNAL
	else if(id == LINK_ID_EXTERNAL)
		txQueueExt.push((const TTxRequest*)&txRq);
#endif
}

//
// Добавляет в выходную очередь запрос на чтение параметра
// id				id канала связи
//
void addTxMsgRead(uint16_t a, uint32_t p0, uint32_t f, uint8_t id)
{
	TTxRequest txRq(DataIO::MsgParameterRead, a, p0, 0, f);
	if(id == LINK_ID_LOCAL)
		txQueueLoc.push((const TTxRequest*)&txRq);
#ifdef LINK_ID_EXTERNAL
	else if(id == LINK_ID_EXTERNAL)
		txQueueExt.push((const TTxRequest*)&txRq);
#endif
}          


//
// Добавляет в выходную очередь прикладное сообщение
// a			адрес получателя
// t			тип прикладного сообщения
// sz			число байт данных - макс. 4
// data		байты данных  0..3
// f			tx flags - TTxRequest::FRqAnswer, FBreakPacket
// id			id канала связи
//
bool addTxMsgApp(uint16_t a, uint32_t t, uint32_t sz, uint32_t data, uint32_t f, uint8_t id)
{   
	if(sz <= sizeof(data))
	{                 
		// parameter0		b31:b16 - messageType
		//							b15:b0  - dataSize
		// parameter1		data (max. 4 bytes)
		TTxRequest txRq(DataIO::MsgAppType, a, ((t << 16) | sz), data, f);

		if(id == LINK_ID_LOCAL)
			txQueueLoc.push((const TTxRequest*)&txRq);
#ifdef LINK_ID_EXTERNAL
		else if(id == LINK_ID_EXTERNAL)
			txQueueExt.push((const TTxRequest*)&txRq);
#endif
		return true;
	}
	else return false;
}                                                    


// Добавляет в выходную очередь запрос на чтение блока параметров    
//
// a			адрес устройства
// base		базовый адрес блока параметров
// size		кол-во параметров
// f			флаги управления передачей сообщения
// id			идентификатор канала свзяи - см. LINK_ID_..
void addTxMsgReadBlock(uint16_t a, uint16_t base, uint16_t size, uint32_t f, uint8_t id)
{
	TTxRequest txRq(DataIO::MsgReadBlock, a, base, size, f);
	if(id == LINK_ID_LOCAL)
		txQueueLoc.push((const TTxRequest*)&txRq);
#ifdef LINK_ID_EXTERNAL
	else if(id == LINK_ID_EXTERNAL)
		txQueueExt.push((const TTxRequest*)&txRq); */
#endif
}      


//-----------------------------------------------------------------------------
//                    Обмен по интерфейсу Hi_IO
//-----------------------------------------------------------------------------

//
// Добавить сообщение в пакет для передачи.
//
// Используется только при ответе на запросы от ВУ.
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
//
int16_t addMessage(uint8_t* msgPtr, uint8_t* data)
{
	return protocolInst_HiIO.addMessage(msgPtr, data);
}


//
// Возвращает размер данных в выходном пакете
//
uint16_t getTxDataSize(void)
{ 
	return protocolInst_HiIO.getTxDataSize(); 
}


//
// Передача исходящего пакета в канал связи
//
int16_t sendPacket(int16_t src)
{
	return protocolInst_HiIO.sendPacket(src);
}     

//
// Инициализировать выходной пакет перед формированием сообщений
//
void startTx(void)
{                                                     
	packetHandlersHiIO.onBeginRxPacketProcess();
}



//
// Обработка принятого пакета DataIOApp - без заголовка пакета.
//
// buffer					буфер, содержащий входные данные
// n							количество байт для чтения из буфера        
// destAddress		адрес получателя
// srcAddress			адрес отправителя                                   
//
// Возврат:  количество разобранных из buffer прикладных сообщений
//
int16_t processInputAppData(uint8_t* buffer, uint16_t n, uint16_t dest, uint16_t src)
{             
	// Установка макс. размера данных в соответствии с пакетом DataIOApp
#ifdef MAX_TX_APP_SIZE_DATAIO_APP
	if(hi_io::protocolType != hi_io::kProtocolApp)
	{                              
		hi_io::protocolType = hi_io::kProtocolApp;
		protocolInst_HiIO.setPayload(MAX_TX_APP_SIZE_DATAIO_APP);
	}
#endif

	return protocol_HiIO.processInputAppData(buffer, n, dest, src);
}        


//-----------------------------------------------------------------------------
//          Обработка входящих данных по протоколу DataIO
//-----------------------------------------------------------------------------

//
// Обработчик принятых данных из канала связи
// buffer		входящие данные
// n				число байт для обработки
// id				id канала связи - LINK_ID_..
//
int16_t processInputData(TBuffer* buffer, uint16_t n, uint16_t id)
{         
	int16_t r=0;
	if(id == LINK_ID_HI_IO)              
	{               
		// Установка макс. размера данных в соответствии с пакетом проткола DataIO
#ifdef MAX_TX_APP_SIZE_DATAIO
		if(hi_io::protocolType != hi_io::kProtocolDataIO)
		{                              
			hi_io::protocolType = hi_io::kProtocolDataIO;
			protocolInst_HiIO.setPayload(MAX_TX_APP_SIZE_DATAIO);
		}
#endif
		
		r = protocol_HiIO.processInputData(buffer, n);             
	}
	else if(id == LINK_ID_LOCAL)              
	{	
		MasterTxLocal.onRxSlaveData();
		r = protocolInst_Local.processInputData(buffer, n);             
	}
#ifdef LINK_ID_EXTERNAL
	else if(id == LINK_ID_EXTERNAL)              
		r = protocolInst_External.processInputData(buffer, n);             
#endif		    

	return r;
}                                                                  

};		// namespace



			
			
