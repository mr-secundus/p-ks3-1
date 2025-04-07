// dataio_app_const.h
//
// Определение значений подтипов для сообщений DataIO.MsgApp
//
// ВНИМАНИЕ: идентификаторы прикладных блоков данных должны занимать не более 
// 9 младших значащих разрядов - старшие разряды используются как служебные 
// в модуле работы с FLASH накопителем.  
// TODO: переделать модуль работы с FLASH.
//
// 13.09.23  Константы со значениями подтипов MsApp вынесены в файл dataio_app_const.h
// 13.09.23  Boot2 (проект приема прошивки и записи в FLASH) - добавлены типы MsgApp_Boot..
// 22.12.22  УРМ - ПРД. Добавлены структуры TMsgAppStateInfo_9221 TMsgAppStateInfo_5430, а также
//						идентификаторы MsgApp_StateInfo_9221 MsgApp_StateInfo_5430.
// 12.07.22  i54-3-control (Контроллер ПРД) - добавлено MsgApp_DataAbsence2
// 28.04.11  СКВС - добавлен тип MsgApp_DataDevice9400
// 01.06.11  СКВС - добавлено MsgApp_FlashGoBusy           
// 04.06.11  СКВС - исправлена ошибка в вычислении TMsgAppSvData::dataSize - ВАЖНО, исправить в СККВ 
// 07.06.11  СКВС - добавлен тип MsgApp_SvData9400
// 12.01.12  СККВ2: добавлены типы MsgApp_DataEx, MsgApp_SvDataEx, TMsgAppStateInfo_32.
// 31.01.12  СККВ2: MsgApp_SvData переименовано в MsgApp_SvData32, добавлено MsgApp_SvData16.
// 01.10.12  СККВ2: Добавлено MsgApp_Wakeup  
// 23.10.12  iss41: Добавлены идентификаторы MsgApp_Dataint16_tx2 .. MsgApp_Dataint32_tx8,
//                  MsgApp_SvData16x3 MsgApp_SvData32x3
// 17.12.13  iss41: Добавлены идентификаторы MsgApp_SvData16x4, x2, x8 MsgApp_SvData32x4, x2
// 15.01.15  iss7:  В TMsgAppWakeup добавлены два байта для формального выравнивания addr на границу DWORD.
//                  Фаткически это ранее и выполнялось.
// 08.06.15  iss32-i7:  Добавлен TMsgParameterBlockHeader - заголовок сообщения типа "Блок параметров"
// 10.12.19  SK-MLPC1788:	Добавлен TMsgParameterBlockHeader2 - заголовок сообщения типа "Блок параметров мин. вариант"
// 28.04.20  SK-MLPC1788:	В TMsgParameterBlockHeader2 добавлены сервисные функции для доступа к данным
//
//*****************************************************************************

#ifndef DATAIO_APP_CONST_H
#define DATAIO_APP_CONST_H

