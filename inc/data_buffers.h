// data_buffers.h

#include "dataio/dataio_app_types.h"                 

namespace data_buffers
{    
// Диапазон номеров параметров, обрабатываемых модулем 
constexpr int16_t	kMinParameter = 40;
constexpr int16_t	kMaxParameter = 41;

// Параметр opCode для handleParameters_DataIO()
constexpr int16_t	kOpWrite 	= 0;	
constexpr int16_t	kOpRead 	= 1;	

// Коды возарата функций модуля
enum rc_t
{
	// Информация о наличии данных в буфере FIFO
	kIsFullBuffer = 2,					// один или более заполненных буферов
	kNoData = 0,								// нет данных
	// Результат записи блока данных в контейнер
	kNoBuffers = -1,						// нет свободных буферов
	kIncorrectSize = -2,				// несоответствие размера входящего буфера
	kIncorrectId = -3						// тип данных не поддерживается
};


//
// Инициализация модуля.
// В отладочном варианте заполняет буфер данных, используемый для имитации.
//
void init(void);

//
// Сброс состояния модуля
//
void reset(void);

//
// Обработка чтения/записи параметров/регистров модуля вывода и буферизации данных
//
// n				номер параметра
// v				адрес значения параметра
// opCode		тип операции - kOpWrite, kOpRead
//
// return		DataIO::ResultOk								Ok
//					DataIO::ResultBadParameter			некорректный номер параметра
//					DataIO::ResultBadValue				  недопустимое значение параметра
//					DataIO::ResultOperationErrorr		ошибка выполнения операции
//
int16_t handleParameters(uint16_t opCode, uint16_t n, int32_t *v);

//
// Обработка запроса на передачу блока данных.
// При наличии заполненного блока посылается очередной заполненный блок,
// если заполненных буферов нет, берется текущий используемый буфер при
// наличии в нем данных, иначе посылается сообщение об отсутствии данных.
//
// srcAddress		сетевой адрес источника запроса
// sendState		отправить StateInfo
//
void handleMsgRequestData(uint16_t srcAddress, DataIO::TMsgAppTypeHeader* msg, bool sendState=false);

//
// Обработка сообщения TMsgDataAcknowledge - подтверждение получения данных
//
void handleMsgDataAcknowledge(uint16_t srcAddress, DataIO::TMsgAppDataAcknowledge* msg);

//
// Возвращает информацию о количестве данных для передачи
//
// return			kIsFullBuffer, kIsData, kNoData0
//                                                  
int16_t getDataReadyInfo(void);

//
// Запись блока в контейнер
//
// Выделяет очередной свободный блок в контейнере и копирует в него данные из b.
// Возвращает ошибку, если размер данных в b превышает ёмкость буфера в контейнере.
//
// buffer			указатель на входящий блок данных
// dataId			тип данных - соответствует полю DataIO::TMsgAppTypeHeader.type
//
// return			>0							полный размер скопированного блока, байт
//						kNoBuffers 			нет свободных буферов
//						kIncorrectSize	несоответствие размера входящего буфера
//						kIncorrectId		тип данных не поддерживается
//
int16_t writeBuffer(TDataBuffer* buffer, uint32_t dataId);
};		// namespace



