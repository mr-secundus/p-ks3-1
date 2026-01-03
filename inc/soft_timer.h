// soft_timer.h
//
// Программный таймер

#ifndef SOFT_TIMER_H
#define SOFT_TIMER_H

class SoftTimer
{
private:
	uint32_t 	value,
						threshold;
	
public:
	SoftTimer(uint32_t th) : threshold(th) 
	{
		value = 0;
	}

	void reset(void)
	{
		value = 0;
	}
	
	void increment(uint32_t incValue)
	{
		value += incValue;
	}
	
	bool active(void)
	{
		return value >= threshold;
	}
};

#endif
