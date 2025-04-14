// hal_uarts.h

#include "port.h"
#include "hal.h"
#include "cfg/defines.h"
#include "cfg/protocol.h"
#include "protocol/protocol.h"

extern "C" {
#include <putx.h>
#include <constrinput.h>
}


namespace hal::uarts
{
	#define UART_RS232		LPC_USART3
	#define UART_RS485		LPC_UART1
	
	uint8_t			raw1tx[UART1_TX_SIZE], raw1rx[UART1_RX_SIZE];
	RINGBUFF_T	txBuffer1, rxBuffer1;

	uint8_t			raw3tx[UART3_TX_SIZE], raw3rx[UART3_RX_SIZE];
	RINGBUFF_T	txBuffer3, rxBuffer3;
	
	constexpr RINGBUFF_T	*txBuffer_Rs232 = &txBuffer3;  
	constexpr RINGBUFF_T	*rxBuffer_Rs232 = &rxBuffer3;  
	constexpr RINGBUFF_T	*txBuffer_Rs485 = &txBuffer1;  
	constexpr RINGBUFF_T	*rxBuffer_Rs485 = &rxBuffer1;
	
	bool 	u1Last 	= false;		// флаг передачи последнего фиктивного байта по UART1 для управления
														// драйвером RS485

/*	
#ifdef UART0_TX_SIZE
uint8_t			raw0tx[UART0_TX_SIZE], raw0rx[UART0_RX_SIZE];
RINGBUFF_T	txBuffer0, rxBuffer0;
#endif

#ifdef UART1_TX_SIZE
uint8_t			raw1tx[UART1_TX_SIZE], raw1rx[UART1_RX_SIZE];
RINGBUFF_T	txBuffer1, rxBuffer1;
#endif
*/
};		// namespace hal::uarts


///-----------------------------------------------------------------------------
//                       Организация вывода на STDOUT
//-----------------------------------------------------------------------------

// вывод символа на stdout
extern "C" void __putc(const char c)
{
  const uint8_t d = c;
  while(Chip_UART_SendRB(UART_RS232, hal::uarts::txBuffer_Rs232, &d, 1) == 0);
}

// вывод строки на stdout
extern "C" void __puts(const char* s)
{
  uint16_t n, size;
  uint8_t *p;
  for(size=0, p=(uint8_t*)s; *p; size++) p++;
  p = (uint8_t*)s;

  while(size)
  {
    n = Chip_UART_SendRB(UART_RS232, hal::uarts::txBuffer_Rs232, (const uint8_t *)p, size);
    p += n; size -= n;
  }
  return;
}


//-----------------------------------------------------------------------------
//                                  ISR
//-----------------------------------------------------------------------------

extern "C" void UART1_IRQHandler(void)
{
//	Chip_UART_IRQRBHandler(LPC_UART1, &hal::uarts::rxBuffer1, &hal::uarts::txBuffer1);

	// Handle transmit interrupt if enabled
/*	if(LPC_UART1->IER & UART_IER_THREINT) 
	{
		Chip_UART_TXIntHandlerRB(LPC_UART1, &hal::uarts::txBuffer1);

		// Disable transmit interrupt if the ring buffer is empty
		if(RingBuffer_IsEmpty(&hal::uarts::txBuffer1)) 
			Chip_UART_IntDisable(LPC_UART1, UART_IER_THREINT);
	} */

	if(LPC_UART1->IIR & UART_IIR_INTID_THRE) 
	{
		if(hal::uarts::u1Last)		// передавался последний (фиктивный) байт
		{
			hal::uarts::u1Last = false;
			hal::setUart1dir(false);

			// Shut down transmit
			Chip_UART_IntDisable(LPC_UART1, UART_IER_THREINT);
		}
		else
		{
			uint8_t c;
	
			// Fill FIFO until full or until TX ring buffer is empty
			while ((Chip_UART_ReadLineStatus(LPC_UART1) & UART_LSR_THRE) != 0  &&
					 RingBuffer_Pop(&hal::uarts::txBuffer1, &c)) 
			{
				Chip_UART_SendByte(LPC_UART1, c);
			}
			
			// Turn off interrupt if the ring buffer is empty
			if(RingBuffer_IsEmpty(&hal::uarts::txBuffer1)) 
			{
				// Shut down transmit
//				Chip_UART_IntDisable(LPC_UART1, UART_IER_THREINT);
				
				// После передачи данных передать ещё один фиктивный байт, после которого 
				// будет выключен драйвер RS485
				hal::uarts::u1Last = true;
				Chip_UART_SendByte(LPC_UART1, 0xFF);
			}
		}
	}
	else		// Handle receive interrupt
		Chip_UART_RXIntHandlerRB(LPC_UART1, &hal::uarts::rxBuffer1);
}

