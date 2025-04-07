// p_mastertx.h    
// 
// Реализация отправки запросов к ведомым устройствам

#ifndef __P_TMASTERTX

#include <tqueue.h>
#include <hal_base.h>
#include "protocol_app/p_thandlers.h"
#include "protocol_app/p_tprotocolinst.h"

//
// Запрос на передачу сообщения в устройство.
// Помещается в очередь, далее по запросам формируются пакеты для передачи.
//
class TTxRequest
{                                   
public:
	// разряды поля flags
	static const uint16_t FRqAnswer			= 1;		// требуется ответ на запрос
	static const uint16_t FBreakPacket	= 2;		// закончить формирование пакета
	                                       
	uint32_t	parameter0,		// номер регистра
						parameter1;		// значение
	uint32_t	flags;				
	uint16_t	type,					// тип сообщения
						address;			// destination address
												
					
	TTxRequest(void) {}

	TTxRequest(uint16_t	t, uint16_t a, uint32_t p0, uint32_t p1, uint32_t f) :
		parameter0(p0), parameter1(p1), flags(f), type(t), address(a) {}	
	
	TTxRequest& operator=(const TTxRequest& c)
	{
		type = c.type;
		address = c.address;
		parameter0 = c.parameter0;
		parameter1 = c.parameter1;                                  
		flags = c.flags;
		return *this;
	}
	
	TTxRequest(const TTxRequest& c)
	{
		operator=(c);
	}

	void set(uint16_t	t, uint16_t a, uint32_t p0, uint32_t p1, uint32_t f)
	{
		type = t; address = a; parameter0 = p0; parameter1 = p1; flags = f;
	}	
};


//           
// Реализация автомата отправки запросов к ведомым утройствам
//
class TMasterTx
{
	public: 
		static const uint16_t	MaxTxSize = 128;		// макс. размер пакета для передачи мастером
		                                  
		// Интерфейс для уведомления прикладного уровня о событиях в автомате
		class TCallback
		{
			public:
				// eventId
				// 0		Таймаут ответа от устройства; p1: адрес устройства
				virtual void proc(uint16_t eventId, uint32_t p1, uint32_t p2) {}
		};
		
	protected:
		typedef enum {stReadyToTx=0, stWaitAnswer, stWaitRxComplete} TState;
	
		uint8_t 	sn,									// sequence number
						id;									// идентификатор канала связи 
		uint16_t	txSize,							// текущее число байт в txBuffer
						state,              // состояние автомата
						address;						// адрес получателя последнего отправленного пакета
		uint32_t  timer,							// оперативный при обработке состояния
						timeoutSlaveAnswer,	// таймаут ответа от устройства
						timeoutEndPacket;		// таймаут ожидания конца пакета
		
		TProtocolInstance		*protocol;			// обработчик протокола 
		TLayer1TxInterface	*txInterface;		// интерфейс передачи в канал связи
		TQueue<TTxRequest>	*txQueue;				// очередь исходящих сообщений
		TCallback						*callback;
						
		// Возвращает очередной sequenceNumber для нового запроса
		uint8_t getSeqNumber(void)
		{
			uint8_t s = sn;
			if(sn++ >= 254) sn = 0;
			return s;
		}

		// Отправка сформированного пакета. Вызывается из обработчика автомата.
		// rc:			false - не удалось передать данные в канал связи, или нет данных для передачи
		bool sendPacket(void)
		{        
			if(txSize)
			{                      
				txSize = 0;
				if(txInterface->write(id, protocol->getTxBuffer(), txSize) == txSize)
					return true;
			}   
			return false;
		}                    
		
	public:
		TMasterTx(uint8_t aid, TProtocolInstance *aprotocol, TLayer1TxInterface *atxInterface,
							TQueue<TTxRequest> *atxQueue, TCallback *cb,
							uint32_t atimeoutSlaveAnswer, uint32_t atimeoutEndPacket) :
			sn(0), id(aid), txSize(0), state(0), timer(0),
			timeoutSlaveAnswer(atimeoutSlaveAnswer),
			timeoutEndPacket(atimeoutEndPacket),
			protocol(aprotocol),
			txInterface(atxInterface),
			txQueue(atxQueue),
			callback(cb)
			{	}
		
		// Обработка автомата состояний
		void process(void);  
		
		// Добавить запрос для передачи
		// *rq				запрос
		// rqAnswer		=true - ожидается ответ на запрос
		void addMessage(TTxRequest* rq, bool rqAnswer);
		
		// Уведомление о приеме данных по обслуживаемому интерфесйу
		// Используется для перехода в режим - или продолжения ожидания освобождения канала связи
		inline void onRxSlaveData(void)
		{                                                                
			if(state == stReadyToTx  ||  state == stWaitRxComplete)
			{
				state = stWaitRxComplete;
				timer = now;
			}
		}

		// Уведомление о приеме пакета от ведомого устройства
		// Завершает режим ожидания ответа и переводит в режим ожидания освобождения канала связи
		// address		адрес вед. устр-ва
		inline void onRxSlavePacket(uint16_t address)
		{                                  
			if(state == stWaitAnswer)
			{
				state = stWaitRxComplete;	
				timer = now;
			}
		}

		// Возвращает true, если автомат готов к обработке и передаче запроса.
		// Иначе - находится в режиме ожидания ответа или отработки таймаута 
		bool idle(void) { return state==0 ? true : false; } 
};           

#endif
