// tparameters.h

#ifndef TPARAMETERS_H
#define TPARAMETERS_H                                                 

//
class TDevice_Parameters
{
protected:	 
  uint16_t base(void) { return 0; }
                                    
  // переопределяются в производных классах для реализации функциональности
  bool __write(uint16_t n, int32_t v);
  bool __read(uint16_t n, int32_t* vptr);

public:
  // запись значения v в параметр с номером n
  int16_t write(uint16_t n, int32_t v)
  {
//			return have(n) ? __write(n, v) : -1;
    return __write(n, v)  ? 0 : -1;
  }

  // чтение значения параметра с номером n в переменную по адресу vptr
  int16_t read(uint16_t n, int32_t* vptr)
  {
//			return have(n) ? __read(n, vptr) : -1;
    return __read(n, vptr) ? 0 : -1;
  }
};

void initParameters(void);

// tdevice_parameters.cpp

bool internalSetParameter(uint16_t n, uint32_t v);
bool internalGetParameter(uint16_t n, uint32_t* v);


extern TDevice_Parameters Parameters;    

#endif

