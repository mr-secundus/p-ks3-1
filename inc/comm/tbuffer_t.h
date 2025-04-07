// tbuffer.h            
//
// 21.10.2021
// 1. Добавлен конструктор для инициализации по массиву, содержащему данные.
// 2. Добавлен конструктор копирования.
// 3. Добавлен operator =()
// 3. Добавлен внутренний флаг full для корректной работы с полностью заполненым
//    буфером.
//
// 16.07.2020
// free и dataSize реализованы через атомарные операции с запретом/разрешением прерываний -
// по результатам отладки ошибок обмена в проекте SK-LPC178x/test1 - i52-1.
//
// 15.03.2016
// Добавлено undoRead()
//
// 27.0515
// Копирование с использованием memcpy.
//
// 11.05.15
// Оптимизированы циклы копирования в write и read для массивов.
//
// 29.01.15  iss7
//   - Добавлена skip - пропуск данных вместо чтения;
//   - Изменены функции инкремента индексов на прямое сравнение вместо AND
//     с маской, теперь размер буфера может иметь произвольное значение.
//

#ifndef __TBUFFER_T_H
#define __TBUFFER_T_H

#include <string.h>
#include <ports.h>

#define TBUFFER_MEMCPY				// использовать memcpy, иначе побайтовое копирование в цикле
// уменьшение времени выполнения по сравнению с побайтовым копированием, раз:
// size=16:  1.8
// size=32:  3
// size=98:  4.7
// size=198: 6


template<class T> class TBuffer_T
{
	protected:
		T *buffer;
		int32_t size;
    int32_t iread, iwrite;
    bool  full;
		
    inline void incrementIRead(void)
    {
      iread++;
      if(iread >= size)
        iread = 0;
      full = false;
    }

    inline void incrementIWrite(void)
    {
      iwrite++;
      if(iwrite >= size)
        iwrite = 0;
      if(iwrite == iread)
        full = true;
    }

	public:                
		TBuffer_T(T* abuffer, uint16_t asize) :
			buffer(abuffer),
			size(asize),   
			iread(0),
			iwrite(0),
			full(false)
			{}

		// Инициализация по буферу, содержащему данные размером len -
		// выполняет установку iwrite в соответствии с len.
		TBuffer_T(T* abuffer, uint16_t asize, uint16_t len) :
      buffer(abuffer),
      size(asize),
      iread(0),
      iwrite(len)
      {
		    full = len == asize ? true : false;
      }

		TBuffer_T(const TBuffer_T& b) :
      buffer(b.buffer),
      size(b.size),
      iread(b.iread),
      iwrite(b.iwrite),
      full(b.full)
      {}

		const TBuffer_T& operator =(const TBuffer_T& b)
		{
      buffer = b.buffer;
      size =   b.size;
      iread =  b.iread;
      iwrite = b.iwrite;
      full =   b.full;
      return *this;
		}
			
		// сброс индексов чтения/записи выходного буфера
		inline void flush(void) { iwrite = iread = 0; full = false; }

		// возвращает объем свободного места
    inline uint32_t free(void)
	  {
      if(full) return 0;

			int32_t r, w;
			disable();
			r = iread; w = iwrite;
			enable();
			if(r > w) return r - w - 1;
			else 			return size + r - w - 1;
		} 
		
		// возвращает объем данных
    inline uint32_t dataSize(void)
	  {
      if(full) return size;

      int32_t r, w;
			disable();
			r = iread; w = iwrite;
			enable();
			if(r <= w) return w - r;
			else 			return size + w - r;
		} 

    inline uint32_t write(T c)
    {                                                       
//    	if(!free()) return 0;
//      if(full) return 0;
    	buffer[iwrite] = c;
    	incrementIWrite();
	  	return 1;
    }

    inline void write_wo_check(T c)
    {                                                       
    	buffer[iwrite] = c;
    	incrementIWrite();
    }
    
#ifdef TBUFFER_MEMCPY
    inline uint32_t write(T* buf, uint16_t n)
    {
    	uint16_t n1 = free() > n ? n : free();

    	if(iwrite + n1  <  size)               
    	{
				memcpy((uint8_t*)(buffer + iwrite), (uint8_t*)buf, n1*sizeof(T));
				iwrite += n1;
			}
	    else
	    {                      
	    	uint16_t d = size - iwrite;
				memcpy((uint8_t*)(buffer + iwrite), (uint8_t*)buf,       d*sizeof(T));
				memcpy((uint8_t*)buffer,            (uint8_t*)(buf + d), (n1 - d)*sizeof(T));
				iwrite = n1 - d;				
	    }

    	if(iwrite == iread)  full = true;

    	return n1; 
  	}                
#else
    inline uint32_t write(T* buf, uint16_t n)
    {
    	uint16_t lim, n1 = free() > n ? n : free();
    	if(iwrite + n1  <  size)               
    	{
				for(lim=iwrite+n1; iwrite<lim; iwrite++)	buffer[iwrite] = *buf++;
				if(iwrite == size) iwrite = 0;
			}
	    else
	    {
				lim = n1-(size-iwrite);
				for(; iwrite<size; iwrite++)				buffer[iwrite] = *buf++;
				for(iwrite=0; iwrite<lim; iwrite++)	buffer[iwrite] = *buf++;
	    }
    	return n1; 
  	} 

#endif
		
		inline T read(void)
		{               
			T c=0;
			if(dataSize())
			{	
				c = buffer[iread];
				incrementIRead();
			}        
			return c;
		}

		inline T read_wo_check(void)
		{               
			T c = buffer[iread];
			incrementIRead();
			return c;
		}

#ifdef TBUFFER_MEMCPY
    inline uint32_t read(T* buf, uint32_t n)
    {                                                       
    	int32_t n1 = dataSize() > n ? n : dataSize();
    	if(iread + n1  <  size)               
    	{
				memcpy((uint8_t*)buf, (uint8_t*)(buffer + iread), n1*sizeof(T));
				iread += n1;
			}
	    else
	    {
	    	uint16_t d = size - iread;
				memcpy((uint8_t*)buf,       (uint8_t*)(buffer + iread),  d*sizeof(T));
				memcpy((uint8_t*)(buf + d), (uint8_t*)buffer,            (n1 - d)*sizeof(T));
				iread = n1 - d;				
	    }
    	full = false;
    	return n1; 
    }          
#else
    inline uint32_t read(T* buf, uint32_t n)
    {                                                       
    	uint32_t lim, n1 = dataSize() > n ? n : dataSize();
    	if(iread + n1  <  size)               
    	{
				for(lim=iread+n1; iread<lim; iread++)	*buf++ = buffer[iread];
				if(iread == size) iread = 0;
			}
	    else
	    {
				lim = n1-(size-iread);
				for(; iread<size; iread++)				*buf++ = buffer[iread];
				for(iread=0; iread<lim; iread++)	*buf++ = buffer[iread];
	    }
    	return n1; 
    }
#endif
};                     

#endif

