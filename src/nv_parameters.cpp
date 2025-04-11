// nv_parameters.cpp
//
// Операции с блоками параметров, сохраняемыми в NV памяти
//
// Для хранения используется встроенная EEPROM МК. Стирание EEPROM выполняется
// постранично, поэтому каждый блок параметров размещается в отдельной странице.
//
// 1) Размер блока параметров не должен превышать размер страницы EEPROM - 128 байт.
// 2) Операции с EEPROM выполняются словами по 4 байта -> размер блоков параметров
//    должен быть кратен 4.
// 3) В двух последних байтах блока размещается CRC16.


#include <putx.h>
#include <crc16.h>
#include "data.h"
#include "nv_parameters.h"

// Номера страниц EEPROM для блоков параметров 
#define PG_SETUP1				0
#define PG_SETUP2				1
#define PG_SETUP3				2

namespace nv_parameters
{
//-----------------------------------------------------------------------------
//                                   Types
//-----------------------------------------------------------------------------

// Используется при загрузке из ЭНП
uint32_t buffer[EEPROM_PAGE_SIZE];


//-----------------------------------------------------------------------------
//                                 Static
//-----------------------------------------------------------------------------

// Возвращает битовую маску в R99, соответствующую блоку с указанным id.
// Бит устанавливается в R99 при ошибке загрузки блока, сбрасывается при
// успешном выполнении операции.
uint32_t getBlockMaskRead(uint16_t id)
{
	return 1 << ((id - 1) * 2);
}

// Возвращает битовую маску в R99, соответствующую блоку с указанным id.
// Бит устанавливается в R99 при ошибке сохранения блока, сбрасывается при
// успешном выполнении операции.
uint32_t getBlockMaskWrite(uint16_t id)
{
	return 2 << ((id - 1) * 2);
}


// Получение параметров для операций с блоком NV параметров
//
// id 		     	идентификатор запрашиваемого блока - см. defines.h/NV_ID_..
// address      адрес блока данных в fram
// size         размер блока, байт
// p            адрес для загрузки данных
//
// return     	rcOk rcError
rc_t getAttributes(uint16_t id, uint32_t* address, uint16_t* size, uint8_t** p)
{
	switch(id)
	{
		default:
			return rcError;
		
		case NV_ID_SETUP1:  
			*address = EEPROM_ADDRESS(PG_SETUP1, 0);
			*size = sizeof(TSetup1);
			*p = (uint8_t*)&setup1;
			break;

		case NV_ID_SETUP2:  
			*address = EEPROM_ADDRESS(PG_SETUP2, 0);
			*size = sizeof(TSetup2);
			*p = (uint8_t*)&setup2;
			break;

		case NV_ID_SETUP3:
			*address = EEPROM_ADDRESS(PG_SETUP3, 0);
			*size = sizeof(TSetup3);
			*p = (uint8_t*)&setup3;
			break;
	}
	return rcOk;
}


//-----------------------------------------------------------------------------
//                                Public
//-----------------------------------------------------------------------------

// Инициализация периферии, используемой для хранения NV параметров
void init(void)
{
	Chip_EEPROM_Init(LPC_EEPROM);
	Chip_EEPROM_SetAutoProg(LPC_EEPROM, EEPROM_AUTOPROG_AFT_1WORDWRITTEN);
}


// Загрузка блока параметров из NV памяти
//
// id						идентификатор блока - см. defines.h/NV_ID_..
// opResult			указатель на переменную, в которой устанавливается (сбрасывается)
//							бит при ошибке (успешном выполнении) операции - см. описание R99
//
// return		rcOk rcError rcCrcError
rc_t load(uint16_t id, uint32_t* opResult)
{
	uint32_t *p;
	uint32_t address;
	uint16_t size, crc = 0xFFFF;
	
	rc_t rc = getAttributes(id, &address, &size, (uint8_t**)&p);
	if(rc != rcOk) return rc;

	// Чтение из EEPROM в buffer
	uint32_t *pEeprom = (uint32_t*)address;
	for(int i = 0; i < size/4; i++)
		buffer[i] = pEeprom[i];

	crc = calcBlockCRC16i((uint8_t*)buffer, size - sizeof(uint16_t), crc);

	// Сохраненное crc16 в двух последних байтах блока
	if(crc != *(uint16_t*)((uint8_t*)buffer + size - sizeof(uint16_t)))
	{
		if(opResult != nullptr)
		{
			*opResult |= getBlockMaskRead(id);
			return rcCrcError;
		}
	}

	// Скопировать из bufer в блок параметров
	for(int i = 0; i < size/4; i++, p++)
		*p = buffer[i];
	
	if(opResult != nullptr)
		*opResult &= ~getBlockMaskRead(id);	// успешное завершение - сбросить флаг ошибки
	
	return rcOk;
}


// Сохранение блока параметров в NV память
//
// id						идентификатор блока - см. defines.h/NV_ID_..
// opResult			указатель на переменную, в которой устанавливается (сбрасывается)
//							бит при ошибке (успешном выполнении) операции - см. описание R99
//
// return		rcOk rcInvalidData
rc_t save(uint16_t id, uint32_t* opResult)
{
	uint32_t *p;
	uint32_t address;
	uint16_t size, crc = 0xFFFF;

	rc_t rc = getAttributes(id, &address, &size, (uint8_t**)&p);
	if(rc != rcOk) return rc;

	crc = calcBlockCRC16i((uint8_t*)p, size - sizeof(uint16_t), crc);

	*(uint16_t*)((uint8_t*)p + size - sizeof(uint16_t)) = crc;

	uint32_t *pEeprom = (uint32_t*)address;

	for(int i = 0; i < size/4; i++)
	{
		// Erase EEPROM
		pEeprom[i] = 0;
		Chip_EEPROM_WaitForIntStatus(LPC_EEPROM, EEPROM_INT_ENDOFPROG);
		// Write to EEPROM
		pEeprom[i] = *p++;
		Chip_EEPROM_WaitForIntStatus(LPC_EEPROM, EEPROM_INT_ENDOFPROG);
	}

	if(opResult != nullptr)
		*opResult &= ~getBlockMaskRead(id);		// успешное завершение - сбросить флаг ошибки
	return rcOk;
}

};		// namespace

