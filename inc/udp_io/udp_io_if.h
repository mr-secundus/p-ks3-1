// udp_io_if.h
//
// »нтерфейс к модулю обмена по UDP

#include "udp_io/udp_io.h"

namespace udp_io
{
// callback дл€ обработки прин€тых данных UDP
//	
// arg0		указатель на UdpData
typedef void (*T_v_TUDPDataPtr)(UdpData*);

// 
// ¬озвращает макс. размер прикладных данных в пакете
//
uint16_t getMaxAppDataSize(void);       

// 
// ¬озвращает размер свободного места в выходном буфере.
// ¬ычисл€етс€ исход€ из количества свободных буферов и размера прикладных данных 
// в буфере - без учета длины заголовков UDP IP ETH.
//
uint32_t getTxFree(void);

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
int16_t addTxData(uint8_t* data, uint16_t size);

//
// ќбработка очереди прин€тых пакетов
// ƒанные передаютс€ на обработку в processRxData()
//
// maxPasses		макс. количество пакетов, обрабатываемых за один вызов
//
void processRxQueue(T_v_TUDPDataPtr processRxData, uint16_t maxPasses);       

//
// ѕередача данных из очереди исход€щих пакетов
//
// maxPasses		макс. количество пакетов, обрабатываемых за один вызов
//
void processTxQueue(uint16_t maxPasses);
};
