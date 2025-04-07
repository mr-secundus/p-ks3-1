// tmavrgfilt.h
//
// moving average filter   
//
// 26.02.19  При n<size взвращается усредненное значение по текущим сумме/количеству отсчетов
// 05.01.11  Введено оперативное изменение длины фильтра 
//
//

#ifndef __TMAVRGFILT_H
#define __TMAVRGFILT_H                   

template<class T> class TMovAvgFilter
{
	unsigned short	size;		    // длина фильтра
	unsigned short	index, n;
	T *data;
	long sum;
	
	public:
		TMovAvgFilter(unsigned short sz, T* buffer) :
			size(sz),
			data(buffer)
			{
				reset();
			}
		
	void reset(void)
	{
		int i;
		for(i=0; i<size; i++) data[i] = 0;
		index = size-1;
		sum = 0;
		n = 0;
	  return;
	}            

	T process(T v)
	{               
		sum -= data[index];
		sum += v;         
		data[index] = v;
		index++;
		if(index >= size) index=0;
/*		if(n < size)
		{
			n++; return v;
		}
		else return sum/size; */
		if(n < size) n++;
		return sum/n; 
	}                                                     
	
	void setSize(unsigned short sz) { size = sz; reset();	}
	unsigned short getSize(void)    { return size;	}
};

#endif


