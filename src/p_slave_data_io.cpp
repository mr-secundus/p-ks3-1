// p_slave_data_io.cpp
//
// Опрос ведомых устройств и обработка принятых данных
//
// Управление автоматом опроса ведомых устройств:
//   - пуск/останов опроса
//   - периодическое выполнение автомата
// Интерфейс между обработчиком сообщений от ведомых устройств
// и контейнером буферов данных для передачи на ВУ.

#include <stdio.h>

#include <putx.h>

#include "hal.h"
#include "debug.h"
#include "cfg/defines.h"
#include "data_buffers.h"
#include "data.h"
#include "p_slave_poll.h"
#include "p_slave_data_io.h"

#ifdef SIM_DATA	
#include "test/sim_data.h"
#endif


namespace p_slave_data_io
{
int16_t dataHandler(uint16_t srcAddress, TDataBuffer* p, uint16_t dataId);

//-----------------------------------------------------------------------------
//                                Variables
//-----------------------------------------------------------------------------

SlavesPoll	slPoll(dataHandler);

#ifdef SIM_DATA	
TDataBuffer simBuffer;

SimData simData;
#endif


//-----------------------------------------------------------------------------
//                                 Private
//-----------------------------------------------------------------------------

// Обработка принятого блока данных
int16_t dataHandler(uint16_t srcAddress, TDataBuffer* p, uint16_t dataId)
{
	int16_t rc = data_buffers::writeBuffer(srcAddress, p, dataId); 

	char s[128];
	
	// вывод информации о принятом блоке данных
	sprintf(s, "%lu rcv addr:%d pn:%d sy:%lu size:%d\n",
					now, srcAddress, p->packet, p->sync, p->size);
	puts(s);

	// вывод значений отсчетов
/*	int32_t* d = (int32_t*)p->data;
	for(int i=0; i<p->dataSize; i+=ADC_CH_NUM)
	{
		uint32_t t = now;
		sprintf(s, "%d %d %d\n", d[i], d[i+1], d[i+2]);
		puts(s);
		while(t == now);
	} */
	
	return rc;
}


//-----------------------------------------------------------------------------
//                                 Public
//-----------------------------------------------------------------------------

// Инициализация автомата опроса
void init(void)
{ 
	slPoll.configure(SLAVE_IO_RQ_PERIOD, SLAVE_IO_RQ_TIMEOUT);
	slPoll.setup(setup3.slaves);
	
#ifdef SIM_DATA	
	uint16_t sladdr[SLAVE_IO_SLAVES_N] = {1, 0, 0, 0, 0, 0, 0, 0};
	
	simData.setSlavesAddress(sladdr);
	simData.setFd(50000);								// 50 Гц -> T=840 мс
#endif
}


// Пуск опроса
void start(void)
{
	slPoll.start();
	
#ifdef SIM_DATA
	simData.reset(now);
#endif	
}


// Останов опроса
void stop(void)
{
	slPoll.stop();
}


// Текущие операции работы автомата опроса 
void process(void)
{
	slPoll.process();
	
#ifdef SIM_DATA
	// Автомат опроса находится в режиме ожидания ответа от устройства?
	if(slPoll.getState() ==  SlavesPoll::kStWait)
	{
		// Готовность данных по текущему адресу?
		if(simData.slaveReady(slPoll.getAddress(), now))
		{
			// Заполнить блок данных и передать его на обработку
			simData.getData(slPoll.getAddress(), now, &simBuffer);
			slPoll.handleData(slPoll.getAddress(), &simBuffer, kDataId);
		}
	}
#endif	
}


// Обработка принятого блока данных
//
// srcAddress   сетеовй адрес отправителя
// p					  блок данных
// dataId			  идентификатор типа данных из входящего сообщения
//
// return       > 0   размер обработанного блока
//              < 0   ошибка - см. data_buffers::writeData()
int16_t handleMessageData(uint16_t srcAddress, TDataBuffer* p, uint16_t dataId)
{
	return slPoll.handleData(srcAddress, p, dataId);
}


// Обработка сообщения об отсутствии данных
//
// srcAddress   сетеовй адрес отправителя
void handleMessageNoData(uint16_t srcAddress)
{
	slPoll.handleNoData(srcAddress);
}

};		// namespace
