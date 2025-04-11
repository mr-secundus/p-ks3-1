// udp_io.cpp
//
// Прием и передача данных по UDP
//
//-----------------------------------------------------------------------------
// Процедура установления соединения с устройством
//
// 1. ВУ посылает сообщение на BroadcastAddress:Port_config.
// 2. Устройство принимает сообщение на BroadcastAddress:Port_config и 
//    <создает  временное соединение udp_conn_tx_temp на адрес отправителя данного сообщения и Port_config>
//    запоминает адрес отправителя сообщения RemoteAddr.
// 3. При передаче выходного пакета (был корретктный запрос с использованием DataIO)
//    создаются udp_conn_tx и udp_conn_rx на RemoteAddr.
//    Если udp_conn_tx и udp_conn_rx существуют, они уничтожаются и создаются заново.
//
// Вопросы и замечания:
// - ситуация, когда отправитель Broadcast сообщения только выполняет поиск устройств
//   без продолжения работы.
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <string.h>

#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/udp.h"

#include <putx.h>

#include "hal.h"
#include "debug.h"
#include "udp_io/udp_io.h"
#include "udp_io/udp_io_cfg.h"


//-----------------------------------------------------------------------------
//                                 Defines
//-----------------------------------------------------------------------------

// Извлечение октетов из uint32_t
#define U32B3(u)		(((u) >> 24) & 0xFF)
#define U32B2(u)		(((u) >> 16) & 0xFF)
#define U32B1(u)		(((u) >>  8) & 0xFF)
#define U32B0(u)		((u) & 0xFF)

// Общее число буферов данных
//#define UDP_BUFFERS_N					(UDP_TX_Q_SIZE + UDP_RX_Q_SIZE)

// Буферизуются только исходящие данные. 
// Для входящих хранятся ссылки на полученный при приеме pbuf.   
#define UDP_BUFFERS_N					(UDP_TX_Q_SIZE)				


//-----------------------------------------------------------------------------
//                              Static data
//-----------------------------------------------------------------------------

namespace udp_io
{
uint16_t 	lport = 0,								// локальный порт для входящих сообщений
					rport = 0;								// удаленный порт, на который отправляются сообщения

// Передача исходящих сообщений
struct udp_tx_data_t
{
	struct udp_pcb 	*upcb;
	struct pbuf 		*pbuffer;
	ip_addr_t 			raddr;						// адрес отправителя из ранее принятого сообщения - 
																		// используется для создания исходящего соединения
	bool 	connected,									// для upcb выполнено connect к remoteAddr
				update;											// требуется переподключение после изменения remoteAddr
	
	udp_tx_data_t()
	{
		pbuffer = NULL;
		upcb = NULL;
    IP4_ADDR(&raddr, 0, 0, 0, 0);
		connected = update = false;
	}
} txData;

bool inited = false;

//-----------------------------------------------------------------------------

// Буферизация входных/выходных данных UDP


// Очереди принятых и отправляемых сообщений

uint8_t txQueueBuffer [UDP_TX_Q_SIZE * sizeof(UdpData)];
uint8_t rxQueueBuffer [UDP_RX_Q_SIZE * sizeof(UdpData)];

TQueue<UdpData> txQueue(txQueueBuffer, UDP_TX_Q_SIZE);
TQueue<UdpData> rxQueue(rxQueueBuffer, UDP_RX_Q_SIZE);


// Буферы нижнего уровня

// raw блок для размещения буферов  --> RamLoc40
uint8_t udpRAW_Buffer[(UDP_DATA_MAX_SZ) * (UDP_BUFFERS_N)] __attribute__((section(".data.$RAM2")));

uint8_t *udpPBuffers[UDP_BUFFERS_N];	// указатели на буферы пула
	
uint8_t udpPState[UDP_BUFFERS_N];			// состояние буферов пула

// пул буферов для передачи исходящих данных
TBuffersPool udpBuffers(udpPBuffers, udpPState, UDP_BUFFERS_N);

};	// namespace


//-----------------------------------------------------------------------------
//                             UDP callback
//-----------------------------------------------------------------------------

