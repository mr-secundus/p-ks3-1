// udp_io.h
//
// Интерфейс к модулю обмена по UDP

#include <tqueue.h>
#include "udp_io/udp_data.h"
#include "udp_io/tbufferspool.h"

namespace udp_io
{
// Коды возврата функций модуля
static const int16_t	
	kRcOk							= 0,
	kRcNetErr					= -1,			// ошибка сетевого стека
	kRcAllocErr				= -2,			// ошибка выделения памяти
	kRcNoConnection		= -3,			// отсутствует соединение для передачи данных
	kRcTxQueueFull		= -4,			// нет места в выходной очереди
	kRcTxBufferFull		= -5;			// нет свободных буферов     
		

extern TQueue<UdpData> txQueue, rxQueue;		// очереди исходящих и входящих сообщений
extern TBuffersPool 	 udpBuffers;	
	
//
// Инициализация модуля
//
// alport		local port
// arport		remote port
//
// return		kRcOk, kRcNetErr
//
int16_t init(uint16_t alport, uint16_t arport);

//
void process(void);   

//
// Передача пакета
//
// return		размер переданного пакета, байт
//
uint32_t send(UdpData *data);

};
