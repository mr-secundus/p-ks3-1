// data.h
//

#ifndef DATA_H
#define DATA_H

#include <port.h>
#include "cfg/defines.h"
#include "cfg/protocol.h"


// Определение состояния ошибки загрузки/сохранения блоков парамтеров в NV памяти
//
// block 		идентификатор блока - см. NV_ID_SETUP..
#define NV_BLOCK_RD_ERROR(block)  (R99 & (1 <<((block-1)*2)) ? true : false)
#define NV_BLOCK_WR_ERROR(block)  (R99 & (2 <<((block-1)*2)) ? true : false)


//-----------------------------------------------------------------------------
// Сохраняемые параметры устройства
//
// ВНИМАНИЕ: при сохранении структур в ЭНП, в последние два байта записывается CRC16.
//           Т.о., структура должна содержать выделенные два байта под CRC.

// Setup1  - информация об устройстве
using TSetup1 = struct setup1_t 
{
	uint32_t	serialNumber;						// серийный номер
	uint16_t	reserved[5];
	uint16_t	crc16;

	setup1_t(void) 
	{
		serialNumber = DEFAULT_SERIAL_NUMBER;

		for(uint32_t i = 0; i < sizeof(reserved) / sizeof(reserved[0]); i++)
			reserved[i] = 0xFFFF;
	}		
};


// Setup2  - сетевые настройки
using TSetup2 = struct setup2_t 
{
	uint16_t  netAddrHi,							// сетевой адрес для связи с ВУ 
						netAddrRs485_1,					// сетевой адрес RS485-1
						netAddrRs485_2,					// сетевой адрес RS485-2
						res0;
	uint32_t	baudRs232,							// RS232 baud  
						baudRs485_1,						// RS485-1 (X4/X5) ISO baud
						baudRs485_2;						// RS485-2 (X6) baud
	uint32_t	IP_HostAddress,					// IP адрес устройства
						IP_GatewayAddress,			// Gateway IP
						IP_SubnetMask;					// маска подсети
	uint16_t	localPort,							// локальный порт UDP для приема входящих пакетов
						destPort;								// удаленный порт UDP для отправки пакетов
	uint16_t	reserved[5]; 
	uint16_t	crc16;

	setup2_t(void) 
	{
		netAddrHi				= DEFAULT_NET_ADDRESS_HI_IO;
		netAddrRs485_1	= DEFAULT_NET_ADDRESS_LINK1;
		netAddrRs485_2	= DEFAULT_NET_ADDRESS_LINK2;
		baudRs232				= RS232_BAUD;
		baudRs485_1			= RS485_1_BAUD;
		baudRs485_2			= RS485_2_BAUD;
		
		crc16 = 0xFFFF;
		
		for(uint32_t i = 0; i < sizeof(reserved) / sizeof(reserved[0]); i++)
			reserved[i] = 0xFFFF;
	}		
};


// Setup3  - адреса ведомых устройств для LINK1
using TSetup3 = struct setup3_t 
{
	uint16_t	slaves[LINK1_SLAVES_N];
	uint16_t	reserved[6];
	uint16_t	crc16;

	setup3_t(void) 
	{
		for(uint32_t i = 0; i < sizeof(slaves) / sizeof(slaves[0]); i++)
			slaves[i] = 0;

		for(uint32_t i = 0; i < sizeof(reserved) / sizeof(reserved[0]); i++)
			reserved[i] = 0xFFFF;
	}		
};

//-----------------------------------------------------------------------------
//
// Состояние устройства
//
/*typedef struct tagDeviceState
{
  uint32_t
    isMeasOn          : 1,    // идет измерение
    b1                : 1,
    b2                : 1,
    isSensorError     : 1,    // неисправность датчика
    isMeasStart       : 1,    // выполняется старт измерения
    isSyncOk          : 1,    // выполнялась синхронизация (запись в R21) - с ВУ или восст.
    wasSyncRecovery   : 1,    // выполнялось восстановление синхронизации после
                              // powerdown, при этом isSyncOk устанавливается
    startReason       : 4,    // причина включения:
                              // 1 Uext
                              // 2 кнопка
                              // 3 радиоканал
                              // 4 RTC
                              // 5 PowerOnReset
                              // 6 WDT
                              // 7 BOD
                              // 8 SYSRST
    batteryTFail      : 1,    // выход Tакк за пределы - копирует powerState.State.batteryTFail
    isDelayedSleep    : 1,    // выполняется отложенный переход в спящий режим
    b13                 : 1,  // дискретный вход
    b14                 : 1,  // дискретный вход
    b15                 : 1,  
    bSensor2Error       : 1,  // неисправность контрольного акселерометра
    battery             : 2,  // состояние аккумулятора: 0/1/2/3 Ok/Low/Discharge/Ext. power
    b19                 : 1,
    accOverX            : 1,  // выход за пределы измерения акселерометтра, канал X
    accOverY            : 1,  // --//--//-- канал Y
    accOverZ            : 1,  // --//--//-- канал Z
    batteryUFail        : 1,
    reserved            : 8;

  inline uint32_t getU32(void) { return *((uint32_t*)this); }
  
  tagDeviceState(void)
  {
    *((uint32_t*)this) = 0;
  }
} TDeviceState;
*/

//
// Флаги для внутреннего использования
//
typedef struct tagServiceFlags
{
  uint32_t
    enableRxMsgProcess  		: 1,    // разрешена обработка входящих сообщений
    cmdAcceptAddrLinkHi   	: 1,    // требуется применение сетевого адреса для LinkHi
    cmdAcceptAddrLink1   		: 1,    // требуется применение сетевого адреса для Link1
    cmdAcceptAddrLink2   		: 1,    // требуется применение сетевого адреса для Link2
    cmdAcceptRs485Baud  		: 1,    // требуется применение RS485 baud
    reserved            		: 11;

//  inline uint32_t getU16(void) { return *((uint16_t*)this); }
} TServiceFlags;


//-----------------------------------------------------------------------------
//    Данные
//-----------------------------------------------------------------------------

extern TSetup1		setup1;
extern TSetup2		setup2;
extern TSetup3		setup3;

//extern TDeviceState     deviceState;
extern TServiceFlags    serviceFlags;

extern uint32_t R14, R15, R19, R99;

#endif
