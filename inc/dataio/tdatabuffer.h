// tdatabuffer.h
//
// Хранение блоков данных (отсчёты от АЦП) с сопутствующей информацией

#ifndef TDATABUFFER_H
#define TDATABUFFER_H

#include <stdint.h>

#ifndef MAX_DATA_BUFFER_SIZE
#include "dataio_config.h"
#endif


// Блок данных
// Содержит массив отсчётов и дополнитеьную информацию, которые будут помещены в
// выходной пакет для передачи или записи в УХД.
class TDataBuffer
{   
public:             
	// размер буфера данных, байт
  static constexpr uint32_t kMaxDataSize = (MAX_DATA_BUFFER_SIZE) - 8;

	uint16_t	packet,			// номер пакета - заполняется при переводе буфера из состояния
												// заполнение данными в состояние ожидания передачи в КС или УХД
												// циклическая нумерация от 0 до 0xFFFF
						size;				// число ОТСЧЕТОВ ДАННЫХ ПРИКЛДНОГО ТИПА в data[]
	uint32_t	sync;				// значение таймера синхронизации для первого отсчета в data[]
					
	uint8_t		data[kMaxDataSize];

	// Размер вспомогательных данных
	static constexpr uint16_t getServiceInfoSize(void)
	{ 
		return sizeof(packet) + sizeof(size) + sizeof(sync); 
	}

	// Запись одного отсчёта в буфер
	//	void write(uint16_t v)	{ reinterpret_cast<uint16_t*>(data)[dataSize++] = v; }
	//	void write(int16_t v)		{ reinterpret_cast<int16_t* >(data)[dataSize++] = v; }
	//	void write(int32_t v)		{ reinterpret_cast<uint32_t*>(data)[dataSize++] = v; }   
	template<class T> void write(T v)	{ reinterpret_cast<T*>(data)[size++] = v; }

	//
	// Ref_T_Size     размер типа данных, который считает dataSize, в байтах
	//
	
	// Запись одного отсчета данных из байтового буффера
	//
	// t_size		размер одного отсчета данных, байт
	void write(uint8_t* b, const uint32_t t_size)
	{
		uint16_t index = size * t_size;
		for(uint16_t i = 0; i < t_size; )
			data[index++] = b[i++];
		size++;
	}   

	// Запись нескоьких отсчетов данных из байтового буффера.
	// Проверяет размер свободного места в data[], при заполнении перкращает запись.  
	//
	// t_size		размер одного отсчета данных, байт
	// n				количество отсчётов для записи
	//
	// Возвращает количество записанных отсчётов
	uint16_t write(uint8_t* b, const uint32_t t_size, uint16_t n)
	{
		uint16_t j = 0;
		uint16_t index = size * t_size;
		
		while(j < n  &&  (kMaxDataSize - size * t_size) >= t_size)
		{
			for(uint16_t i = 0; i < t_size; )
				data[index++] = b[i++];
			
			size++;
			j++;
		}
		return j;
	}   

	// Методы с использованием RefT_Size применяются, когда dataSize представляет 
	// количество многоканальных отсчетов, состоящих из нескольких отсчетов базового типа.
	// Например, при использовании для каждого канала отсчетов типа int16_t
	// размер многоканального отсчета для N каналов:
	//   RefT_Size = N * sizeof(int16_t)

	// Возвращает размер свободного места в буфере в отсчетах типа T      
	template<class T> uint32_t free(void)
	{
	  return kMaxDataSize / sizeof(T) - size;
	}

	// Возвращает размер свободного места в буфере в отсчетах типа T,
	// при хранении в data отсчетов другого типа (RefT)
	//
	// RefT_Size - sizeof типа, для которого считается dataSize
	//
	// >> специфичная операция, надо смотреть происхождение гле-то в контроллере ПРД 2022 
	template<class T> uint32_t free(const uint32_t RefT_Size)
	{
	  return kMaxDataSize / sizeof(T) - size * RefT_Size / sizeof(T);
	}

	// Возвращает указатель на текущую позицию для записи данных
	// RefT_Size - sizeof типа, для которого считается dataSize
	template<class T> T* getCurrentDataPtr(const uint32_t RefT_Size)
	{
    return ((T*)data) + size * RefT_Size / sizeof(T);
	}

	// Возвращает указатель на текущую позицию для записи данных
	template<class T> T* getCurrentDataPtr(void)
	{
    return ((T*)data) + size;
	}

	// Добавление n к dataSize после записи в буфер внешними методами
	void dataSizeAdd(uint32_t n)
	{
	  size += n;
	}
};          


// Расширенный блок данных
// Содержит дополнительную информацию для работы с многоканальными отсчётами данных
using TDataBufferEx = struct tagTDataBufferEx
{                
	// размер буфера данных, байт
	static constexpr uint32_t kMaxDataSize = (MAX_DATA_BUFFER_SIZE) - 12;

	uint16_t	packet,					// номер пакета - заполняется при переводе буфера из состояния
														// заполнение данными в состояние ожидания передачи в КС или УХД
														// циклическая нумерация от 0 до 0xFFFF
						size,						// число ОТСЧЕТОВ ДАННЫХ ПРИКЛАДНОГО ТИПА в data[]
						format,					// id формата данных (DataIO::TMsgAppTypeHeader.type)
						channels;				// битовые флаги каналов в data[]
	uint32_t	sync;						// значение таймера синхронизации для первого отсчета в data[]
					
	uint8_t		data[kMaxDataSize];

	// Размер вспомогательных данных
	static constexpr uint16_t getServiceInfoSize(void)
	{
//		return sizeof(packetNumber) + sizeof(dataSize) + sizeof(sync) + sizeof(format) +  + sizeof(channels);
		return sizeof(tagTDataBufferEx) - sizeof(data); 
	}

	// Макс. количество отсчётов_прикладного_типа в массиве данных блока
	constexpr uint16_t getMaxDataSize(uint8_t szOfDataT)
	{ 
		return sizeof(data) / szOfDataT; 
	}
	
	// Запись одного отсчёта в буфер
	//	void write(uint16_t v)	{ reinterpret_cast<uint16_t*>(data)[dataSize++] = v; }
	//	void write(int16_t v)		{ reinterpret_cast<int16_t* >(data)[dataSize++] = v; }
	//	void write(int32_t v)		{ reinterpret_cast<uint32_t*>(data)[dataSize++] = v; }   
	template<class T> void write(T v)	{ reinterpret_cast<T*>(data)[size++] = v; }
	
	// Запись одного отсчета данных из байтового буффера
	//
	// ref_t_size		размер одного отсчета данных, байт
	void write(uint8_t* b, const uint32_t ref_t_size)
	{
		uint16_t index = size * ref_t_size;
		for(uint16_t i = 0; i < ref_t_size; )
			data[index++] = b[i++];
		size++;
	}   
}; 

#endif              
