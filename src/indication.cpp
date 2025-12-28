 // indication.cpp

#include "cfg/defines.h"
#include "hal.h"
#include "indication.h"

namespace indication
{
	// Состояния автомата индикации
	enum class fm_state_t
	{
		kStart,										// начало цикла индикации
		kCheckState,							// определение состояния
		kIndication								// отработка индикации
	};
	
	// Состояние индикатора
	enum led_state_t { kLedOff, kLedRed, kLedGreen };

	// Тип индикации
	enum itype_t { kI0, kI1, kI2, kI3, kI4 };
	
	bool state[AppState::kMaxState];					// отдельные состояния прикладного уровня
  fm_state_t fmState = fm_state_t::kStart;  // состояние автомата индикации

  // Обслуживание индикации для отдельного индикатора
  struct
  {
    uint32_t  timer;           // отсчет интервалов изменения состояния индикатора
    uint8_t   cnt;             // счетчик состояний
    itype_t 	st;              // тип индикации
    bool      end;             // флаг завершения цикла индикации
  } led0 = {0, 0, kI0, false}, 
	  led1 = {0, 0, kI0, false};
  
  led_state_t l0set, l1set;		// управление выводом на индикаторы  

	
	// Установка значения состояния устройства
	void setState(AppState index, bool value)
	{
		state[index] = value;
	}
	
	
	// Обработка автомата индикации
	void process(void)
	{
	  uint32_t tm, tmnow = now;
		
	  switch(fmState)
	  {
	    default:
	    case fm_state_t::kStart:
	        led0.cnt = led1.cnt = 0;
	        led0.end = led1.end = false;
	        fmState = fm_state_t::kCheckState;
	      break;

	    case fm_state_t::kCheckState:
	    	if(state[kHw_Err])
	    		led0.st = kI0;
	    	else if(state[kSetup_Err])												// ошибка загрузки setup
	    		led0.st = kI1;
	    	else if(!state[kEth_Link])												// нет линка
	    		led0.st = kI2;
	    	else if(state[kEth_Link]  &&  !state[kEth_Io])		// есть линк, нет обмена по Ethernet
	    		led0.st = kI3;
	    	else if(state[kEth_Link]  &&  state[kEth_Io])			// есть обмен по Ethernet
	    		led0.st = kI4;
	    	
	    	if(!state[kRs485_Io])															// отсутствует обмен по RS485
	    		led0.st = kI0;
	    	else if(state[kRs485_Err])												// ошибки обмена по RS485
	    		led1.st = kI1;
	    	else
	    		led0.st = kI2;																	// обмен по RS485
	    	
	      fmState = fm_state_t::kIndication;
	      led0.timer = led1.timer = tmnow;
	    	break;

	    case fm_state_t::kIndication:
	      if(tmnow >= led0.timer)
	      {
	        switch(led0.st)
	        {
	        	default: 
	            if(led0.cnt==0)
	            	{ l0set = kLedOff; tm = 2000; led0.end = true; }
	            break;
	        		
	          case kI1:																// ошибка загрузки setup
	            if(led0.cnt==0 || led0.cnt==2)
	            	{ l0set = kLedRed;   tm = 500;   led0.cnt++; }
	            else if(led1.cnt==1 || led1.cnt==3)
	            	{ l0set = kLedOff;   tm = 500;   led0.cnt++; }
	            if(led1.cnt==3)
	            	led0.end = true;
	            break;

	          case kI2:																// нет линка
	            if(led0.cnt==0)
	            	{ l0set = kLedRed; tm = 2000; led0.end = true; }
	            break;

	          case kI3:																// есть линк, нет обмена по Ethernet
	            if(led0.cnt==0)
	            	{ l0set = kLedGreen;  tm = 2000; led0.end = true; }
	            break;

	          case kI4:																// есть обмен по Ethernet
	            if(led0.cnt==0)
	            	{ l0set = kLedGreen;  tm = 500;  led0.cnt++; }
	            else if(led1.cnt==1)
	            	{ l0set = kLedOff;  tm = 1500;  led0.end = true; }
	            break;
	        }
	        
	        led0.timer += MS_TO_TICKS(tm);
	        
	        bool l0, l1;
	        if(l0set == kLedRed)        { l0 = true; l1 = false; }
	        else if(l0set == kLedGreen) { l0 = false; l1 = true; }
	        else                        { l0 = false; l1 = false; }
					hal::setLed(hal::kLed0, l0);
					hal::setLed(hal::kLed1, l1);
				}

	      if(tmnow >= led1.timer)
	      {
	        switch(led1.st)
	        {
	        	default: 
	            if(led1.cnt==0)
	            	{ l1set = kLedOff; tm = 2000; led1.end = true; }
	            break;
	        		
	          case kI0:																// отсутствует обмен по RS485
	            if(led1.cnt==0)
	            	{ l1set = kLedRed; tm = 2000; led1.end = true; }
	            break;

	          case kI1:																// ошибки обмена по RS485
	            if(led1.cnt==0 || led1.cnt==2)
	            	{ l1set = kLedRed;  tm = 500;  led1.cnt++; }
	            else if(led1.cnt==1 || led1.cnt==3)
	            	{ l1set = kLedOff;  tm = 500;  led1.cnt++; }
	            break;

	          case kI2:																// обмен по RS485
	            if(led1.cnt==0 || led1.cnt==2)
	            	{ l1set = kLedGreen;  tm = 500;  led1.cnt++; }
	            else if(led1.cnt==1 || led1.cnt==3)
	            	{ l1set = kLedOff;  tm = 500;  led1.cnt++; }
	            if(led1.cnt==3)
	            	led1.end = true;
	            break;
	        }
	        
	        led1.timer += MS_TO_TICKS(tm);
	        
	        bool l0, l1;
	        if(l1set == kLedRed)        { l0 = true; l1 = false; }
	        else if(l1set == kLedGreen) { l0 = false; l1 = true; }
	        else                        { l0 = false; l1 = false; }
					hal::setLed(hal::kLed2, l0);
					hal::setLed(hal::kLed3, l1);
				}
	      break;		// case fm_state_t::kIndication:
	  }
	}
};		// namespace
