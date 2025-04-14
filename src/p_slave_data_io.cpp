// p_slave_data_io.cpp
//
// Опрос ведомых устройств и обработка принятых данных

#include <stdio.h>

#include <putx.h>

#include "hal.h"
#include "debug.h"
#include "cfg/defines.h"
#include "data_buffers.h"
#include "data.h"
#include "p_slave_poll.h"
#include "p_slave_data_io.h"


namespace p_slave_data_io
{
int16_t dataHandler(TDataBuffer* p, uint16_t dataId);

//-----------------------------------------------------------------------------
//                              Variables
//-----------------------------------------------------------------------------

SlavePoll	slPoll(dataHandler);

//-----------------------------------------------------------------------------
//                                 Private
//-----------------------------------------------------------------------------

//
// Обработка принятого блока данных
//
int16_t dataHandler(TDataBuffer* p, uint16_t dataId)
{
	int16_t rc = data_buffers::writeBuffer(p, dataId); 

	char s[128];
	
	// вывод информации о принятом блоке данных
	sprintf(s, "%lu rcv data pn:%d sy:%lu size:%d\n", now, p->packet, p->sync, p->size);
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

// Инициализация
void init(void)
{ 
	slPoll.configure(SLAVE_IO_RQ_PERIOD, SLAVE_IO_RQ_TIMEOUT);
	slPoll.setup(setup3.slaves);
}


//
void start(void)
{
	slPoll.start();
}


//
void stop(void)
{
	slPoll.stop();
}


// Главный цикл работы 
void process(void)
{
	slPoll.process();
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
