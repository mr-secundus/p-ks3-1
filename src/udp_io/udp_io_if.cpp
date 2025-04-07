// udp_io_if.cpp
//
// »нтерфейс приема и передачи данных по UDP

// –еализаци€ с копированием данных в буфер нижнего уровн€.
//
// ѕри добавлении исход€щих данных, выдел€етс€ буфер из пула буферов нижнего
// уровн€ и в него копируютс€ вход€щие данные.


#include <stdio.h>
#include <string.h>

#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/udp.h"

#include <putx.h>

#include "cfg/defines.h"
#include "hal.h"
#include "debug.h"
#include "udp_io/udp_io_if.h"
#include "udp_io/udp_io_cfg.h"


namespace udp_io
{
// 
// ¬озвращает макс. размер прикладных данных в пакете
//
uint16_t getMaxAppDataSize(void)
{                                       
	return UDP_DATA_MAX_SZ;
}


// 
// ¬озвращает размер свободного места в выходном буфере.
// ¬ычисл€етс€ исход€ из количества свобоных буферов и размера прикладных данных 
// в буфере - без учета длины заголовков UDP IP ETH.
//
uint32_t getTxFree(void)
{                                       
	return (UDP_TX_Q_SIZE - txQueue.size()) * getMaxAppDataSize();
}


//
// ƒобавл€ет данные в очередь исход€щих пакетов. 
// –азмер передаваемых данных ограничиваетс€ UDP_APP_DATA_MAX_SZ.
//
// data		данные дл€ передачи
// size		число байт в data
//
// return		> 0		число переданных байт
//					< 0		kRcNetErr, kRcNoConnection, kRcTxQueueFull, kRcTxBufferFull     
//
int16_t addTxData(uint8_t* data, uint16_t size)
{         
//	if(udp_conn_tx_data == 0  &&  !updateConnTx) return kRcNoConnection;       
	if(txQueue.full())		return kRcTxQueueFull;
		
	uint8_t* buffer = udpBuffers.get();

	if(buffer != nullptr)
	{
		if(size > getMaxAppDataSize())
			size = getMaxAppDataSize();
				
//		UdpData udpdata(buffer, size, udp_conn_tx_data, now);
		UdpData udpdata(buffer, size, nullptr);

		memcpy((uint8_t*)udpdata.appData(buffer), data, size);
		
		txQueue.push(&udpdata);
		return size;
	}
	else
		return kRcTxBufferFull;
}


//
// ќбработка очереди прин€тых пакетов
// ƒанные передаютс€ на обработку в processRxData()
//
// maxPasses		макс. количество пакетов, обрабатываемых за один вызов
//
void processRxQueue(T_v_TUDPDataPtr processRxData, uint16_t maxPasses)
{                           
	uint16_t cnt=0;
	while(!rxQueue.empty()  &&  cnt < maxPasses)
	{                 
		UdpData *data = rxQueue.front();
		
		processRxData(data);
		
		pbuf_free((struct pbuf *)data->ex);
		udpBuffers.release(data->buffer);
		rxQueue.pop();                         
		
		cnt++;
	}
} 

//
// ѕередача данных из очереди исход€щих пакетов
//
// maxPasses		макс. количество пакетов, обрабатываемых за один вызов
//
void processTxQueue(uint16_t maxPasses)
{
	uint16_t cnt = 0;
	
//	while(!txQueue.empty()  &&  eth_get_free_tx() != 0  &&  cnt < maxPasses)
	while(!txQueue.empty()  &&  cnt < maxPasses)
	{                 
		// TODO: перед отправкой провер€ть условие (data->size < eth_get_free_tx()) 
		
		UdpData *data = txQueue.front();

		if(send(data) > 0)	// пакет передан в MAC - освободить буфер и извлечь из очереди
		{
			udpBuffers.release(data->buffer);
			txQueue.pop();
		} 
		
		cnt++;
	}
}

}			// namespace    
