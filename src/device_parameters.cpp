// device_parameters.cpp

#include "supervisor/supervisor.h"
#include "taskdef.h"
#include "hal.h"
#include "cfg/defines.h"
#include "tparameters.h"
#include "data_buffers.h"
#include "version.h"
#include "data.h"


TDevice_Parameters Parameters;

//
bool TDevice_Parameters::__write(uint16_t n, int32_t v)
{        
  bool rc = false;
  int32_t __v = v;
//  int32_t i;

  if(n >= data_buffers::kMinParameter  &&  n <= data_buffers::kMaxParameter)
  {
		if(data_buffers::handleParameters(data_buffers::kOpWrite, n, &__v) == DataIO::ResultOk)
		  rc = true;
		else
		  rc = false;
		// продолжить обработку в главном switch
  } 

	switch(n)
	{
		default: return rc;

    case 2:
      if(R14 == (R14_MASK_WR_REG | 2))
      {
//        setup1.serialNumber = v;
        R14 = 0;
      }
      else return false;
      break;

    case 3:
//      setup1.netAddress = v;
//      serviceFlags.cmdAcceptSAddress = 1;   // обрабатывается в TsMonitor
      break;

		case 4:
/*			switch(v)
			{
				default: return false;
				case 1200: case 2400: case 4800: case 9600: case 19200:
				case 38400: case 57600: case 115200: case 230400:
          if(n == 4)
          {
            serviceFlags.cmdAcceptRs485Baud = 1;
            setup2.baud = v;
          }
          else
          {
            supervisor.activateTask(task_SetupIOConfig);     
            sendMessage(grpRegisterIO, msgSetupIOConfig, n, v); 
          }
					break;
			} */
			break;
		
    case 14:
    	R14 = v; 
    	break;

    case 15:
/*      if(R14 == 0xB2)
      {
        R15 = v;
        R14 = 0;

        if(     (uint32_t)v == 0x88000000)
        {
          shutdownSystem(kOffLed_L0R_1s, HAL::kPD_modeOff, 4);    // выключение
          return false;
        }
        else if((uint32_t)v == 0x84000000)
          HAL::reset();                                         // reset
        else if((uint32_t)v == 0x82000000)
        {
          shutdownSystem(kOffLed_L0R_1s, HAL::kPD_modeWOR, 5);    // WOR
          return false;
        }
        else if((uint32_t)v == 0x82100000)                    // delayed sleep/WOR
        {
          if(HAL::powerState.isUextConnected()) return false;
          timeCmdGoSleep = now;
          deviceState.isDelayedSleep = 1;
          R15 |= 0x0008;
        }
        // Тестирование аварийных ситуаций
        else if((uint32_t)v == 0x82000001)                    // сброс по WDT
        {
          while(1);
        }
        else if((uint32_t)v == 0x82000002)                    // ошибка адреса -> HardFault
        {
          volatile unsigned int *ttt = (unsigned int *) 0xFFFFFF00;
          *ttt = 1;
        }
        else if((uint32_t)v == 0x82000003)                    // division by zero
        {
          v = R15 / R14;
        }
      }
      else return false; */
      break;

    case 19:
      // Проверка разрешения для критических операций
/*      if(v & R19_MASK_PROTECTED_OP)
      {
        if(R14 == (R14_MASK_WR_REG | 19))
          R14 = 0;
        else
          return false;
      }
      R19 = v; */
      break;

    case 21:
      // не выполнять синхронизацию при запущенном измерении
/*      if(!deviceState.isMeasStart  &&  !deviceState.isMeasOn)
      {
        HAL::syncTimer = v;
        R20 = 1;
        deviceState.isSyncOk = 1;
        deviceState.wasSyncRecovery = 0;
        getDateTime(&syncDateTime);
        R29 = time(0);
        sendMessage(grpRegisterIO, msgRegisterWrite, 21, v);
      }
      else
        return false; */
      break;
	}
  return true;
}

//-----------------------------------------------------------------------------
//
bool TDevice_Parameters::__read(uint16_t n, int32_t* v)
{
#ifdef PERFOMANCE_MONITOR
  if(n >= 600  &&  n <= 640)
  {
    if(     n >= 600  &&  n < 600 + PM_SIZE) *v = pmGetMax(n - 600);
    else if(n >= 610  &&  n < 610 + PM_SIZE) *v = pmGetTimeMax(n - 610);
    else if(n >= 620  &&  n < 620 + PM_SIZE) *v = pmGetAverage(n - 620);
    else if(n >= 630  &&  n < 630 + PM_SIZE) *v = pmGetIntegral(n - 630);
    else if(n == 640) *v = PM_SIZE;
    else return false;
    return true;
  }
#endif

  if(n >= data_buffers::kMinParameter  &&  n <= data_buffers::kMaxParameter)
  {
		if(data_buffers::handleParameters(data_buffers::kOpRead, n, v) == DataIO::ResultOk)
		  return true;
		else
		  return false;
  } 

  switch(n)
	{
		default: return false;

		case 0: *v = DEVICE_ID;		break;
		case 1: *v = VERSION_N;		break;
//    case 2: *v = setup1.serialNumber;   break;
//    case 3: *v = setup1.netAddress;     break;
//		case 4:	*v = setup2.baud;						break;
		case 2: *v = DEFAULT_SERIAL_NUMBER;   break;
		case 3: *v = DEFAULT_NET_ADDRESS_HI_IO;     break;
		case 4:	*v = UART1_BAUD;							break;

    case 14:  *v = R14;   break;
    case 15:  *v = R15;		break;

    case 21:  *v = now;		break;
	}
	return true;
}