namespace DataIO
{
//-----------------------------------------------------------------------------
// значения поля type для сообщений прикладного типа
const uint8_t MsgApp_RegBlock							= 0x08;		// M >> S				блок регистров
const uint8_t MsgApp_RegBlock2						= 0x09;		// M >> S				блок регистров минимальный вариант

const uint8_t MsgApp_RequestData					= 0x10;		// M >> S
const uint8_t MsgApp_DataAcknowledge			= 0x11;		// M >> S
const uint8_t MsgApp_DataAbsence					= 0x12;		// M << S
const uint8_t MsgApp_DataAcknowledgeOk		= 0x13;		// M << S
const uint8_t MsgApp_DataAcknowledgeError	= 0x14;		// M << S
const uint8_t MsgApp_RequestStateInfo			= 0x15;		// M >> S
const uint8_t MsgApp_StateInfo						= 0x16;		// M << S
const uint8_t MsgApp_DataAbsence2					= 0x17;		// M << S
const uint8_t MsgApp_Dataint32						= 0x20;		// M << S
const uint8_t MsgApp_Dataint16						= 0x21;		// M << S
const uint8_t MsgApp_DataEx							  = 0x22;		// M << S
const uint8_t MsgApp_RequestDateTime			= 0x30;		// M >> S
const uint8_t MsgApp_DateTime							= 0x31;		// M << >> S
const uint8_t MsgApp_Wakeup								= 0x32;		// M >> S

const uint8_t MsgApp_RequestSvData					= 0x40;		// M >> S
const uint8_t MsgApp_SvDataAcknowledge			= 0x41;		// M >> S
const uint8_t MsgApp_SvDataAbsence					= 0x42;		// M << S
const uint8_t MsgApp_SvDataAcknowledgeOk		= 0x43;		// M << S
const uint8_t MsgApp_SvDataAcknowledgeError	= 0x44;		// M << S
const uint8_t MsgApp_SvDateTime							= 0x45;		// M << S
const uint8_t MsgApp_SvData32								= 0x46;		// M << S
const uint8_t MsgApp_SvDataReadError				= 0x47;		// M << S
const uint8_t MsgApp_FlashGoBusy						= 0x48;		// M << S
const uint8_t MsgApp_SvData16								= 0x49;		// M << S

// Состояние устройства
const uint8_t MsgApp_StateInfo_31						= 0x51;		// M << S
const uint8_t MsgApp_StateInfo_32						= 0x52;		// ДСА-М БИЦ-1 БИЦ-2
const uint8_t MsgApp_StateInfo_9221					= 0x53;		// БИТ-1 БРЭПП
const uint8_t MsgApp_StateInfo_5430					= 0x54;		// ПРД

// идентификаторы сообщений с данными измерения
const uint8_t MsgApp_DataInt16x2	= 0x62;	
const uint8_t MsgApp_DataInt16x3	= 0x63;	
const uint8_t MsgApp_DataUnt16x4	= 0x64;	
const uint8_t MsgApp_DataInt16x6  = 0x66;
const uint8_t MsgApp_DataInt16x8	= 0x68;	
const uint8_t MsgApp_DataInt32x2	= 0x6a;	
const uint8_t MsgApp_DataInt32x3	= 0x6b;	
const uint8_t MsgApp_DataInt32x4	= 0x6c;	
const uint8_t MsgApp_DataInt32x8	= 0x6d;	

// идентификаторы сообщений с данными УХД
const uint8_t MsgApp_SvData16x2		= 0x72;	
const uint8_t MsgApp_SvData16x3		= 0x73;	
const uint8_t MsgApp_SvData16x4		= 0x74;	
const uint8_t MsgApp_SvData16x6   = 0x76;
const uint8_t MsgApp_SvData16x8		= 0x75;	
const uint8_t MsgApp_SvData32x2		= 0x7a;	
const uint8_t MsgApp_SvData32x3		= 0x7b;	
const uint8_t MsgApp_SvData32x4		= 0x7c;	
const uint8_t MsgApp_SvData32x8		= 0x7d;	

// Данные от ДСУx: Сила + угол1 + угол2  (int32 + int16 + int16)
const uint8_t MsgApp_DataDevice9400					= 0x81;		
const uint8_t MsgApp_SvData9400							= 0x82;

//
// Сообщения, используемые модулем BootSender.
// Подробности в проекте DataIOTool/BootSender.cs
//
// Контейнер для передачи различных команд, содержащих небольшой объем данных.
// Блок данных MsgAppType.data[] представляется как массив uint32_t_t[] msgData.
const uint16_t MsgApp_BootService  = 0x100;
// Данные для записи в буфер RAM
const uint16_t MsgApp_BootRamData  = 0x101;
	// Ответ от устройства на команду или блок данных
const uint16_t MsgApp_BootAck      = 0x102;

//
// Кодированые идентификаторы для многоканальных блоков данных
// MsgApp_Dataint16_txBase - базовое значение идентификатора
// 
// id = MsgApp_Dataint16_txBase + N  
//
// N - количество каналов
//
const uint16_t MsgApp_DataInt16xBase	= 0x200;		// данные int16
const uint16_t MsgApp_DataInt32xBase	= 0x300;		// данные int32
};		// namespace

#endif
