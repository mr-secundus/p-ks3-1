// debug.h
                    
#if !defined __DEBUG_H
#define      __DEBUG_H

#define	DEBUG_INIT							// выносим за NODEBUG

#if !defined	NDEBUG

#include <putx.h>

//	#define DEBUG_TASKS									// вывод инф. при активации/деактивации задач
	
//	#define DEBUG_LOWIO_IN
//	#define DEBUG_LOWIO_OUT
//	#define DEBUG_APPIO_OUT 

//	#define DEBUG_PARAMETERS_RD                                                                                                                          
//	#define DEBUG_PARAMETERS_WR     

//	#define DEBUG_PROTOCOL_MESSAGES			// обмен с ¬”
//	#define DEBUG_PROTOCOL_MESSAGES_LOC	// сообщени€ от узлов на локальной шине

//	#define DEBUG_TX_BUFFERS						// буферизаци€ данных измерени€
	
//	#define DEBUG_EMAC_APP							// EMAC и сетевой стек - уровень приложени€ 
//	#define DEBUG_EMAC_SERVICE					// задача контрол€ состо€ни€ подключени€

//	#define DEBUG_UDP_IO								// UDP low IO
//	#define DEBUG_UDP_RX  							// UDP rx queue
//	#define DEBUG_UDP_TX  							// UDP tx queue
//	#define DEBUG_UDP_RX_APP  					// только размер прин€тых пакетов 
//	#define DEBUG_UDP_TX_APP  					// только размер выходного пакета
//	#define DEBUG_SL_POLL  							// опрос ведомых устройств

//#define SIM_DATA											// имитаци€ блоков данных	
	
#else 
	#ifdef	DEBUG_INIT
		#include <putx.h>
	#endif
#endif	// NDEBUG

#define putlog(s) { putdw(TICKS_TO_MS(now)); puts(s); }
 
#endif	// __DEBUG_H
