// sim_data.h
//
// »митаци€ блоков данных от нижнего уровн€ дл€ тестировани€ опроса
// блоков измерени€ и передачи данных на ¬”

#ifndef SIM_DATA_H
#define SIM_DATA_H

#include "hal.h"
#include "dataio/tdatabuffer.h"
#include "cfg/slave_io.h"

class SimData
{
private:
	// ћакс. число имитируемых устройств
	static constexpr uint16_t kMaxSlaves = SLAVE_IO_SLAVES_N;
	// ћакс. число отсчетов данных в блоке
	static constexpr uint32_t kSamplesN =
			TDataBuffer::kMaxDataSize / kSampleSize_Int32x3;
	
	
	static const int32_t
		kSampleInc = 1000,											// инкремент значени€ отсчета
		kSampleVmin = -kSampleInc * 2000,				// мин. значение отсчета  
		kSampleVmax =  kSampleInc * 2000; 			// макс. значение отсчета
	
	using sample_x3_t = struct tag_sample_x3	// трехканальный отсчет данных
	{
		int32_t x, y, z;
	}; 

	using slave_t = struct					// данные дл€ одного устройства
	{
		uint16_t	address,						// сетевой адрес
							packetNumber;				// текущий номер пакета
		uint32_t	packetPeriod,				// длительность одного блока данных, мс
							sync,								// текущее значение sync
							packetCounter,			// счЄтчик сформированных пакетов
							time;								// врем€ формировани€ предыдущего пакета
		sample_x3_t	v;								// текущие значени€ данных
	};
	
	slave_t slaves[kMaxSlaves];
	
public:
	SimData(void)
	{
		for(auto& x : slaves)
		{
			x.address = 0;
		}
		reset(now);
	}

	// —брос состо€ни€ устройств
	// ¬ызывать при старте ввода данных
	//
	// tm				врем€ пуска формировани€ данных (sync дл€ первого отсчета)
	void reset(uint32_t tm)
	{
		for(auto& x : slaves)
		{
			x.packetNumber = 0;
			x.packetCounter = 0;
			x.sync = 0;
			x.time = tm;
			x.v.x = x.v.y = x.v.z = 0;
		}
	}
	
	
	// »нициализаци€ адресов устройств
	//
	// address				массив адресов
	void setSlavesAddress(uint16_t *address)
	{
		for(uint16_t i = 0; i < kMaxSlaves; i++)
			slaves[i].address = address[i];
	}


	// ”становка Fd
	// ѕо Fd вычисл€етс€ длительность одного блока данных
	//
	// fd				частота дискретизации, √ц * 1000
	//
	// return		true  	Ok
	//					false		некорректное значение fd
	bool setFd(uint32_t fd)
	{
		if(fd >= 1000  &&  fd <= 4000000)
		{
			uint32_t period_x1000 = 1000000 / fd;
			uint32_t packetPeriod = period_x1000 * kSamplesN;   
		
			for(uint16_t i = 0; i < kMaxSlaves; i++)
				slaves[i].packetPeriod = packetPeriod;
			
			return true;
		}
		else return false;
	}
	
	
	// ¬озвращает флаг готовности данных дл€ выбранного устройства
	// 
	// ѕровер€ет интервал времени с момента старта или формировани€ предыдущего 
	// блока данных. 
	//
	// address			адрес устройства
	// tm						текущее врем€
	//
	// return				true		готовность данных
	//							false		нет данных или некорректный адрес
	bool slaveReady(uint16_t address, uint32_t tm)
	{
		for(uint16_t i = 0; i < kMaxSlaves; i++)
			if(address == slaves[i].address)
			{
				if((tm - slaves[i].time) >= slaves[i].packetPeriod)
					return true;
				else
					return false;
			}
						
		return false;
	}
	
	
	// «аполн€ет блок данных и устанавливает packet и sync
	//
	// address			адрес устройства
	// tm						текущее врем€
	// b						блок дл€ заполнени€
	//
	// return				true		Ok
	//							false		нет данных или некорректный адрес
	bool getData(uint16_t address, uint32_t tm, TDataBuffer* b)
	{
		uint16_t i;
		for(i = 0; i < kMaxSlaves; i++)
			if(address == slaves[i].address)
				break;
						
		if(i == kMaxSlaves)
			return false;
		
		slave_t *s = &slaves[i];

		b->size = 0;
		
		while(b->free<sample_x3_t>() > 0)
		{
			b->write((uint8_t*)&s->v, sizeof(s->v));
			s->v.x += kSampleInc;
			s->v.y += kSampleInc;
			s->v.z += kSampleInc;
		}
		
		if(s->v.x > kSampleVmax) s->v.x = kSampleVmin;
		if(s->v.y > kSampleVmax) s->v.y = kSampleVmin;
		if(s->v.z > kSampleVmax) s->v.z = kSampleVmin;

		b->packet = s->packetNumber++;
		b->sync   = s->sync;
		
		if(s->packetNumber > 1000)
			s->packetNumber = 1;
		
		s->time = now;
//		s->sync += s->packetPeriod;
		s->sync = now;
		s->packetCounter++;
		
		return true;
	}
};

#endif		// SIM_DATA_H
