#ifndef HAL_UARTS_H
#define HAL_UARTS_H

namespace hal::uarts
{
void init(void);

// Set UART baud
//
// n				UART number 0..4
// baud			baud rate, bit/s
//
// return		The actual baud rate, or 0 if no rate can be found
uint32_t setBaud(uint32_t n, uint32_t baud);

// Write data to UART
//
// n				UART number 0..4
// data			data
// size			data size, bytes 
//
// return		Number of bytes, writen to buffer
uint32_t write(uint32_t n, uint8_t* data, uint8_t size);

// Get free space at output buffer
//
// n				UART number 0..4
//
// return		Free space, number ofbytes
uint32_t getTxFree(uint32_t n);

// Reset output buffer
//
// n				UART number 0..4
void resetTx(uint32_t n);

//
void process(void);
};

#endif
