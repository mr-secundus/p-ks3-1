// dataio_types.h
//
// »дентификаторы, коды результата операции и типы протокола DataIO

#ifndef DATA_IO_TYPES_H
#define DATA_IO_TYPES_H

#include <stdint.h>

#include <dataio_config.h>

namespace DataIO
{
	const uint16_t 	MaxLayer2DataSize	= MAX_DATA_BUFFER_SIZE;		// макс. размер данных пакета уровн€ 2
	const uint16_t 	MaxParametersBlockSize	= 192;	// макс. размер блока параметров ¬ Ѕј…“ј’

	//***************************************************************************
	//                          “ипы ошибок протокола
	//***************************************************************************

	typedef enum {
		// layer 2
		errorRxDataCRC=0,				// ошибка crc прин€тых данных
		errorRxHeaderCRC,				// ошибка crc заголовка
		errorRxTimeout,					// таймаут приема пакета
		errorLossRxPacket,			// переполнение приемного буфера - не считан прин€тый пакет
		errorTxFrame,						// ошибка передачи пакета
		// layer 3
		errorUnknownMsgId 			// неизвестный идентификатор сообщени€
	} TErrors;


	//***************************************************************************
	//                        »дентификаторы сообщений
	//***************************************************************************

	const uint8_t MsgParameterWrite		= 0x10;
	const uint8_t MsgParameterRead		= 0x11;
	const uint8_t MsgParameterValue		= 0x12;        
	const uint8_t MsgReadBlock				= 0x13;
	const uint8_t MsgReadBlock2				= 0x14;
	const uint8_t MsgAppType					= 0x80;                                
	

	//***************************************************************************
	//      оды результата операции дл€ запросов на запись и чтение параметра
	//***************************************************************************

	const int16_t ResultOk						= 0;
	const int16_t ResultBadParameter	= -1;		// недопустимое значение номера параметра
	const int16_t ResultBadValue			= -2;		// недопустимое значение параметра
	const int16_t ResultOperationError	= -3;	// ошибка выполнени€ операции


	//***************************************************************************
	//                ѕредопределенные типы сообщений уровн€ 3
	//***************************************************************************

	// id  - идентификатор типа сообщени€ - см. Msg..
	//
	// secuenceNumber  - пор€дковый номер запроса - присваиваетс€ в исход€щем
	// запросе прикладной программой, этот же номер возвращаетс€ в ответном
	// сообщении и используетс€ прикладной программой дл€ сопоставлени€ запроса
	// и ответного сообщени€.
	// «арезервированные значени€:
	//   0xff - дл€ исход€щих сообщений от мастера, не предполагающих ответ
	//   0xfe - дл€ сообщений, генерируемых устройством без запроса
	//
	// parameterNumber  - номер параметра
  //
	// parameterValue   - значение параметра
	//
	// result           - результат выполнени€ операции, см. Result..

  // «апись параметра
  typedef struct tagMsgParameterWrite  
  {
  	uint8_t		id;
  	uint8_t		sequenceNumber;
  	uint16_t	parameterNumber;
  	int32_t		parameterValue;
  }TMsgParameterWrite;
  
  // „тение параметра
  typedef struct tagMsgParameterRead
  {
  	uint8_t		id;
  	uint8_t		sequenceNumber;
  	uint16_t	parameterNumber;
  }TMsgParameterRead;

  // „тение блока параметров
  typedef struct tagMsgReadBlock			// @iss7 05.06.2015
  {
  	uint8_t		id;
  	uint8_t		sequenceNumber;
  	uint16_t	base;							// номер начального параметра
  	uint8_t		size;							// число параметров
  	uint8_t		reserved;
  }TMsgReadBlock;

  // «начение параметра - передаетс€ в ответ на запросы "«апись параметра"
  // и "„тение параметра". ¬ поле result передаетс€ результат выполнени€ операции.
  typedef struct tagMsgParameterValue
  {
  	uint8_t		id;
  	uint8_t		sequenceNumber;
  	uint16_t	parameterNumber;
  	int32_t		parameterValue;          
  	int16_t		result;
  	uint16_t  reserved;							// @v5.12 iss32-1 23.09.2013
  }TMsgParameterValue;

	// «аголовок сообщени€ прикладного типа.
	// type     - номер прикладного типа
	// dataSize - размер св€занного блока данных
  typedef struct tagMsgAppTypeHeader
  {
  	uint8_t		id;
  	uint8_t		sequenceNumber;
  	uint16_t	type;
  	uint16_t	dataSize;
  }TMsgAppTypeHeader;
};                     

#endif