/**
  * @brief This function is called when an UDP datagrm has been received on the port UDP_PORT.
  * @param arg user supplied argument (udp_pcb.recv_arg)
  * @param pcb the udp_pcb which received data
  * @param p the packet buffer that was received
  * @param addr the remote IP address from which the packet was received
  * @param port the remote port from which the packet was received
  * @retval None
  */
void udp_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t *addr, u16_t port)
{
#ifdef DEBUG_UDP_IO
puts(">> udp_rcv len:"); putd(p->len); putc('\n');

static char tmp_buff[16];
char sbuf[96];                        
sprintf(sbuf, " adr:%s/%d\n",
				ipaddr_ntoa_r((const ip_addr_t*)addr, tmp_buff, 16), port);
puts(sbuf);
sprintf(sbuf, " loc:%s/%d\n",
				ipaddr_ntoa_r((const ip_addr_t*)&upcb->local_ip, tmp_buff, 16), upcb->local_port);
puts(sbuf);
sprintf(sbuf, " rem:%s/%d\n",
				ipaddr_ntoa_r((const ip_addr_t*)&upcb->remote_ip, tmp_buff, 16), upcb->remote_port);
puts(sbuf);
sprintf(sbuf, " arg:%lx upcb:%lx flags:%x recv:%lx p:%lx\n\n",
				(uint32_t)arg, (uint32_t)upcb, upcb->flags, (uint32_t)upcb->recv, (uint32_t)p);
puts(sbuf);
#endif

/*
  // echo
	udp_connect(upcb, addr, 4023);
	udp_send(upcb, p);
	udp_disconnect(upcb);
	pbuf_free(p);
	return; */

	// Если пришел пакет с нового адреса, или udp_conn_tx_data еще не создано
	if(udp_io::txData.raddr.addr != addr->addr  ||  !udp_io::txData.connected)
	{
		// запомнить адрес отправителя пакета
		udp_io::txData.raddr.addr = addr->addr;
		
		// при дальнейшей передаче выполнить connect
		udp_io::txData.update = true;							
	}

	// Создать объект с указателем на данные и доп. информацией.
	// p используется при освобождении буфера
	UdpData data((uint8_t*)p->payload, p->len, nullptr, (void*)p);

	udp_io::rxQueue.push(&data);
		
	// буфер освобождается в processRxQueue() после обработки данных 
//	pbuf_free(p);
}


//
void udp_receive_callback2(void *arg, struct udp_pcb *upcb, struct pbuf *p, ip_addr_t *addr, u16_t port)
{
#ifdef DEBUG_UDP_IO
puts(">> udp_rcv2 len:"); putd(p->len); puts("\n");

static char tmp_buff[16];
char sbuf[96];                        
sprintf(sbuf, " adr:%s/%d\n",
				ipaddr_ntoa_r((const ip_addr_t*)addr, tmp_buff, 16), port);
puts(sbuf);
/*sprintf(sbuf, " loc:%s/%d\n",
				ipaddr_ntoa_r((const ip_addr_t*)&upcb->local_ip, tmp_buff, 16), upcb->local_port);
puts(sbuf);
sprintf(sbuf, " rem:%s/%d\n",
				ipaddr_ntoa_r((const ip_addr_t*)&upcb->remote_ip, tmp_buff, 16), upcb->remote_port);
puts(sbuf); */
sprintf(sbuf, " arg:%lx upcb:%lx flags:%x recv:%lx p:%lx\n\n",
				(uint32_t)arg, (uint32_t)upcb, upcb->flags, (uint32_t)upcb->recv, (uint32_t)p);
puts(sbuf);
#endif

//	pbuf_free(p);
}


//-----------------------------------------------------------------------------
//                               Public
//-----------------------------------------------------------------------------

