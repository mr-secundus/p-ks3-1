// protocol.h

//-----------------------------------------------------------------------------
//      Обмен по протоколу DataIO с ВУ, узлами и внешними устройствами
//-----------------------------------------------------------------------------

//
// Идентификаторы каналов связи - передаются в соотв. объекты TProtocolInstance,
// соответствуют номерам используемых UART - см. реализацию TLayer1TxInterface в hal_uarts.cpp.
// Указаны номера разъемов на плате интерфейса.
//
#define LINK_ID_HI_IO				3			// связь с ВУ                   RS485 @ X6
#define LINK_ID_LOCAL				1			// внутренняя шина устройства		RS485 @ X7


#define NET_ADDRESS_LOCAL						0					// сетевой адрес интерфейса связи с локальными устройствами
#define NET_ADDRESS_EXT							0					// сетевой адрес интерфейса связи с внешними
																							// ведомыми устройствами

#define PROTOCOL_BROADCAST_ADDRESS	0xff		// broadcast address
#define PROTOCOL_RX_TIMEOUT					5		  // таймаут приема данных, миллисекунд

// обмен с узлами по внутренней шине [миллисекунды]
#define TIMEOUT_SLAVE_ANSWER_LOC   20				// таймаут ожидания ответа от устройства
#define TIMEOUT_END_PACKET_LOC      2				// таймаут ожидания конца пакета (макс. интервал между сообщениями в одном пакете)
//#define TM_PACKET_INTERVAL_LOC		  2				// мин. интервал между пакетами			
#define TM_POLL_INTERVAL_LOC			 10 			// интервал опроса устройств

#define TX_RQ_N_LOC								16				// размер очереди запросов Local

#define AI4R_NET_ADDRESS					10 				// сетевой адрес модуля AI4R

// Макс. размер прикладных данных в выходном пакете для разных вариантов
#define MAX_TX_APP_SIZE_DATAIO				(TProtocol::MaxPacketPayload)			// RS485
#define MAX_TX_APP_SIZE_DATAIO_APP		1472															// UDP

