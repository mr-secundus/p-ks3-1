// dataio_app_types.h
//
// Определение прикладных типов данных протокола DataIO     
//
// ВНИМАНИЕ: идентификаторы прикладных блоков данных должны занимать не более 
// 9 младших значащих разрядов - старшие разряды используются как служебные 
// в модуле работы с FLASH накопителем.  
//
// 13.09.23  Константы со значениями подтипов MsApp вынесены в файл dataio_app_const.h
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
//
// Используемые сокращения: КС - канал связи;  УХД - устройство хранения данных
//
//*****************************************************************************
//
// Прикладные сообщения, не имеющие данных, передаются в виде типа TMsgAppTypeHeader
// и идентифицируются значением поля type.
// Для сообщений, имеющих данные, определяются отдельные типы сообщений, содержащие
// поле header типа TMsgAppTypeHeader, и далее набор данных.
//
// MsgApp_RequestData  (данных не содержит)
//		Запрос от мастера на получение очередного блока данных.
//		В ответном сообщении используется sequenceNumber из исходящего сообщения.
//		При отсутствии заполненных буферов посылает сообщение MsgApp_DataAbsence.
//		При наличии заполненных буферов посылает сообщение MsgApp_Data<type T>.
//
// MsgApp_DataAcknowledge 
//		Команда от мастера на удаление блока (подтверждение получения данных).
//		Данные: packetNumber - номер пакета для удаления - берется из поля
//		packetNumber полученного мастером блока данных
//    При нормальном выполнении команды и sequenceNumber входящего запроса равном 0xff
//		ответ не посылатся.
//    При нормальном выполнении команды и sequenceNumber входящего запроса не равном 0xff
//		посылатся ответное сообщение MsgApp_DataAcknowledgeOk.
//    При ошибке выполнения команды (некорректный номер пакета) всегда посылается 
//		ответное сообщение MsgApp_DataAcknowledgeError.
//		В ответном сообщении используется sequenceNumber из исходящего сообщения.
//
// MsgApp_DataAbsence  (данных не содержит)
//		Сообщение мастеру об отсутствии данных.
//		sequenceNumber соответствует значению из входящего запроса
//                                                                       
// MsgApp_Data<type Т>
//		В поле данных сообщения передается блок данных типа TDataBuffer (см. tdatabuffer.h)
//		sequenceNumber берется из входящего запроса, либо равно 0xfe при
//		автоматической передаче.
//		TMsgData.dataSize = размер служебных полей из TDataBuffer + число байт,
//		содержащихся в TDataBuffer.data. TDataBuffer.dataSize = число ОТСЧЕТОВ данных
//		типа T в TDataBuffer.data. В настоящее время для хранения данных АЦП1 (отсчетов)
//		используем тип int32_t, т.о. устройство передает данные в сообщениях TMsgDataint32_t.
//		Отдельный тип структуры для сообщения не описан, использовать как
//		TMsgAppTypeHeader + блок данных типа TDataBuffer
//		                                                
// MsgApp_StateInfo
//		Передается информация о текущем состоянии устройства:
//			- напряжение аккумулятора
//			- состояние системы питания
//			- состояние модуля буферизации данных
//		Передается либо по запросу MsgApp_RequestStateInfo (sequenceNumber берется
//		из входящего запроса), либо посылается устройством в одном пакете с данными,
//		при этом sequenceNumber=0xfe                      
//
// MsgApp_RequestStateInfo
//	Запрос на передачу MsgApp_StateInfo
//
//*****************************************************************************
//
// MsgApp_RequestDateTime
//	Запрос на передачу MsgApp_DateTime
//
// MsgApp_DateTime 
//	В поле данных содержится дата и время.
//	При передаче от мастера в прибор выполняется установка RTC прибора по содержащимся данным.
//	При передаче от прибора мастеру по запросу содержит текущие значения RTC прибора.
//  
//*****************************************************************************
//
// MsgApp_RequestSvData  (данных не содержит)
//		Запрос от мастера на получение сохраненного блока данных из УХД.
//		В ответном сообщении используется sequenceNumber из исходящего сообщения.
//		При отсутствии несчитанных данных в УХД посылает сообщение MsgApp_SvDataAbsence.
//		При наличии данных посылает сообщение MsgApp_SvData, в поле page которого 
//    передается номер страницы УХД, из которой был считан блок. Page используется 
//    в сообщении от мастера, подтверждающем прием данных.
//    При наличии информации о дате/времени записи страницы, в пакете также 
//    передается сообщение MsgApp_SvDateTime, содержащее также номер страницы.
//
// MsgApp_SvData
//		Содержит номер страницы УХД и блок данных типа TDataBuffer.
//		sequenceNumber соответствует значению из входящего запроса.
//
// MsgApp_SvDateTime 
//	  Содержит номер страницы УХД и структуру TDateTime.
//		sequenceNumber соответствует значению из входящего запроса.
//    ВНИМАНИЕ: в текущей версии ПО датчиков ДСА сообщение содержит календарное
//    время выполнения синхронизации (считанное по RTC устройства).
//
// MsgApp_SvDataAbsence  (данных не содержит)
//		Сообщение мастеру об отсутствии данных в УХД для чтения.
//		sequenceNumber соответствует значению из входящего запроса.
//
// MsgApp_SvDataReadError
//		Сообщение мастеру об ошибке чтения из УХД.
//    Содержит номер страницы, из которой производилось чтение и код ошибки.
//		sequenceNumber соответствует значению из входящего запроса.
//    Значения кода ошибки:
//      -1 - УХД не готово к выполнению операции
//      -2 - страница не содержит данных (ошибка CRC заголовка страницы)
//      -3 - нет данных для чтения
//      -4 - ошибка CRC блока данных
//      -5 - неизвестный тип блока данных
//
// MsgApp_SvDataAcknowledge 
//		Команда от мастера на очистку страницы УХД.
//    Может посылаться в ответ на сообщения MsgApp_SvData или MsgApp_SvDataReadError.
//		Содержит номер страницы УХД, который берется из сообщения MsgApp_SvData.
//    При нормальном выполнении команды и sequenceNumber входящего запроса равном 0xff
//		ответ не посылатся.
//    При нормальном выполнении команды и sequenceNumber входящего запроса не равном 0xff
//		посылатся ответное сообщение MsgApp_SvDataAcknowledgeOk.
//    При ошибке выполнения команды всегда посылается ответное сообщение
//    MsgApp_SvDataAcknowledgeError. Возможные причины ошибки:
//      - УХД не готово - либо при аппаратной неисправности либо при выполнении операции
//        параллельно с вводом данных от АЦП и сохранением в УХД;
//      - УХД не содержит данных;
//      - текущий номер страницы для чтения не соответствует номеру страницы во
//        входящем запросе.
//    В двух последних случаях необходимо повторить запрос на получение данных из УХД.
//		В ответном сообщении используется sequenceNumber из исходящего сообщения.
//
// MsgApp_FlashGoBusy
//    Посылается в ответ на MsgApp_SvDataAcknowledge, если было запущено стирание сектора,
//    которое может выполняться до нескольких секунд.
//
//*****************************************************************************
//       
// Последовательность действий по получению данных при Автопередача=выкл:
//
//	Цикл:
//	- Послать запрос TMsgRequestData
//
//  - После получения сообщения TMsgData послать сообщение TMsgDataAcknowledge 
//		с полем packetNumber из входящего сообщения;
//		возможно получение сообщения TMsgDataAbsence (при получении таких сообщений
//		необходимо сопоставлять таймаут прихода данных с установленной в конфигурации
//		частотой дискретизации и при превышении таймаута получения данных
//		диагностировать сбой устройства с выполнением переинициализации;
//
//	- При таймауте получения данных повторить запрос TMsgRequestData или
//		обрабатывать потерю связи с прибором.
// 
//
// Последовательность действий по получению данных при Автопередача=вкл:
//
//	- разрешить автопередачу установкой бита R20.2
//
//	Цикл:
//  - После получения сообщения TMsgData послать сообщение TMsgDataAcknowledge 
//		 с полем packetNumber из входящего сообщения
//
//	- При таймауте получения данных обрабатывать потерю связи с прибором.
//
//*****************************************************************************