namespace udp_io
{
//                                                                 
// Инициализация модуля
//
// alport		local port
// arport		remote port
//
// return		kRcOk, kRcNetErr,	kRcAllocErr

//
int16_t init(uint16_t alport, uint16_t arport)
{                                    
	inited = false;

	lport = alport;
	rport = arport;
	
	for(int i = 0; i < UDP_BUFFERS_N; i++)
		udpPBuffers[i] = &udpRAW_Buffer[UDP_DATA_MAX_SZ * i];


	// Схема работы:
	// Первый пакет принимается с любого адреса на LPORT.
	// Далее создается connection с адресом отправителя для передачи ответных сообщений.
	// При приеме пакета с другого адреса, соединение закрывается, создается на новый адрес.
	// 
	// Static connection:
	// 1) FF.FF.FF.FF/DestPort=LPORT    - прием на broadcast адрес при поиске устройств
	// 2) 00.00.00.00/DestPort=LPORT    - прием от ведущего

	// Инициализация элементов, используемых при передаче.
	// pbuffer размещается статически. При передаче в pbuffer копируеются данные для передачи.
	// upcb размещается статически. При получении входящего сообщения, адрес отправителя
	// запоминается в raddr. При последующей передаче, если raddr был изменен, для upcb выполняется 
	// connect на raddr.
	
	txData.pbuffer = pbuf_alloc(PBUF_TRANSPORT, UDP_DATA_MAX_SZ, PBUF_POOL);
	
	if(txData.pbuffer == NULL)
		return kRcAllocErr;
	
	txData.upcb = udp_new();

	if(txData.upcb == NULL)
	{
		pbuf_free(txData.pbuffer);
		return kRcAllocErr;
	}
	

	struct udp_pcb *upcb = udp_new();

	if(upcb == NULL)
		return kRcAllocErr;

	if(udp_bind(upcb, IP_ADDR_ANY, lport) == ERR_OK)
	{
		udp_recv(upcb, udp_receive_callback, NULL);
		inited = true;
		return kRcOk;
	}
	else
	{
		udp_remove(upcb);
		pbuf_free(txData.pbuffer);
		udp_remove(txData.upcb);
		return kRcNetErr;
	}
}


//
//
//
void process(void)
{                  
	if(inited)
	{
	}
}       


//-----------------------------------------------------------------------------
//
// Передача пакета
//
// return		размер переданного пакета, байт
//					0 при ошибке
//
uint32_t send(UdpData *data)
{ 
#ifdef DEBUG_UDP_IO
	static char tmp_buff[16];
	char sbuf[96];                        
#endif
	
	err_t rc;
	
//	if(/*txData.pbuffer != NULL  &&*/  txData.upcb != NULL  &&  inited)
	if(inited)
	{
		if(txData.update)
		{
			if(txData.connected)
			{
				udp_disconnect(txData.upcb);
				txData.connected = false;
			}

			rc = udp_connect(txData.upcb, &txData.raddr, rport);
			if(rc != ERR_OK)
			{
#ifdef DEBUG_UDP_IO
				puts(" connect err:"); putd(rc);
#endif				
				return 0;
			}
			
      udp_recv(txData.upcb, udp_receive_callback2, NULL);  
			
			txData.connected = true;
			txData.update = false;

#ifdef DEBUG_UDP_IO
			sprintf(sbuf, " conn:%s/%d\n", ipaddr_ntoa_r((const ip_addr_t *) &txData.raddr, tmp_buff, 16),  rport);
			puts(sbuf);
#endif			
		}

		if(txData.connected)
		{
			txData.pbuffer->tot_len = txData.pbuffer->len = data->size;		// hack 
			pbuf_take(txData.pbuffer, data->buffer, data->size);

			rc = udp_send(txData.upcb, txData.pbuffer);
			
#ifdef DEBUG_UDP_IO
			puts(" send sz:"); putd(data->size); 

			sprintf(sbuf, " dest:%s/%d", ipaddr_ntoa_r((const ip_addr_t *) &txData.raddr, tmp_buff, 16),  rport);
			puts(sbuf);

			if(rc != ERR_OK)
			{
				puts("  error:"); putd(rc);
			}
			puts("\n");
#endif			
			
			if(rc == ERR_OK)
				return data->size;
		}
	}
	return 0;
}

}			// namespace    
