// tqueue.h
//
// Параметризованная очередь объектов фиксированного типа.
// Для специфицируемого типа должен быть определен оператор =.
// Под буфер используется внешний статический массив, передаваемый конструктору оъекта.
//

#ifndef __TQUEUE_H
#define __TQUEUE_H

template<class T> class TQueue
{
  private:
   typedef T *PBuffer;
   typedef unsigned short TIndex;

   const TIndex msgBufferSize;
   volatile TIndex begin, end;
   const PBuffer msgBuffer;
   volatile bool isFull, isEmpty;

  public:
   
   // bufSize - макс. количество элементов типа T в очереди
   TQueue(unsigned char* buffer, const short bufSize) :
      msgBufferSize(bufSize),
      begin(0),
      end(0),
      msgBuffer((T*)buffer),
      isFull(false),
      isEmpty(true)
      { }

		void clear() volatile { end = begin; isEmpty = true; isFull = false; }
		bool empty() const volatile { return isEmpty; }
		bool full() const volatile { return isFull; }
		long size() const volatile { return empty() ? 0 : end > begin ? end - begin : msgBufferSize - begin + end; }
		
		void push(const T* const msg) volatile
		{
			if(full()) return;    
			msgBuffer[end] = *msg;
			end++;
			if(end == msgBufferSize) end = 0;
			if(end == begin) isFull = true;
			isEmpty = false;
		}
		
		T* front(void) volatile
		{
			if (!empty()) return msgBuffer+begin;
			else return (T*)0;
		}

		void pop(void) volatile
		{
			if(!empty())
			{
				begin++;
				if(begin == msgBufferSize) begin = 0;
				if(end == begin) isEmpty = true;
			}
			isFull = false;
		}
};

#endif

