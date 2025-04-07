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

// Максимальное число ведомых устройств для опроса
static constexpr int32_t kMaxSlaves = 4;

#define SLAVE_RQ_PERIOD				200			// период отправки запросов на получение данных, мс
#define SLAVE_RQ_TIMEOUT			 50			// таймаут ответа от устройства, мс


#endif // SLAVE_IO_H
