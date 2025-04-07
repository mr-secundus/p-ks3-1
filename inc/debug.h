// debug.h
                    
#if !defined __DEBUG_H
#define      __DEBUG_H

#define	DEBUG_INIT							// выносим за NODEBUG

#if !defined	NDEBUG

#include <putx.h>

//	#define DEBUG_TASKS									// вывод инф. при активации/деактивации задач
	
//	#define DEBUG_NVDATA_APP
	
//	#define DEBUG_LOWIO_IN
//	#define DEBUG_LOWIO_OUT
//	#define DEBUG_APPIO_OUT 

//	#define DEBUG_PARAMETERS_RD                                                                                                                          
//	#define DEBUG_PARAMETERS_WR     

//	#define DEBUG_PROTOCOL_MESSAGES			// обмен с ВУ
//	#define DEBUG_PROTOCOL_MESSAGES_LOC	// сообщения от узлов на локальной шине

//	#define DEBUG_TX_BUFFERS						// передача данных АЦП
	
	#define DEBUG_EMAC_APP							// EMAC и сетевой стек - уровень приложения 
//	#define DEBUG_EMAC_SERVICE					// задача контроля состояния подключения

//	#define DEBUG_UDP_RX  							// UDP low rx
	#define DEBUG_UDP_TX  							// UDP low tx
//	#define DEBUG_UDP_RX_APP  					// только размер принятых пакетов 
//	#define DEBUG_UDP_TX_APP  					// только размер выходного пакета 

//	#define DEBUG_ADC1B_DATA						// ADC1_Buffers - обработка данных
	
#else 
	#ifdef	DEBUG_INIT
		#include <putx.h>
	#endif
#endif	// NDEBUG

#define putlog(s) { putdw(TICKS_TO_MS(now)); puts(s); }
 
#endif	// __DEBUG_H
