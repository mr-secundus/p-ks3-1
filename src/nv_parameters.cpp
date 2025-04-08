// nv_parameters.cpp
//
// Операции с nvdata - хранение данных в FRAM (блоки параметров устройства)

#include <putx.h>

#include <crc16.h>
#include <data.h>

#include "nvdata/nvdata.h"
#include "cfg/flash_map.h"
#include "nv_parameters.h"

// Адреса блоков параметров в fram
#define ADDR_SETUP1				(kFramAddr_NvParameters)
#define ADDR_SETUP2				(ADDR_SETUP1 + sizeof(TSetup1))
#define ADDR_SETUP3				(ADDR_SETUP2 + sizeof(TSetup2))
#define ADDR_USER_DATA		(ADDR_SETUP3 + sizeof(TSetup3))
#define ADDR_USER_DATA2		(ADDR_USER_DATA + sizeof(TUserNVData))
#define ADDR_SETUP_HT_CONTROL		(ADDR_USER_DATA2 + sizeof(TUserNVData2))
//#define ADDR_SETUP00			(ADDR_SETUP_HT_CONTROL + sizeof(TSetupHtControl))
#define ADDR_SETUP00			(kFramAddr_End - 8 + 1 - sizeof(TSetup00))					// Setup00 в конце FRAM с отступом 8 байт

namespace nv_parameters
{
//-----------------------------------------------------------------------------
//                                   Types
//-----------------------------------------------------------------------------

/*
// Обработка отложенного сохранения блоков в NV память.
class TSaveNvDataControl
{
	static const uint32_t kTmSaveInterval = 25;		// мин. интервал между операциями записи в ЭНП, мс
	static const uint16_t kQueueSize	= 4;				// макс. число запросов в очереди
	static const uint16_t kItemInactive	= 0xFFFF;	// значение id для незанятой ячейки queue
	
	uint16_t 	q[kQueueSize];											// id блоков для сохранения; -1 - ячейка не занята
	uint32_t  timer;															// время последнего выполнения сохранения блока в тиках
	bool			active;															// true - в q находятся невыполненные запросы
	
public:
	TSaveNvDataControl(void)
	{
		active = false;
		timer = 0;
		for(int i=0; i<kQueueSize; i++) q[i] = kItemInactive;
	}

	// return
	//	rcOk			запрос поставлен в очередь, либо успешное выполнение сохранения
	//	rcError 	очередь заполнена
	// остальные коды - см. __saveNVBlock
	rc_t save(uint16_t id)
	{                                  
		if(TIMEOUT(timer, kTmSaveInterval))
		{
			timer = now;
			return __save(id);
		}
		
		for(int i=0; i<kQueueSize; i++)
			if(q[i] == kItemInactive)
			{                                
				q[i] = id;
				active = true;
				return rcOk;
			}
		return rcError;				
	} 

	// 	
	void process(void)
	{                 
		if(active  &&  TIMEOUT(timer, kTmSaveInterval))
		{
			for(int i=0; i<kQueueSize; i++)
				if(q[i] != kItemInactive)
				{                                
					__save(q[i]);
					q[i] = kItemInactive;
					timer = now;
					return;
				}
			active = false;
		}
	}
}; */


//-----------------------------------------------------------------------------
//                                   Data
//-----------------------------------------------------------------------------

const uint32_t kBufferSize		= 176;		// размер локального буфера при обработке NV блоков
																				// выбирается по макс. размеру блока параметров

//TSaveNvDataControl	saveDataControl;		// отложенное сохранение данных
FlashM25LowIo* 			lowIo;							// интерфейс к fram

/*
// Параметры блоков, сохраняемых в NV памяти
using NvBlocksMap = struct tagNvBlocksMap
{
  uint16_t  id,         // id блока - см. defines.h/NV_ID_..
            address,    // линейный байтовый адрес в fram
            size;       // размер блока (crc16?)
	uint8_t*	address;		// адрес структуры в ОЗУ
};

NvBlocksMap nvBlocksMap[NV_BLOCKS_NUM] = 
{
	{ NV_ID_SETUP1, 		ADDR_SETUP1, 			sizeof(TSetup1),  		&setup1},
	{ NV_ID_SETUP2, 		ADDR_SETUP2, 			sizeof(TSetup2),  		&setup2},
	{ NV_ID_SETUP3, 		ADDR_SETUP3, 			sizeof(TSetup3),  		&setup3},
	{ NV_ID_USER_DATA, 	ADDR_USER_DATA, 	sizeof(TUserNVData), 	&userNVData},
	{ NV_ID_USER_DATA2,	ADDR_USER_DATA2, 	sizeof(TUserNVData2), &userNVData2},
	{ NV_ID_SETUP00,		ADDR_SETUP00,			sizeof(TSetup00), 		&setup00}
}; */

// используется при загрузке из ЭНП
uint8_t buffer[kBufferSize] __attribute__((aligned(4)));

//-----------------------------------------------------------------------------
//                                 Static
//-----------------------------------------------------------------------------

// Возвращает битовую маску в R99, соответствующую блоку с указанным id.
// Бит устанавливается в R99 при ошибке загрузки блока, сбрасывается при
// успешном выполнении операции.
uint32_t getBlockMaskRead(uint16_t id)
{
	return 1 << ((id-1)*2);
}

// Возвращает битовую маску в R99, соответствующую блоку с указанным id.
// Бит устанавливается в R99 при ошибке сохранения блока, сбрасывается при
// успешном выполнении операции.
uint32_t getBlockMaskWrite(uint16_t id)
{
	return 2 << ((id-1)*2);
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
		default: return rcError;
		
		case NV_ID_SETUP00:
			*address = ADDR_SETUP00;
			*size = sizeof(TSetup00);
			*p = (uint8_t*)&setup00;
			break;

		case NV_ID_SETUP1:  
			*address = ADDR_SETUP1;
			*size = sizeof(TSetup1);
			*p = (uint8_t*)&setup1;
			break;

		case NV_ID_SETUP2:  
			*address = ADDR_SETUP2;
			*size = sizeof(TSetup2);
			*p = (uint8_t*)&setup2;
			break;

		case NV_ID_SETUP3:
			*address = ADDR_SETUP3;
			*size = sizeof(TSetup3);
			*p = (uint8_t*)&setup3;
			break;

		case NV_ID_USER_DATA:  
			*address = ADDR_USER_DATA;
			*size = sizeof(TUserNVData);
			*p = (uint8_t*)&userNVData;
			break;

		case NV_ID_USER_DATA2:  
			*address = ADDR_USER_DATA2;
			*size = sizeof(TUserNVData2);
			*p = (uint8_t*)&userNVData2;
			break;

		case NV_ID_SETUP_HT_CONTROL:  
			*address = ADDR_SETUP_HT_CONTROL;
			*size = sizeof(TSetupHtControl);
			*p = (uint8_t*)&setupHtControl;
			break;
	}
	return rcOk;
}


