//*****************************************************************************
// dataio_types.h
//
// Описание идентификаторов, кодов результата операции и типов протокола DataIO
//
//*****************************************************************************
//
// 15.11.22  Исключено использование определения APP_PACKET_PAYLOAD и соответствующие
//           директивы условного определения значения MaxParametersBlockSize.
//           При необходимости увеличения MaxParametersBlockSize это следует делать
//           динамически с использованием механизма layer3::getTxAppDataSize().
//
// 05.06.15  Добавлен идентификатор MsgReadBlock и структура TMsgReadBlock
//
// 28.08.09  Изменено значение MaxLayer2DataSize: 240 -> 244
//
//*****************************************************************************

#ifndef __DATA_IO_TYPES_H
#define __DATA_IO_TYPES_H

#include <stdint.h>

namespace DataIO
{
	const uint16_t 	MaxLayer2DataSize	= 244;				// макс. размер данных пакета уровня 2
	
	const uint16_t 	MaxParametersBlockSize	= 192;	// макс. размер блока параметров В БАЙТАХ

	//***************************************************************************
	//                          Типы ошибок протокола
	//***************************************************************************

	typedef enum {
		// layer 2
		errorRxDataCRC=0,				// ошибка crc принятых данных
		errorRxHeaderCRC,				// ошибка crc заголовка
		errorRxTimeout,					// таймаут приема пакета
		errorLossRxPacket,			// переполнение приемного буфера - не считан принятый пакет
		errorTxFrame,						// ошибка передачи пакета
		// layer 3
		errorUnknownMsgId 			// неизвестный идентификатор сообщения
	} TErrors;


	//***************************************************************************
	//                        Идентификаторы сообщений
	//***************************************************************************

	const uint8_t MsgParameterWrite		= 0x10;
	const uint8_t MsgParameterRead		= 0x11;
	const uint8_t MsgParameterValue		= 0x12;        
	const uint8_t MsgReadBlock				= 0x13;
	const uint8_t MsgReadBlock2				= 0x14;
	const uint8_t MsgAppType					= 0x80;                                
	

	//***************************************************************************
	//     Коды результата операции для запросов на запись и чтение параметра
	//***************************************************************************

	const int16_t ResultOk						= 0;
	const int16_t ResultBadParameter	= -1;		// недопустимое значение номера параметра
	const int16_t ResultBadValue			= -2;		// недопустимое значение параметра
	const int16_t ResultOperationError	= -3;	// ошибка выполнения операции


	//***************************************************************************
	//                Предопределенные типы сообщений уровня 3
	//***************************************************************************

	// id  - идентификатор типа сообщения - см. Msg..
	//
	// secuenceNumber  - порядковый номер запроса - присваивается в исходящем
	// запросе прикладной программой, этот же номер возвращается в ответном
	// сообщении и используется прикладной программой для сопоставления запроса
	// и ответного сообщения.
	// Зарезервированные значения:
	//   0xff - для исходящих сообщений от мастера, не предполагающих ответ
	//   0xfe - для сообщений, генерируемых устройством без запроса
	//
	// parameterNumber  - номер параметра
  //
	// parameterValue   - значение параметра
	//
	// result           - результат выполнения операции, см. Result..

  // Запись параметра
  typedef struct tagMsgParameterWrite  
  {
  	uint8_t		id;
  	uint8_t		sequenceNumber;
  	uint16_t	parameterNumber;
  	int32_t		parameterValue;
  }TMsgParameterWrite;
  
  // Чтение параметра
  typedef struct tagMsgParameterRead
  {
  	uint8_t		id;
  	uint8_t		sequenceNumber;
  	uint16_t	parameterNumber;
  }TMsgParameterRead;

  // Чтение блока параметров
  typedef struct tagMsgReadBlock			// @iss7 05.06.2015
  {
  	uint8_t		id;
  	uint8_t		sequenceNumber;
  	uint16_t	base;							// номер начального параметра
  	uint8_t		size;							// число параметров
  	uint8_t		reserved;
  }TMsgReadBlock;

  // Значение параметра - передается в ответ на запросы "Запись параметра"
  // и "Чтение параметра". В поле result передается результат выполнения операции.
  typedef struct tagMsgParameterValue
  {
  	uint8_t		id;
  	uint8_t		sequenceNumber;
  	uint16_t	parameterNumber;
  	int32_t		parameterValue;          
  	int16_t		result;
  	uint16_t  reserved;							// @v5.12 iss32-1 23.09.2013
  }TMsgParameterValue;

	// Заголовок сообщения прикладного типа.
	// type     - номер прикладного типа
	// dataSize - размер связанного блока данных
  typedef struct tagMsgAppTypeHeader
  {
  	uint8_t		id;
  	uint8_t		sequenceNumber;
  	uint16_t	type;
  	uint16_t	dataSize;
  }TMsgAppTypeHeader;
};                     

#endif

