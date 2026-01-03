// events.h
//
// Идентификаторы сообщений                                         

#include <supervisor/basetask.h>                 
#include "events_app.h"   
                                         
//-----------------------------------------------------------------------------
// группы собщений
/*const TMsgMask grpSysControl			= 0x00000001UL;		// управление системой - команды на выполнение
const TMsgMask grpSlaveControl		= 0x00000002UL;		// управление устройствами
const TMsgMask grpSlaveIO					= 0x00000004UL;		// обмен с устройствами
const TMsgMask grpData						= 0x00000008UL;		// данные измерения
const TMsgMask grpInternal				= 0x00000010UL;		// внутренние сообщения концентратора
const TMsgMask grpRegisterIO			= 0x00000020UL;		// обращения к регистрам
const TMsgMask grpSystem					= 0x10000040UL;		// системный уровень: состояние устройства, ошибки.

const TMsgMask grpControl					= 0x00000080UL;		// управление устройством (вкл/выкл, режимы)

const TMsgMask grpALL							= 0x0FFFFFFFUL;		// отладочный код
*/

const TMsgMask grpRegisterIO			= 0x00000002UL;		// обмен с ВУ - чтение/запись регистров
const TMsgMask grpControl					= 0x00000004UL;		// управление устройством
const TMsgMask grpNet							= 0x00000200UL;		// состояние EMAC и события подключения по Ethernet
const TMsgMask grpNetIo						= 0x00000400UL;		// обмен с ВУ по Ethernet
const TMsgMask grpSlavesIo				= 0x00000800UL;		// обмен с ведомыми устройствами

const TMsgMask grpSystem					= 0x10000010UL;		// системный уровень: состояние устройства, ошибки.

const TMsgMask grpNetState				= 0x00000200UL;		// состояние EMAC и события подключения по сети

const TMsgMask grpALL							= 0x0FFFFFFFUL;		// отладочный код

 
//-----------------------------------------------------------------------------

const short msgRegisterWrite			= 10;
const short msgRegisterRead				= 11;    


const short msgSysInit						= 1101; 			// полная инициализация цустройства
const short msgSysIdle						= 1102; 			// перевод в Idle
const short msgSysFailure					= 1103; 			// неисправность устройства
const short msgSysTSSync					= 1104; 			// выполнена операция синхронизации 
																								// (запись в таймер синхронизации)
																								
const short msgNetInitStart				= 1200; 			// начало инициализации сетевого интерфейса
const short msgNetInitOk					= 1201; 			// успешная инициализация сетевого интерфейса
const short msgNetInitError				= 1202; 			// ошибка инициализации сетевого интерфейса
const short msgNetConnected				= 1203; 			// установлено подключение Ethernet
const short msgNetDisconnected		= 1204; 			// разорвано подключение Ethernet
const short msgNetError						= 1205; 			// аппаратная ошибка сетевого интерфейса
const short msgUDP_InitOk					= 1206; 			// успешная инициализация модуля UDP
const short msgUDP_InitError			= 1207; 			// ошибка инициализации модуля UDP

const short msgNetIoHiRequest		  = 1210; 			// получен запрос от ВУ

const short msgSlaveMsg	  				= 1220; 			// получено сообщение от ведомого устройства







