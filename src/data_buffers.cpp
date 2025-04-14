// data_buffers.cpp    
//
// Операции с буферами данных
//  - размещение контейнера буферов и выделение памяти
//  - обработка запросов от ВУ на передачу и подтверждение БД
//  - запись данных в буферы
//
// Реализация с выделением памяти под буферы из статического массива.


#include <stdio.h>

#include <putx.h>

#include "hal.h"
#include "protocol.h"
#include "data_buffers.h"
#include "tdatabufferscontainer.h"
#include "cfg/slave_io.h"
#include "debug.h"

// Формирование блоков данных в режиме имитации для тестирования модуля
//#define SIM_DATA				

#define DATA_BUFFERS_N	SLAVE_IO_DATA_BUFFERS_N


namespace data_buffers
{    
//-----------------------------------------------------------------------------
//                                  Data
//-----------------------------------------------------------------------------

uint8_t rawBuffer[(MAX_DATA_BUFFER_SIZE) * (DATA_BUFFERS_N)] __attribute__((section(".data.$RAM2")));

TDataBuffersContainer buffers;
	
//-----------------------------------------------------------------------------
//                                 Private
//-----------------------------------------------------------------------------

//
// Формирует сообщения с состоянием устройства и блоком данных и заносит в
// текущий выходной пакет. При полном заполнении пакета выполняет передачу пакета,
// затем инициируется новый пакет.
//
// seqNumber		  sequenceNumber посылаемого пакета. При отправке данных в ответ
//                на запрос берется из запроса, при автопередаче равно 0xfe.
//               
// return		 0  Ok
//          -1  недостаточно места в выходном буфере
//
int16_t addMsgData(uint8_t seqNumber, TDataBuffer* buffer)
{    
	DataIO::TMsgAppTypeHeader msg;
	msg.id							= DataIO::MsgAppType;
  msg.sequenceNumber	= seqNumber;
  msg.type						= kDataId;
  msg.dataSize				= buffer->getServiceInfoSize() + buffer->size * kDataSampleSize;
  
	int16_t result = Protocol::addMessage((uint8_t*)&msg, (uint8_t*)buffer);
		
#ifdef DEBUG_TX_BUFFERS
  puts(sendStateInfo ? " S:" : " s:"); putd(buffer->packetNumber);
  puts(" sq:");   putl(seqNumber);
  puts(" sy:");   putl(buffer->sync);
  puts(" dsz:");  putl(buffer->dataSize);
  puts(" rc:");   putd(result);

  if(result != 0)  puts(" Error");
#endif

  return result == 0 ? 0 : -1;
}


//-----------------------------------------------------------------------------
//                                 Public
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                         Управление модулем 
//-----------------------------------------------------------------------------

//
// Инициализация модуля.
// В отладочном варианте заполняет буфер данных, используемый для имитации.
//
void init(void)
{       
	for(int i = 0; i < DATA_BUFFERS_N; i++)
		buffers.setup(i, reinterpret_cast<TDataBuffer*>(&rawBuffer[MAX_DATA_BUFFER_SIZE * i]));
	
#ifdef SIM_DATA	
	using sample_x3_t = struct tag_sample_x3
	{
		int32_t x, y, z;
	}; 
	
	sample_x3_t sample = {0, 200, 400};
	
	// Заполнить в buffers несколько блоков данных 
	for(int i = 0; i < DATA_BUFFERS_N; i++)
	{
		int16_t n = buffers.lockFreeBuffer();
		
		if(n != buffers.kRcErr)
		{
			TDataBuffer* b = buffers.getBuffer(n);

			while(b->free<sample_x3_t>() > 0)
			{
				b->write((uint8_t*)&sample, sizeof(sample));
				sample.x++;
				sample.y++;
				sample.z++;
			}

			b->packet = i;
			b->sync = now;
			buffers.unlockBuffer(n);
		}
	}
#endif	
}                         


//
void reset(void)
{                      
//	txInterface.reset();
	buffers.reset();	
}                         


//-----------------------------------------------------------------------------
//             Интерфейс к обработчику протокола - обмен с ВУ 
//-----------------------------------------------------------------------------
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
int16_t handleParameters(uint16_t opCode, uint16_t n, int32_t *v)
{                                 
	if(opCode == kOpWrite)
		switch(n)
		{         
			default: return DataIO::ResultBadParameter;
		}
	else if(opCode == kOpRead)
		switch(n)
		{         
			default:	return DataIO::ResultBadParameter;
			
			case 40:	*v = buffers.getFullBuffers();			break;
			case 41:	*v = buffers.getFreeBuffers();			break;
//			case 42:	*v = lostSamplesCounter;						break;
//      case 44:  *v = maxFullBuffers;                break;
		}                                            
	return DataIO::ResultOk;
}


//
// Обработка запроса на передачу блока данных.
// Передаёт очередной заполненный блок.
//
// srcAddress		сетевой адрес источника запроса
// sendState		отправить StateInfo - в данной реализации не выполняется
//
void handleMsgRequestData(uint16_t srcAddress, DataIO::TMsgAppTypeHeader* msg, bool sendState)
{
//	int16_t rc = -1;
	int16_t n = buffers.getFullBufferSortBySync();
	
	if(n != buffers.kRcErr)
	{
//		rc = addMsgData(msg->sequenceNumber, buffers.getBuffer(n));
		addMsgData(msg->sequenceNumber, buffers.getBuffer(n));
	}
}


//
// Обработка сообщения TMsgDataAcknowledge - подтверждение получения данных
//
void handleMsgDataAcknowledge(uint16_t srcAddress, DataIO::TMsgAppDataAcknowledge* msg)
{                                    
#ifdef DEBUG_TX_BUFFERS
	bool result = buffers.releaseBufferByPacketNumber(msg->packetNumber);
	putlog(result ? " Ack:" :  " AckErr:"); putd(msg->packetNumber); putc('\n');
#else
	buffers.releaseBufferByPacketNumber(msg->packetNumber);
#endif
}            


//
// Возвращает информацию о количестве данных для передачи.
// 	kNoData					нет данных
// 	kIsFullBuffer		один или более заполненных буферов
//                                                  
int16_t getDataReadyInfo(void)
{
	if(buffers.getFullBuffers() > 0)
		return kIsFullBuffer;
	else 
		return kNoData;
}    


//-----------------------------------------------------------------------------
//                       Интерфейс записи данных 
//-----------------------------------------------------------------------------
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
int16_t writeBuffer(TDataBuffer* buffer, uint32_t dataId)
{
	if(dataId != kDataId)
		return kIncorrectId;
	
	int16_t n = buffers.lockFreeBuffer();
	
	if(n != buffers.kRcErr)
	{
		TDataBuffer* b = buffers.getBuffer(n);

		// размер buffer->data[], байт
		uint32_t dataSize = buffer->size * kDataSampleSize;
		
		if(dataSize <= TDataBuffer::kMaxDataSize)
		{
			memcpy(b, buffer, TDataBuffer::getServiceInfoSize() + dataSize);
			return TDataBuffer::getServiceInfoSize() + dataSize;
		}
		else
			return kIncorrectSize;
	}
	else
		return kNoBuffers;
}

};		// namespace
