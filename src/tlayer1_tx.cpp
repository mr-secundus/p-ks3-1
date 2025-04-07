// tlayer1_tx.cpp                     

#include "port.h"
#include "hal_uarts.h"
#include "protocol_app/p_thandlers.h"
//#include "protocol.h"


//*****************************************************************************
//                         TLayer1TxInterface
//*****************************************************************************

// Интерфейс передачи в канал связи. Используется в модуле обработки протокола.

// Передача данных из buffer в канал связи
// size     - кол-во байт для передачи
// forceTx  - немедленная передача 
// Возвращает кол-во переданных байт
uint16_t TLayer1TxInterface::write(uint16_t num, uint8_t *buffer, uint16_t size, bool forceTx)
{
	return hal::uarts::write(num, buffer, size);
}

// Возвращает кол-во свободных байт в выходном буфере
uint16_t TLayer1TxInterface::getTxFree(uint16_t num)
{
	return hal::uarts::getTxFree(num);
}

// Сброс выходного буфера
void TLayer1TxInterface::resetTx(uint16_t num)
{
	hal::uarts::resetTx(num);
}