//-----------------------------------------------------------------------------
//                                Public
//-----------------------------------------------------------------------------

// Инициализация объекта доступа к интерфейсу fram
void setup(FlashM25LowIo* io)
{
	lowIo = io;

#ifndef NDEBUG
/*
	puts("TSetup1   ");		putd(sizeof(TSetup1));	putc(' '); putd(ADDR_SETUP1);
	puts("\nTSetup2   ");	putd(sizeof(TSetup2));	putc(' '); putd(ADDR_SETUP2);
	puts("\nTSetup3   ");	putd(sizeof(TSetup3));	putc(' '); putd(ADDR_SETUP3);
	puts("\nUserData  ");	putd(sizeof(TUserNVData));	putc(' '); putd(ADDR_USER_DATA);
	puts("\nUserData2 ");	putd(sizeof(TUserNVData2));	putc(' '); putd(ADDR_USER_DATA2);
	puts("\nHtControl ");	putd(sizeof(TSetupHtControl));	putc(' '); putd(ADDR_SETUP_HT_CONTROL);
	puts("\nTSetup00  ");	putd(sizeof(TSetup00));	putc(' '); putd(ADDR_SETUP00);
	puts("\n");
*/
#endif	
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
//	uint8_t buffer[kBufferSize];
	uint8_t *p;
	uint32_t address;
	uint16_t size, crc=nvdata::kInitCrc;
	
	rc_t rc = getAttributes(id, &address, &size, &p);
	if(rc != rcOk) return rc;

	lowIo->fastSetup();
	lowIo->Read(address, buffer, size);

	crc = calcBlockCRC16i(buffer, size-sizeof(uint16_t), crc);

	// Сохраненное crc16 в двух последних байтах блока
	if(crc != *reinterpret_cast<uint16_t*>(&buffer[size-sizeof(uint16_t)]))
	{
		if(opResult != nullptr)
			*opResult |= getBlockMaskRead(id);
		return rcCrcError;
	}

	for(int i=0; i<size; i++) *p++ = buffer[i];
	
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
	uint8_t *p;
	uint32_t address;
	uint16_t size, crc=nvdata::kInitCrc;

	rc_t rc = getAttributes(id, &address, &size, &p);
	if(rc != rcOk) return rc;

	crc = calcBlockCRC16i(p, size-sizeof(uint16_t), crc);

	*(reinterpret_cast<uint16_t*>(&p[size-sizeof(uint16_t)])) = crc;
	
	lowIo->fastSetup();
	lowIo->Write(address, p, size);

	if(opResult != nullptr)
		*opResult &= ~getBlockMaskRead(id);		// успешное завершение - сбросить флаг ошибки
	return rcOk;
}


// Проверка соответствия текущего значения блока NV параметров сохраненным значениям.
//
// id				идентификатор блока
// return		true   1) блок данных изменен; 2) ошибка при чтении блока из NV памяти
//					false  1) блок данных не изменен; 2) некорректный идентификатор блока
bool isModified(uint16_t id)
{ 
	uint8_t buffer[kBufferSize], *p;
	uint32_t address;
	uint16_t size, crc=nvdata::kInitCrc;

	rc_t rc = getAttributes(id, &address, &size, &p);
	if(rc != rcOk) return false;

	lowIo->fastSetup();
	lowIo->Read(address, buffer, size);

	crc = calcBlockCRC16i(buffer, size-sizeof(uint16_t), crc);

	// Сохраненное crc16 в двух последних байтах блока
	if(crc != *reinterpret_cast<uint16_t*>(&buffer[size - sizeof(uint16_t)]))
		return true;

	// Сравнение buffer с исходной структурой, не включая crc16
	for(int i=0; i<size-sizeof(uint16_t); i++)
		if(*p++ != buffer[i])
			return true;

	return false;
}

};		// namespace


