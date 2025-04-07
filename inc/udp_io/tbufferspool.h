// tbufferspool.h
//
// Операции с массивом статических буферов 

class TBuffersPool
{
protected:
	// состояние буфера
	static const uint8_t kStFree		= 0;	// свободен
	static const uint8_t kStBusy		= 1;	// занят 

public:
	uint8_t 	**buffers;
	uint8_t 	*state;
	
	uint16_t	size,									// количество элементов в buffers и state
						freeN;								// количество свободных буферов
	
public:
	void reset(void)
	{
		for(uint16_t i=0; i<size; i++)
			state[i] = kStFree;

		freeN = size;
	}            

	TBuffersPool(uint8_t** abuffers, uint8_t* astate, uint16_t asize) :
		buffers(abuffers),
		state(astate),
		size(asize)
	{
		reset();
	}                        

	// Количество свободных элементов
	inline uint16_t free(void)	{ return freeN; }

	// Общее число элементов
	inline uint16_t capacity(void)	{ return size; }
	
	// Возвращает свободный буфер. При отсутствии свободных буферов возвращает nullptr.
	uint8_t* get(void)
	{                       
		if(freeN == 0) return 0;

		for(uint16_t i=0; i<size; i++)
			if(state[i] == kStFree)
			{                    
				state[i] = kStBusy;
				freeN--;
				return buffers[i];
			}
			
		freeN = 0;
		return nullptr;
	}
		
	// Освобождение буфера
	void release(uint8_t* p)
	{                   
		for(uint16_t i=0; i<size; i++)
			if(buffers[i] == p)
			{                    
				state[i] = kStFree;
				if(freeN < size)
					freeN++;
				return;
			}
	}         
};