#ifndef DATAIO_APP_TYPES_H
#define DATAIO_APP_TYPES_H

//#include <dataio_types.h>
//#include <dataio_app_const.h>
//#include "tdatabuffer.h"
#include <dataio/dataio_types.h>
#include <dataio/dataio_app_const.h>
#include <dataio/tdatabuffer.h>


namespace DataIO
{
//
typedef struct tagMsgAppDataAcknowledge
{
	DataIO::TMsgAppTypeHeader	header;			// dataSize = sizeof(uint16_t)
	uint16_t	packetNumber;
}TMsgAppDataAcknowledge;


//
// Содержание поля powerState структуры TMsgAppStateInfo:
//
// b0:	UbatLow1			- требуется заряд
// b1:	UbatLow2			- аккумулятор разряжен, выключение при работе от батареи
// b2:	UbatHigh			- заряд > 90%
// b3:	batteryFail		- неисправность аккумулятора
// b4:	externalPower	- подключен внешний источник
// b5:	charge				- идет заряд                                             
// b6:	batteryOn			- включено питание от батареи
//
// Содержание поля dataInputState:
//
// b0:	была потеря данных от АЦП (отсчеты не были записаны в буфер)
// b1:	в УХД имеются непереданные данные, сохранённые в течении текущего 
//      сеанса работы (с момента последнего сброса состояния модуля буферизации
//      данных, выполняемого записью R20.0)
// b2:  идет измерение
// b3:  был произведен отложенный пуск АЦП 
// b4:  активен спящий режим (в каком-то виде)   
// b5:  ошибка данных инклинометра


//
// информация о состоянии устройства СККВ/ДСА
//
typedef struct tagMsgAppStateInfo
{
	DataIO::TMsgAppTypeHeader	header;			// dataSize = sizeof(uint8_t)*2 + sizeof(uint16_t)
	//
	uint16_t	Ubat;									// напряжение аккумулятора, мВ
	uint8_t		powerState;
	uint8_t		dataInputState;
	uint16_t	dataBlocksNumber;			// количество заполненных блоков данных, находящихся в буфере,
																// включая уже переданные, но неподтвержденные
	tagMsgAppStateInfo(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= DataIO::MsgApp_StateInfo;
		header.dataSize = sizeof(struct tagMsgAppStateInfo) - sizeof(DataIO::TMsgAppTypeHeader);
	}                         
}TMsgAppStateInfo;     

//
// информация о состоянии устройства СИУ/СИМУ
//
typedef struct tagMsgAppStateInfo_31
{
	DataIO::TMsgAppTypeHeader	header;			
	//
	uint32_t	sync;									// значение таймера синхронизации на момент формирования сообщения
	uint16_t	Upwr;									// напряжение внешнего питания, мВ
	uint8_t		dataInputState;				// состояние системы измерения:
																// b0: была потеря данных 
	uint8_t		dataBlocksNumber;			// количество заполненных блоков данных, находящихся в буфере,
																// включая уже переданные, но неподтвержденные
//	int8_t		t[4];									// показания датчиков температуры [гр. С]
	int16_t		t[4];									// показания датчиков температуры [гр. С]
																// 0: плата П1; 1: плата П2; 2,3: внешние  
																// для датчика крутящего момента в t[3] передаются показания встроенного в 
																// первичный датчик датчика температуры  
	uint8_t		s0;										// b0, b1 - состояние выходов управления нагревателями    
	uint8_t		reserved;

	tagMsgAppStateInfo_31(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= DataIO::MsgApp_StateInfo_31;
		header.dataSize = sizeof(struct tagMsgAppStateInfo_31) - sizeof(DataIO::TMsgAppTypeHeader);
	}                         
}TMsgAppStateInfo_31;

//
// информация о состоянии устройства СККВ2
//
typedef struct tagMsgAppStateInfo_32
{
	DataIO::TMsgAppTypeHeader	header;			// dataSize = sizeof(uint8_t)*2 + sizeof(uint16_t)
	//
	uint16_t	Ubat;									// напряжение аккумулятора, мВ
	uint8_t		powerState;
	uint8_t		dataInputState;
	uint16_t	dataBlocksNumber;			// количество заполненных блоков данных, находящихся в буфере,
																// включая уже переданные, но неподтвержденные
	int16_t		t[2];									// показания датчиков температуры [гр. С * 0.1]
																// 0: плата П1; 1: батарейный отсек 
	tagMsgAppStateInfo_32(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= DataIO::MsgApp_StateInfo_32;
		header.dataSize = sizeof(struct tagMsgAppStateInfo_32) - sizeof(DataIO::TMsgAppTypeHeader);
	}                         
}TMsgAppStateInfo_32;

//
// информация о состоянии устройства СКУ-2/БИТ-1
//
typedef struct tagMsgAppStateInfo_9221
{
	DataIO::TMsgAppTypeHeader	header;			// dataSize = sizeof(uint8_t)*2 + sizeof(uint16_t)
	//
	uint16_t	Ubat;									// напряжение аккумулятора, мВ
	uint8_t		powerState;
	uint8_t		dataInputState;
	uint16_t	dataBlocksNumber;			// количество заполненных блоков данных, находящихся в буфере,
																// включая уже переданные, но неподтвержденные
	int16_t		t[2];									// показания датчиков температуры [гр. С * 0.1]
																// 0: плата П1; 1: батарейный отсек 
	uint32_t	r150;									// состояние id внешних датчиков
	
	tagMsgAppStateInfo_9221(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= MsgApp_StateInfo_9221;
		header.dataSize = sizeof(struct tagMsgAppStateInfo_9221) - sizeof(DataIO::TMsgAppTypeHeader);
	}                         
}TMsgAppStateInfo_9221;

//
// Информация о состоянии устройства
// Введена в ПРД (УРМ 2022)
//
typedef struct tagMsgAppStateInfo_5430
{
	// Константы для битового поля dataInputState
	static const uint16_t
		Mask_loss1	=		0x0001,			// потеря данных АЦП1
		Mask_loss2	=		0x0002,			// потеря данных АЦП2
		Mask_data1	=		0x0004,			// наличие данных АЦП1
		Mask_data2	=		0x0008;			// наличие данных АЦП2

	DataIO::TMsgAppTypeHeader	header;			// dataSize = sizeof(uint8_t)*2 + sizeof(uint16_t)

	uint8_t		dataBlocksNumber1;		// количество заполненных блоков данных в FIFO АЦП1
	uint8_t		dataBlocksNumber2;		// количество заполненных блоков данных в FIFO АЦП2
	uint32_t	r17;									// R17 - состояние устройства
	uint32_t	r150;									// состояние id внешних датчиков
	uint16_t	flags;				        // битовые флаги - см. константы
	
	tagMsgAppStateInfo_5430(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= MsgApp_StateInfo_5430;
		header.dataSize = sizeof(struct tagMsgAppStateInfo_5430) - sizeof(DataIO::TMsgAppTypeHeader);

		r17 = r150 = 0; flags = 0; dataBlocksNumber1 = dataBlocksNumber2 = 0;
	}                         
}TMsgAppStateInfo_5430;

// дата/время
typedef struct tagMsgAppDateTime
{
	DataIO::TMsgAppTypeHeader	header;			// dataSize = sizeof(uint8_t)*2 + sizeof(uint16_t)
	uint16_t	year;
	uint8_t		month, 
					dayOfMonth,
					hours,
					minutes,
					seconds,
					reserved;

	tagMsgAppDateTime(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= MsgApp_DateTime;
		header.dataSize = sizeof(struct tagMsgAppDateTime) - sizeof(DataIO::TMsgAppTypeHeader);
	}                         
}TMsgAppDateTime;

//-----------------------------------------------------------------------------
// запрос на передачу данных из УХД
typedef struct tagSvDataAcknowledge
{
	DataIO::TMsgAppTypeHeader	header;		
	uint16_t	page;

	tagSvDataAcknowledge(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= MsgApp_SvDataAcknowledge;
		header.dataSize = sizeof(struct tagSvDataAcknowledge) - sizeof(DataIO::TMsgAppTypeHeader);
	}                         
}TMsgAppSvDataAcknowledge;

// дата/время записи данных в УХД
typedef struct tagMsgAppSvDateTime
{
	DataIO::TMsgAppTypeHeader	header;			
	uint16_t	page;													// номер страницы УХД
	uint16_t	year;
	uint8_t		month, 
					dayOfMonth,
					hours,
					minutes,
					seconds,
					reserved;

	tagMsgAppSvDateTime(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= MsgApp_SvDateTime;
		header.dataSize = sizeof(struct tagMsgAppSvDateTime) - sizeof(DataIO::TMsgAppTypeHeader);
	}                         
}TMsgAppSvDateTime;

// данные из УХД
typedef struct tagMsgAppSvData
{
	DataIO::TMsgAppTypeHeader	header;	
	uint16_t	page;												// номер страницы УХД
	TDataBuffer data;

	// при создании в header.dataSize заносится размер блока, включающий:
	//   - размер заголовка
	//   - размер page
	//   - размер служебной информации из data
	// перед отправкой сообщения необходимо к header.dataSize добавить реальный размер данных,
	// содержащихся в data.data (байт) - вычисляется как data.dataSize*sizeof(TDataSample)
/*	tagMsgAppSvData(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= MsgApp_SvData;
		header.dataSize = sizeof(uint16_t) + data.getServiceInfoSize();
	}*/                         

	// Конструктор для задания произвольного значения type
	tagMsgAppSvData(uint16_t _type)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= _type;
		header.dataSize = sizeof(uint16_t) + data.getServiceInfoSize();
	}                         

