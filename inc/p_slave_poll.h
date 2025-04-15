// p_slave_poll.h
//
// Автомат опроса источников данных

#ifndef P_SLAVE_POLL_H
#define P_SLAVE_POLL_H

#include "hal.h"
#include "dataio/tdatabuffer.h"
#include "protocol.h"  
#include "cfg/protocol.h"
#include "cfg/slave_io.h"


// Формирование запросов к устройству и прием блоков данных
class SlavesPoll
{
public:
	static constexpr uint16_t kMaxSlaves = SLAVE_IO_SLAVES_N;

	// Состояние автомата опроса
	using state_t = enum
	{
		kStIdle = 0,							// остановлено
		kStStart,									// запуск опроса
		kStStartPeriod,						// начало очередного цикла опроса
		kStSendRq,								// отправка запроса
		kStWait,									// ожидание ответа от устройства
		kStNext,									// переход к следующему устройству
		kStWaitRqInterval,				// ожидание завершения периода опроса 
		kStStop										// останов опроса
	};

	using data_handler_t  = int16_t (*)(uint16_t, TDataBuffer*, uint16_t);
	
	// Результат выполнения запроса к устройству
//	static const uint16_t kResIdle 			= 0;		// запрос не выполнялся
//	static const uint16_t kResRcvData 	= 1;		// приняты данные
//	static const uint16_t kResNoData 		= 2;		// нет данных
//	static const uint16_t kResTimeout 	= 3;		// таймаут ответа
	using rq_res_t = enum
	{
		kResIdle	= 0,						// запрос не выполнялся
		kResWait,									// ожидание ответа
		kResRcvData,							// приняты данные
		kResNoData,								// нет данных
		kResTimeout								// таймаут ответа
	};
	
protected:
	static constexpr uint16_t kPnBufferFree = 0xFFFF;
	
	// Информация об одном ведомом устройстве - адрес + текущее состояние опроса
	using slave_info_t = struct
	{
		uint16_t	address,				// адреса ведомоых устройств
							packet;					// номер принятого пакета
		rq_res_t	state;					// результат предыдущего запроса
	
	};

	data_handler_t	dataHandler;					// обработчик блока данных

	state_t		state;											// состояние автомата опроса

	slave_info_t	slaves[kMaxSlaves];			// ведомые устройства

	uint32_t 	rqPeriod,										// период формирования запросов, мс
						rqTimeout,									// таймаут ожидания ответа
						tmPeriod,										// время начала цикла опроса 
						tmRq;												// время передачи запроса
	uint16_t	index;											// индекс в slaves[] 
	uint8_t 	sequenceNumber;							// sequence number для исходящих запросов
	
	bool			rcvData,										// флаги приема ссобщений от устройства
						rcvNoData;


	// Возвращает очередной sequenceNumber для нового запроса
	uint8_t getSeqNumber(void)
	{
		uint8_t s = sequenceNumber;
		if(sequenceNumber++ >= 254) 
			sequenceNumber = 0;
		return s;
	}


	// Отправка запроса на передачу блока данных
	//
	// addr		адрес получателя
	void sendRequest(uint16_t addr)
	{
/*		dataio::TMsgAppTypeHeader msg;
		msg.id								= dataio::MsgAppType;
		msg.sequenceNumber		= getSeqNumber();
		msg.type							= dataio::MsgApp_RequestData;
		msg.dataSize					= 0;
		
		protocol::sendPacket((uint8_t*)&msg, 0, slaveAddress); */
		
		// Добавляет в выходную очередь прикладное сообщение
		// a			адрес получателя
		// t			тип прикладного сообщения
		// sz			число байт данных - макс. 4
		// data		байты данных  0..3
		// id				id канала связи
		Protocol::addTxMsgApp(addr, DataIO::MsgApp_RequestData, 0, 0,
													TTxRequest::FBreakPacket | TTxRequest::FRqAnswer, SLAVE_IO_LINK_ID);
	}

	// Отправка подтверждения приема блока данных
	//
	// addr		адрес получателя
	// pn			номер пакета
	void sendDataAck(uint16_t addr, uint16_t pn)
	{
/*		dataio::TMsgAppTypeHeader msg;
		msg.id								= dataio::MsgAppType;
		msg.sequenceNumber		= getSeqNumber();
		msg.type							= dataio::MsgApp_DataAcknowledge;
		msg.dataSize					= sizeof(uint16_t);

		protocol::sendPacket((uint8_t*)&msg, (uint8_t*)&pn, slaveAddress); */

		uint16_t dataSize = sizeof(uint16_t);

		Protocol::addTxMsgApp(addr, DataIO::MsgApp_DataAcknowledge, sizeof(uint16_t), dataSize,
													TTxRequest::FBreakPacket | TTxRequest::FRqAnswer, SLAVE_IO_LINK_ID);
	}

public:
	SlavesPoll(data_handler_t h) :
		dataHandler(h),
		state(kStIdle),
		index(0),
		sequenceNumber(0)
	{
	}
	
