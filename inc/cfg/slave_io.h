// slave_io.h
//
// Настройки опроса и приема данных от блоков измерения 

#ifndef SLAVE_IO_H
#define SLAVE_IO_H

#include "dataio/dataio_app_const.h"

// Размер отсчёта данных отдельного типа
static constexpr uint16_t kSampleSize_Int32x3 = (3 * sizeof(int32_t));		

// Идентификатор типа данных
static constexpr uint16_t kDataId	= DataIO::MsgApp_DataInt32x3;

// Размер отсчёта данных
static constexpr uint16_t kDataSampleSize = kSampleSize_Int32x3;


#define SLAVE_IO_LINK_ID		LINK_ID_LOCAL		// используемый канал связи

#define SLAVE_IO_SLAVES_N				 	8					// макс. число ведомых устройств

#define SLAVE_IO_DATA_BUFFERS_N		28 				// количество буферов данных

#define SLAVE_IO_RQ_PERIOD				400				// период отправки запросов на получение данных, мс
#define SLAVE_IO_RQ_TIMEOUT			 	120				// таймаут ответа от устройства, мс


#endif // SLAVE_IO_H