extern "C" void UART3_IRQHandler(void)
{
	Chip_UART_IRQRBHandler(LPC_USART3, &hal::uarts::rxBuffer3, &hal::uarts::txBuffer3);
}


namespace hal::uarts
{
//-----------------------------------------------------------------------------
//                                Private
//-----------------------------------------------------------------------------

//
void setupUart(LPC_USART_T* uart, IRQn_Type irqN, uint32_t irqPri, uint32_t baud)
{
	Chip_UART_Init(uart);
	Chip_UART_SetBaud(uart, baud);
	Chip_UART_ConfigData(uart, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT));

//	Chip_UART_SetupFIFOS(LPC_UART1, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	// Reset and enable FIFOs, FIFO trigger level 3 (14 chars) 
	Chip_UART_SetupFIFOS(uart, (UART_FCR_FIFO_EN | UART_FCR_RX_RS | UART_FCR_TX_RS | UART_FCR_TRG_LEV3));

	// Enable receive data and line status interrupt
	Chip_UART_IntEnable(uart, (UART_IER_RBRINT | UART_IER_RLSINT));
	Chip_UART_TXEnable(uart);
	
	NVIC_SetPriority(irqN, irqPri);
	NVIC_EnableIRQ(irqN);
}

//-----------------------------------------------------------------------------
//                                Public
//-----------------------------------------------------------------------------

//
void init(void)
{
	RingBuffer_Init(&rxBuffer1, raw1rx, 1, UART1_RX_SIZE);
	RingBuffer_Init(&txBuffer1, raw1tx, 1, UART1_TX_SIZE);
	RingBuffer_Init(&rxBuffer3, raw3rx, 1, UART3_RX_SIZE);
	RingBuffer_Init(&txBuffer3, raw3tx, 1, UART3_TX_SIZE);

	setupUart(LPC_UART1,  UART1_IRQn,  2, UART1_BAUD);
	setupUart(LPC_USART3, USART3_IRQn, 6, UART3_BAUD);
	
//	Chip_UART_SetRS485Flags(LPC_UART1, UART_RS485CTRL_DCTRL_EN | UART_RS485CTRL_OINV_1);
//	Chip_UART_SetRS485Delay(LPC_UART1, 200);
}


// Set UART baud
//
// n				UART number 0..4
// baud			baud rate, bit/s
//
// return		The actual baud rate, or 0 if no rate can be found
uint32_t setBaud(uint32_t n, uint32_t baud)
{
	if(n >= 0  &&  n <= 3)
	{
	  LPC_USART_T* uarts[] = {LPC_USART0, LPC_UART1, LPC_USART2, LPC_USART3};
		return Chip_UART_SetBaud(uarts[n], baud);
	}
	else return 0;
}


// Write data to UART
//
// n				UART number 0..4
// data			data
// size			data size, bytes 
//
// return		Number of bytes, writen to buffer
uint32_t write(uint32_t n, uint8_t* data, uint8_t size)
{
	if(n == 1)
	{
		hal::setUart1dir(true);
		uint32_t n = Chip_UART_SendRB(LPC_UART1, &txBuffer1, data, size);
		if(n == 0)
			hal::setUart1dir(false);
		return n;
	}
	else if(n == 3)		return Chip_UART_SendRB(LPC_USART3, &txBuffer3, data, size);
	else
		return 0;
}


// Get free space at output buffer
//
// n				UART number 0..4
//
// return		Free space, number ofbytes
uint32_t getTxFree(uint32_t n)
{
	if(n == 1)				return RingBuffer_GetFree(&txBuffer1);
	else if(n == 3)		return RingBuffer_GetFree(&txBuffer3);
	else
		return 0;
}


// Reset output buffer
//
// n				UART number 0..4
void resetTx(uint32_t n)
{
	if(n == 1)				RingBuffer_Flush(&txBuffer1);
	else if(n == 3)		RingBuffer_Flush(&txBuffer3);
}


//
void process(void)
{
	const int kMaxRxSz = 64;
	
  uint8_t data[kMaxRxSz];
  
  int n = Chip_UART_ReadRB(UART_RS232, rxBuffer_Rs232, &data[0], kMaxRxSz);

  if(n > 0)
    for(int i = 0; i < n; i++)
    {
    	cprocbyte(data[i]);			// консоль
    }

	n = Chip_UART_ReadRB(UART_RS485, rxBuffer_Rs485, &data[0], kMaxRxSz);
	
	if(n > 0)
	{
//		for(int i = 0; i < n; i++)  putc(data[i]);		// RS485 -> консоль
		
		Chip_UART_SendRB(UART_RS485, txBuffer_Rs485, data, n);		// эхо
		
		int nn = RingBuffer_PopMult(rxBuffer_Rs485, data, n);
		
		TBuffer b(data, nn);
		
		Protocol::processInputData(&b, nn, LINK_ID_LOCAL);
	}
}

};		// namespace hal::uarts
