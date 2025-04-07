// udp_io_cfg.h


// Размеры полей сетевого пакета
#define NET_ETH_PAYLOAD				1500							// Ethernet frame payload
#define NET_IPH_LEN       		  20  						// IP header IPv4
//#define NET_IPH_LEN       	  40  						// IP header IPv6
#define NET_UDPH_LEN       	 	   8    					// UDP header

// Настройки размеров буферов

// Макс. размер прикладных данных в буфере
#define UDP_PAYLOAD		((NET_ETH_PAYLOAD) - (NET_UDPH_LEN + NET_IPH_LEN))

// Размер буфера - только прикладные данные
//#define UDP_DATA_MAX_SZ		(UDP_PAYLOAD)		// = 1472
#define UDP_DATA_MAX_SZ		1440

// Количество буферов для принятых и отправляемых сообщений
#define UDP_RX_Q_SIZE		4	
#define UDP_TX_Q_SIZE		4					