	void initDataSize(void)
	{
		header.dataSize = sizeof(uint16_t) + data.getServiceInfoSize();
	}                         
}TMsgAppSvData;

// сообщение об ошибке чтения из УХД
typedef struct tagSvSvDataReadError
{
	DataIO::TMsgAppTypeHeader	header;		
	uint16_t	page;                    
	int16_t   error;

	tagSvSvDataReadError(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= MsgApp_SvDataReadError;
		header.dataSize = sizeof(struct tagSvSvDataReadError) - sizeof(DataIO::TMsgAppTypeHeader);
	}                         
}TMsgAppSvDataReadError;

//
// команда на включение устройства
//
typedef struct tagMsgAppWakeup
{
	static const uint16_t addrSize = 8;
	static const uint16_t pSize = 4;

	DataIO::TMsgAppTypeHeader	header;			// dataSize = sizeof(uint8_t)*2 + sizeof(uint16_t)

	uint8_t  r0, r1;							// @iss7 - для выравнивания addr на границу DWORD, что фактически всегда происходило
	//
	uint32_t addr[addrSize];			// массив битовых флагов адреса
	uint32_t p[pSize];						// параметры - резерв

	tagMsgAppWakeup(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= MsgApp_Wakeup;
		header.dataSize = sizeof(struct tagMsgAppWakeup) - sizeof(DataIO::TMsgAppTypeHeader);
	}                         
}TMsgAppWakeup;     


