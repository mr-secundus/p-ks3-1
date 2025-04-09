// device_parameters.cpp

#include "supervisor/supervisor.h"
#include "taskdef.h"
#include "hal.h"
#include "cfg/defines.h"
#include "tparameters.h"
#include "data_buffers.h"
#include "version.h"
#include "data.h"
#include "nv_parameters.h"


TDevice_Parameters Parameters;

//
bool TDevice_Parameters::__write(uint16_t n, int32_t v)
{        
  bool rc = false;
  int32_t __v = v;
  int32_t i;

  if(n >= data_buffers::kMinParameter  &&  n <= data_buffers::kMaxParameter)
  {
		if(data_buffers::handleParameters(data_buffers::kOpWrite, n, &__v) == DataIO::ResultOk)
		  rc = true;
		else
		  rc = false;
		// продолжить обработку в главном switch
  } 

  if(n >= 100  &&  n < (100 + LINK1_SLAVES_N))
  {
  	setup3.slaves[n - 100] = v;
  	return true;
  } 

	switch(n)
	{
		default: return rc;

    case 2:
      if(R14 == (R14_MASK_WR_REG | 2))
      {
        setup1.serialNumber = v;
        R14 = 0;
      }
      else return false;
      break;

    // R3 R4 только дл€ чтени€ дл€ совместимости с DataIOTool
    // «апись сетевых настроек через R60+ 
      
    case 14:
    	R14 = v; 
    	break;

    case 15:
    	if(R14 == 0xB2)
			{
				R15 = v;
				R14 = 0;

				if((uint32_t)v == 0x84000000)			// reset device
					hal::reset();
			}
    	break;

    	/*      if(R14 == 0xB2)
      {
        R15 = v;
        R14 = 0;

        if((uint32_t)v == 0x88000000)
        {
          shutdownSystem(kOffLed_L0R_1s, HAL::kPD_modeOff, 4);  // выключение
          return false;
        }
        else if((uint32_t)v == 0x84000000)
          HAL::reset();                                         // reset
        else if((uint32_t)v == 0x82000000)
        {
          shutdownSystem(kOffLed_L0R_1s, HAL::kPD_modeWOR, 5);  // WOR
          return false;
        }
        else if((uint32_t)v == 0x82100000)                    // delayed sleep/WOR
        {
          if(HAL::powerState.isUextConnected()) return false;
          timeCmdGoSleep = now;
          deviceState.isDelayedSleep = 1;
          R15 |= 0x0008;
        }
        // “естирование аварийных ситуаций
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
      else return false;
      break; */

    case 19:
      // ѕроверка разрешени€ дл€ критических операций
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
      // не выполн€ть синхронизацию при запущенном измерении
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
      
    case 60: setup2.netAddrHi = v;     		break;
    case 61: setup2.netAddrRs485_1 = v; 	break;
    case 62: setup2.netAddrRs485_2 = v; 	break;
    case 63: setup2.baudRs232 = v; 				break;
    case 64: setup2.baudRs485_1 = v; 			break;
    case 65: setup2.baudRs485_2 = v; 			break;
    case 66: setup2.IP_HostAddress = v; 	break;
    case 67: setup2.IP_GatewayAddress = v;	break;
    case 68: setup2.IP_SubnetMask = v; 		break;
    case 69: setup2.localPort = v; 				break;
    case 70: setup2.destPort = v; 				break;
    
    case 99:
      i = v & 0x00FF;                         // номер блока с 1
      if(i <= NUM_NV_BLOCKS)
      {
				if(v & 0x100)       
					nv_parameters::load(i, &R99);         // загрузка блока из NV пам€ти
				else if(v & 0x200)  
					nv_parameters::save(i, &R99);         // сохранение блока в NV пам€ти
				else
					return false;
      }
      else return false;      
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
  
  if(n >= 100  &&  n < (100 + LINK1_SLAVES_N))
  {
  	*v = setup3.slaves[n - 100];
  	return true;
  } 

  switch(n)
	{
		default: return false;

		case 0: *v = DEVICE_ID;		break;
		case 1: *v = VERSION_N;		break;
    case 2: *v = setup1.serialNumber;		break;
    
    // R3 R4 дл€ совместимости с DataIOTool
    case 3: *v = setup2.netAddrHi;     	break;
		case 4:	*v = setup2.baudRs485_1;		break;

    case 14: *v = R14;   	break;
    case 15: *v = R15;		break;

    case 21: *v = now;		break;
    
    case 60: *v = setup2.netAddrHi;     	break;
    case 61: *v = setup2.netAddrRs485_1; 	break;
    case 62: *v = setup2.netAddrRs485_2; 	break;
    case 63: *v = setup2.baudRs232; 			break;
    case 64: *v = setup2.baudRs485_1; 		break;
    case 65: *v = setup2.baudRs485_2; 		break;
    case 66: *v = setup2.IP_HostAddress; 	break;
    case 67: *v = setup2.IP_GatewayAddress;	break;
    case 68: *v = setup2.IP_SubnetMask; 	break;
    case 69: *v = setup2.localPort; 			break;
    case 70: *v = setup2.destPort; 				break;
    
    case 90:  *v = LINK1_SLAVES_N;		break;
    case 91:  *v = DATA_BUFFERS_N;		break;

    case 99:  *v = R99;		break;
	}
	return true;
}