	// Возвращает текущее состояние автомата
	int16_t getState(void)
	{
		return state;
	}

	// Возвращает адрес текущего опрашиваемого устройства в соответствии с index.
	// При некорректном значении index возвращает 0.
	int16_t getAddress(void)
	{
		return index < kMaxSlaves ? slaves[index].address : 0;
	}

	// Установка параметров опроса
	//
	// arqPeriod			период цикла опроса, мс
	void configure(uint32_t period, uint32_t timeout)
	{
		rqPeriod = period;
		rqTimeout = timeout;
	}
	
	// Инициализация адресов ведомых устройств
	//
	// address				массив адресов
	void setup(uint16_t *address)
	{
		for(uint16_t i = 0; i < kMaxSlaves; i++)
			slaves[i].address = address[i];
	}

	// Пуск/останов опроса 
	void start(void) { state = kStStart; }
	void stop(void)  { state = kStStop; }


	// Автомат формирования запросов и обработки ответа
	void process(void)
	{
		switch(state)
		{
			case kStIdle:		break;

			case kStStart:
				for(auto& x : slaves)
					x.packet = kPnBufferFree;
				
				sequenceNumber = 0;
				state = kStStartPeriod;
				break;
			
			case kStStop:
				state = kStIdle;
				break;
				
			case kStStartPeriod:
				index = 0;
				tmPeriod = now;
				state = kStSendRq;
				break;

			case kStSendRq:
				if(slaves[index].address > 0)
				{
					sendRequest(slaves[index].address);
					rcvData = rcvNoData = false;
					slaves[index].state = kResWait; 
					tmRq = now;
					state = kStWaitRqInterval;
				}
				else
					state = kStNext;
				break;
			
			case kStWait:
				if(TIMEOUT(tmRq, rqTimeout))
				{
					slaves[index].state = kResTimeout; 
					state = kStNext;
				}
				else if(rcvData)
				{
					state = kStNext;
				}
				else if(rcvNoData)
				{
					state = kStNext;
				}
				break;
				
			case kStNext:
/*				state = kStSendRq;
				do
				{
					if(++index == kMaxSlaves)
					{
						state = kStWaitRqInterval; 
						break;
					}
				}while(slaves[index].address == 0); */
				
				if(++index == kMaxSlaves)
				{
					index = 0;
					state = kStWaitRqInterval;
				}
				else
					state = kStSendRq;
				break;

			case kStWaitRqInterval:
				if(TIMEOUT(tmPeriod, rqPeriod))
					state = kStStartPeriod;
				break;
		}
	}

	
	// Обработка принятого блока данных
	//
	// Отправляет подтверждение на полученный блок.
	// При соответствии адреса отправителя вызывает обработчик входящих блоков.
	//
	// srcAddress   сетеовй адрес отправителя
	// p						блок данных
	// dataId				идентификатор типа данных из входящего сообщения
	//
	// return       > 0   размер обработанного блока
	//							  0		несоответствие адреса отправителя, или повторный пакет
	//              < 0   ошибка - см. data_buffers::writeData()
	int16_t handleData(uint16_t srcAddress, TDataBuffer* p, uint16_t dataId)
	{
		rcvData = true;
		sendDataAck(srcAddress, p->packet);						// подтверждение получения блока данных

		if(srcAddress == slaves[index].address)
		{
			slaves[index].state = kResRcvData; 
				
//			sendDataAck(slaves[index].address, p->packet);		// подтверждение получения блока данных
	
			// dataId не контролируется - обрабатываем все входящие пакеты
			if(p->packet != slaves[index].packet)					// исключаем обработку повторных пакетов
			{
				slaves[index].packet = p->packet;
				return dataHandler(srcAddress, p, dataId); 
			}
		}
		return 0; 
	}


	// Обработка сообщения об отсутствии данных
	//
	// srcAddress   сетеовй адрес отправителя
	void handleNoData(uint16_t srcAddress)
	{
		rcvNoData = true;

		if(srcAddress == slaves[index].address)
			slaves[index].state = kResNoData; 
	}
};		// class

#endif