//
// Блок параметров - ответ на MsgReadBlock
// Здесь определение заголовка блока данных прикладного сообщения, 
// после него должен размещаться массив значений параметров
//
typedef struct tagMsgParameterBlockHeader
{
	DataIO::TMsgAppTypeHeader	header;			// sizeof(header) = 6
															//															#байта с 1	
	uint16_t		base;							// номер начального регистра		7 8
	uint8_t			size;							// число параметров							9
	uint8_t			errCount;					// счетчик ошибок								10
	uint16_t		reserved;					//															11 12

	tagMsgParameterBlockHeader(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= MsgApp_RegBlock;
		errCount				= 0;
	}
	
	// b  - номер начального регистра
	// sz - число параметров     
	// Также заполняет поле dataSize в заголовке 
	void setParameters(uint16_t b, uint16_t sz)
	{
		base = b;
		size = sz;
		header.dataSize = size*sizeof(uint32_t) + sizeof(struct tagMsgParameterBlockHeader) - sizeof(DataIO::TMsgAppTypeHeader);
	}      
	
	// возвращает указатель на блок данных прикладного сообщения для передачи в addMessage
	uint8_t* getAppDataPtr(void)
	{                                                                                     
		return (uint8_t*)&base;
	}                  

	// возвращает указатель на блок данных
	uint8_t* getDataPtr(void)
	{    
		return (uint8_t*)this + sizeof(struct tagMsgParameterBlockHeader);
	}
}TMsgParameterBlockHeader;     

