// tdatabufferscontainer.h

//#include "lwip/pbuf.h"

#include "cfg/defines.h"
#include "dataio/tdatabuffer.h"
#include "cfg/slave_io.h"

class TDataBuffersContainer
{
public:
	// Коды возврата методов 
	static constexpr int16_t kRcErr = -1;		// ошибка выполнения операции  

	// Размер контейнера
	static constexpr int16_t kMaxSize = SLAVE_IO_DATA_BUFFERS_N;
	
	// Состояние буфера
	// Именованные константы вместо true/false используются для совместимости с реализацией
	// с тремя состояниями
	static constexpr bool stFree = false,
												stFull = true;

	// Состояние буфера данных
/*	enum state_t
	{
		stFree,					// свободен
		stBusy,					// занят - идет запись
		stFull					// заполнен данными
	}; */

protected:
	// Хранение информации об одном буфере данных. 
	// Базовый элемент для хранения данных - buffer.
	// pbuffer содержит buffer + резерв под служебную информацию для передачи по сети. 
	using buffer_info_t = struct
	{
		TDataBuffer *buffer;					// блок данных
//		struct pbuf	*pbuffer;					// pbuf для размещения buffer
		uint16_t		address;					// адрес ведомого устройства, от которого приняты данные
		bool 				state;						// true - блок содержит данные
	};
	
	buffer_info_t		buffers[kMaxSize];
	
	uint16_t	size,									// количество элементов в buffers и state
						bufferCapacity,				// макс. кол-во отсчётов в одном буфере
						nFree,								// количество свободных буферов
						nFull,								// количество заполненных буферов
						iFree,								// индекс очередного свободного буфера
						iFull,								// индекс очередного заполненного буфера
						sizeofTData;					// размер одного отсчета данных - в данной реализации не используется
	
public:
	TDataBuffersContainer() :
		size(kMaxSize),                                               
		bufferCapacity(TDataBuffer::kMaxDataSize),
		nFree(kMaxSize),
		nFull(0),
		iFree(0),
		iFull(0),
		sizeofTData(0)
	{
		for(auto& x : buffers)
		{
			x.buffer = nullptr;
//			x.pbuffer = nullptr;
			x.state = stFree;
		}
	}                        


	// Возвращает макс. число отсчетов в буфере
	uint16_t getBufferCapacity(void) { return bufferCapacity; }

	// Возвращает число отсчетов в буфере
	uint16_t getBufferDataSize(uint16_t n)	{ return buffers[n].buffer->size; }

	// Возвращает количество свободных буферов 
	uint16_t getFreeBuffers(void)	{ return nFree; }

	// Возвращает количество заполненных буферов 
	uint16_t getFullBuffers(void)	{ return nFull; }

	// Возвращает общее количество буферов
	uint16_t capacity(void)	{ return size; }
	
	// Возвращает true, если буфер с индексом n имеет состояние Full
	bool isFull(uint16_t n)	{	return buffers[n].state == stFull; }

	//---------------------------------------------------------------------------
	//  Запись и управление состоянием буферов

