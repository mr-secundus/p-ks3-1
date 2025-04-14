// protocol.h
//
// Настройки обмена по интерфейсам связи

//
// Идентификаторы каналов связи - передаются в соотв. объекты TProtocolInstance
// Для интерфейсов на базе RS485 должны соответствовать номерам используемых UART -
// передаются при вызове функций hal::uarts::
//
#define LINK_ID_HI_IO				9			// связь с ВУ по UDP
#define LINK_ID_LOCAL				1			// опрос ведомых устройств - RS485-1


// default сетевые адреса интерфейсов связи
#define DEFAULT_NET_ADDRESS_HI_IO    	99
#define DEFAULT_NET_ADDRESS_LINK1			1
#define DEFAULT_NET_ADDRESS_LINK2			1

#define PROTOCOL_BROADCAST_ADDRESS	0xFF		// broadcast address

#define PROTOCOL_RX_TIMEOUT					5		  	// таймаут приема данных, миллисекунд

// Обмен с ведомыми устройствами
#define TIMEOUT_SLAVE_ANSWER_LOC   20				// таймаут ожидания ответа от устройства [мс]
#define TIMEOUT_END_PACKET_LOC      2				// таймаут ожидания конца пакета [мс]
																						// (макс. интервал между сообщениями в одном пакете)
//#define TM_PACKET_INTERVAL_LOC		  2				// мин. интервал между пакетами			
#define TM_POLL_INTERVAL_LOC			 10 			// интервал опроса устройств [мс]

#define TX_RQ_N_LOC									8				// размер очереди запросов Local

// Макс. размер прикладных данных в выходном пакете для разных вариантов
#define MAX_TX_APP_SIZE_DATAIO				(TProtocol::MaxPacketPayload)			// RS485
#define MAX_TX_APP_SIZE_DATAIO_APP		1440															// UDP