//
// Блок параметров #2 - ответ на MsgReadBlock2
// Здесь определение заголовка блока данных прикладного сообщения, 
// после него должен размещаться массив значений параметров
//
// Это минимальный вариант TMsgParameterBlockHeader.
// Кол-во параметров на приемной стороне опредеяется косвенно по значению 
// поля dataSize в заголовке прикладного сообщения.
//
typedef struct tagMsgParameterBlockHeader2
{
	DataIO::TMsgAppTypeHeader	header;			// sizeof(header) = 6
	
	uint16_t		base;							// номер начального регистра		7 8

	tagMsgParameterBlockHeader2(void)
	{
		header.id 			= DataIO::MsgAppType;
		header.type			= MsgApp_RegBlock2;
	}
	
	// b  - номер начального регистра
	// sz - кол-во параметров типа uint32_t
	// Также заполняет поле dataSize в заголовке 
	void setParameters(uint16_t b, uint16_t size)
	{
		base = b;
		header.dataSize = size*sizeof(uint32_t) + sizeof(struct tagMsgParameterBlockHeader2) - sizeof(DataIO::TMsgAppTypeHeader);
	}      
	
	// возвращает указатель на блок данных прикладного сообщения для передачи в addMessage
	uint8_t* getAppDataPtr(void)
	{                                                                                     
		return (uint8_t*)&base;
	}                         
	
	// возвращает количество элементов размера tsize в блоке данных сообщения
	uint16_t getBlockDataSize(uint16_t tsize)
	{
		return (header.dataSize - sizeof(base)) / tsize;
	}                                                 
	
	// возвращает указатель на блок данных
	uint8_t* getDataPtr(void)
	{    
		return (uint8_t*)this + sizeof(struct tagMsgParameterBlockHeader2);
	}
}TMsgParameterBlockHeader2;     

};		// namespace

#endif
