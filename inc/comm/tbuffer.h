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

#ifndef __TBUFFER_H
#define __TBUFFER_H

#include <string.h>
#include <port.h>

#define TBUFFER_MEMCPY				// использовать memcpy, иначе побайтовое копирование в цикле
// уменьшение времени выполнения по сравнению с побайтовым копированием, раз:
// size=16:  1.8
// size=32:  3
// size=98:  4.7
// size=198: 6


class TBuffer
{
	protected:
		uint8_t *buffer;
		int16_t size;
    int16_t iread, iwrite;
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
		TBuffer(uint8_t* abuffer, uint16_t asize) :
			buffer(abuffer),
			size(asize),   
			iread(0),
			iwrite(0),
			full(false)
			{}

		// Инициализация по буферу, содержащему данные размером len -
		// выполняет установку iwrite в соответствии с len.
		TBuffer(uint8_t* abuffer, uint16_t asize, uint16_t len) :
      buffer(abuffer),
      size(asize),
      iread(0),
      iwrite(len)
      {
		    full = len == asize ? true : false;
      }

		TBuffer(const TBuffer& b) :
      buffer(b.buffer),
      size(b.size),
      iread(b.iread),
      iwrite(b.iwrite),
      full(b.full)
      {}

		const TBuffer& operator =(const TBuffer& b)
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
    inline uint16_t free(void)
	  {
      if(full) return 0;

			int16_t r, w;
			disable();
			r = iread; w = iwrite;
			enable();
			if(r > w) return r - w - 1;
			else 			return size + r - w - 1;
		} 
		
		// возвращает объем данных
    inline uint16_t dataSize(void)
	  {
      if(full) return size;

      int16_t r, w;
			disable();
			r = iread; w = iwrite;
			enable();
			if(r <= w) return w - r;
			else 			return size + w - r;
		} 

    inline uint16_t write(uint8_t c)
    {                                                       
//    	if(!free()) return 0;
      if(full) return 0;
    	buffer[iwrite] = c;
    	incrementIWrite();
	  	return 1;
    }

    inline void write_wo_check(uint8_t c)
    {                                                       
    	buffer[iwrite] = c;
    	incrementIWrite();
    }
    
#ifdef TBUFFER_MEMCPY
    inline uint16_t write(uint8_t* buf, uint16_t n)
    {
    	uint16_t n1 = free() > n ? n : free();

    	if(iwrite + n1  <  size)               
    	{
				memcpy(buffer + iwrite, buf, n1);
				iwrite += n1;
			}
	    else
	    {                      
	    	uint16_t d = size - iwrite;
				memcpy(buffer + iwrite, buf,     d);
				memcpy(buffer,          buf + d, n1 - d);
				iwrite = n1 - d;				
	    }

    	if(iwrite == iread)  full = true;

    	return n1; 
  	}                
#else
    inline uint16_t write(uint8_t* buf, uint16_t n)
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
		
		inline uint8_t read(void)                        
		{               
			uint8_t c=0;
			if(dataSize())
			{	
				c = buffer[iread];
				incrementIRead();
			}        
			return c;
		}

		inline uint8_t read_wo_check(void)                        
		{               
			uint8_t c = buffer[iread];
			incrementIRead();
			return c;
		}

#ifdef TBUFFER_MEMCPY
    inline uint16_t read(uint8_t* buf, uint16_t n)
    {                                                       
    	uint16_t n1 = dataSize() > n ? n : dataSize();
    	if(iread + n1  <  size)               
    	{
				memcpy(buf, buffer + iread, n1);
				iread += n1;
			}
	    else
	    {
	    	uint16_t d = size - iread;
				memcpy(buf,     buffer + iread,  d);
				memcpy(buf + d, buffer,          n1 - d);
				iread = n1 - d;				
	    }
    	full = false;
    	return n1; 
    }          
#else
    inline uint16_t read(uint8_t* buf, uint16_t n)
    {                                                       
    	uint16_t lim, n1 = dataSize() > n ? n : dataSize();
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
		
		// пропуск n байт вместо реального чтения
/*    inline void skip(uint16_t n)
    {                     
    	uint16_t s = dataSize();
    	if(n > s) n = s;
    	iread += n;
    	while(iread >= size) iread -= size;
    	full = false;
    }

		// отмена операции чтения - "откат" индекса чтения на n позиций назад
    void undoRead(int16_t n)
    {                     
			if(iread >= n) 	iread -= n;
			else						iread = size + iread - n;
    } */
};                     

//
class TBufferWRandomAccess : public TBuffer
{
	public:              
		TBufferWRandomAccess(uint8_t* abuffer, uint16_t asize) :
			TBuffer(abuffer, asize)
			{}

		// получение символа из позиции в буфере (iwrite-1-index)
		inline uint8_t get(int16_t index)
		{                 
			if(iwrite >= index+1)
				return buffer[iwrite-index-1];
			else
			  return buffer[size-1-index+iwrite];
		}      
};
		
			

#endif