	// Сброс состояния
	// Помечает все буферы как свободные и соответственно устанавливает значения счётчиков.
	// Не меняет значения указателей на буферы.  
	void reset(void)
	{
		for(auto& x : buffers)
		{
			x.state = stFree;
			x.address = 0;
		}
		
		nFree = size;
		nFull = 0;
		iFree = 0;
		iFull = 0;      
	}
	
	
	// Инициализация указателей на буферы данных в элементе контейнера
	//
	// n							индекс буфера
	// pPbuf					pbuf ptr
	// pDataBuffer		TDataBuffer ptr
//	void setup(int16_t n, struct pbuf* pPbuf, TDataBuffer* pDataBuffer)
	void setup(int16_t n, TDataBuffer* pDataBuffer)
	{
		if(n < size)
		{
//			buffers[n].pbuffer = pPbuf;
			buffers[n].buffer = pDataBuffer;
		}
	}
	
	
	// Устанавливает размер отсчёта данных
	void setDataSize(uint16_t sz) { sizeofTData = sz;	}

	
	// Возвращает индекс первого свободного буфера
	// При отсутствии свободных буферов возвращает kRcErr
	int16_t getFreeBuffer(void)
	{
		if(nFree == 0)
			return kRcErr;
		else
			return iFree;
	}
	
	
	// Чтение адреса источника данных для отдельного буфера
	//
	// Возвращает адрес источника данных для буфера с индексом n.
	// При некорректном n возвращает 0.
	uint16_t getAddress(uint16_t n)
	{
		if(n < kMaxSize)
			return buffers[n].address;
		else
			return 0;
	}

	
	// Установка адреса источника данных для отдельного буфера
	//
	// n					индекс в массиве буферов
	// addres			сетевой адрес источника данных
	void setAddress(uint16_t n, uint16_t address)
	{
		if(n < kMaxSize)
			buffers[n].address = address;
	}
		
	
	// Найти свободый буфер и пометить его как занятый.
	// Выполняет установку указателя на следующий свободный буфер.
	//
	// Возвращает индекс выбранного буфера или kRcErr, если нет свободных буферов. 
	int16_t lockFreeBuffer(void)
	{
		if(nFree == 0)
			return kRcErr;
		
		int16_t index = iFree;
		
		if(++iFree >= size)
			iFree = 0;
		
		nFree--;
		
		uint16_t i;
		for(i = 0; i < size  &&  buffers[i].state != stFree; i++)
			if(++index >= size)
				index = 0;

		if(i < size)
		{
			buffers[index].buffer->packet = 0;
			buffers[index].buffer->size = 0;
//			buffers[index].state = stBusy;
			buffers[index].state = stFull;
			return index;
		}
		else
			return kRcErr; 
	}

	
	// Перевод буфера с индексом n в состояние "Free"
	void releaseBuffer(uint16_t n)
	{
		if(buffers[n].state == stFull)
		{                
			if(nFree > 0)
				nFree--;		
			
			if(nFull > 0)
				if(++iFull == size)
					iFull = 0;
		}
		
		if(buffers[n].state != stFree)
		{
			if(nFree < size)
				nFree++;
			
			if(nFree == 1)
				iFree = n;
		}                    
		buffers[n].state = stFree;
		buffers[n].buffer->packet = 0;
		buffers[n].address = 0;
	}         

	// Перевод буфера в состояние "Free"
/*	void releaseBuffer(buffer_info_t* b)
	{
		if(b->state)
			if(nFree > 0)
				nFree--;		
		
		if(b->state)
			if(nFree < size)
				nFree++;

		b->state = stFree;
		b->buffer = 0;
	} */         


	// Снять статус "Busy" с буфера с номером n
	// Если в буфере есть данные, он переводится в состояние "Full", иначе "Free"
	void unlockBuffer(uint16_t n)
	{
		if(buffers[n].buffer->size)
		{
			buffers[n].state = stFull;
			
			if(nFull < size)
				nFull++;
			
			if(nFull == 1)
				iFull = n;
		}
		else
			releaseBuffer(n);   
	}                     
	
	
	// Перевод буфера с packetNumber==pn в состояние "Free"
	//
	// return		true		Ok
	//          false 	не найден буфер с заданным pn
	bool releaseBufferByPacketNumber(uint16_t pn)
	{                         
/*		for(auto& x : buffers)
			if(x.buffer->packet == pn)
			{
				releaseBuffer(i);
				return true;
			} */

		for(uint16_t i = 0; i < size; i++)       
			if(buffers[i].buffer->packet == pn)
			{
				releaseBuffer(i);
				return true;
			}
		
		return false;
	}

	//---------------------------------------------------------------------------
	//  Чтение
	
	// Возврашает индекс очередного заполненного буфера
/*	int16_t getFullBuffer(void)
	{
		for(uint32_t i = 0; i < size; i++, iFull++)
		{
			if(iFull >= size)
				iFull = 0;
			
			if(state[iFull] == stFull)
				return iFull;
		}
		return kRcErr;
	}

	
	// Поиск заполненного блока, начиная с индекса n
	// Возвращает индекс заполненного блока, или kRcErr
	int16_t getNextFullBuffer(uint16_t n)
	{
		for(uint32_t i = 0; i < size; i++, n++)
		{
			if(n >= size)
				n = 0;
			
			if(state[n] == stFull)
				return n;
		}
		return kRcErr;
	} */

	
	// Возвращает указатель на буфер с индексом n 
	TDataBuffer* getBuffer(int16_t n)	{ return buffers[n].buffer;	}

	
	// Возвращает заполненный буфер с минимальным значением sync
	// Если не найден, возвращает NULL
	int16_t getFullBufferSortBySync(void)
	{
		uint32_t syncMin = 0xFFFFFFFF;    // мин. обнаруженное значение sync
		int16_t index = -1;               // индекс выбранного буфера

		for(uint16_t i = 0; i < size; i++)
			if(buffers[i].state == stFull)
				if(buffers[i].buffer->sync < syncMin)
				{
					syncMin = buffers[i].buffer->sync;
					index = i;
				}

		if(index >= 0)
		{
			iFull = index;
			return index;
		}
		else
			return kRcErr;
	}
};
