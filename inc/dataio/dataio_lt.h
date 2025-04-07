// dataio_lt.h   
//
      
#ifndef __DATAIO_LT_H
#define __DATAIO_LT_H

#include <crc16.h>
#include <tqueue.h>

namespace DataIO_Lt
{
	const uint16_t 	LoPreamble				= 0x99AA;
	const uint8_t		LoPreambleB0			= 0xAA;
	const uint8_t		LoPreambleB1			= 0x99;
	const uint16_t 	MaxDataSize				= 12;
	const uint16_t 	HeaderSize				= 4;

	const uint8_t		DataIOPreambleB0	= 0xAA;
	const uint8_t		DataIOPreambleB1	= 0x55;

	const uint16_t 	BroadcastAddress	=	126;			// широковещательный адрес
	const uint16_t 	MaxDeviceAddress	= 125;			// макс. допустимый адрес устройства
	const uint16_t 	DelayAnswerAddr		= 0x80;			// в поле адреса устанавливается b7, если ответ на сообщение
																							// должен быть отложен
	
	//---------------------------------------------------------------------------
	//
	// Прикладное сообщение - без заголовка пакета
	//
	// Значение адреса устройства может иметь значения от 1 до 125.
	// 126 - широковещательный адрес.
	// 127 - зарезервирован для исключения неоднозначностей с обработкой комбинации 
	//       DelayAnswerAddr и DataIO:BroadcastAddress (255)
	//
	// При передаче/приеме поле адреса обрабатывается следующим образом:
	//  1) биты b6..b0 - адрес устройства от 1 до 125, либо 126 (broadcast), бит b7=1:
	//     при необходимости формируется ответ на сообщение и буферизуется;
	//  2) биты b6..b0 - адрес устройства от 1 до 125, либо 126 (broadcast), бит b7=0:
	//     при необходимости формируется ответ, буферизуется, после этого выполняется
	//     передача всего ответного пакета из выходного буфера.
	//
	class TAppMessage
	{          
		public:
		uint8_t		cs,
						address;				// адрес ведомого устройства
		union										// код сообщения + параметр
		{
			struct
			{
				uint16_t	dSizeCode	: 2,		// код размера блока данных: code=size/4
								parameter	: 14;
			} cmd;
			uint8_t b[2];       
			uint16_t w;
		} cmd;
		
		uint8_t data[MaxDataSize];

		//-----------------------------------------------------		
		//
		static inline uint8_t size2dsCode(uint8_t size)
		{
			switch(size)
			{
				default:	return 0;
				case 0:		return 0;
				case 4:		return 1;
				case 8:		return 2;
				case 12:	return 3;
			}				
		}
		
		//
		static inline uint8_t dataSize(uint8_t code)	{ return (code & 0x03) * 4; } 
		inline uint8_t 				dataSize(void)				{ return dataSize(cmd.cmd.dSizeCode);	} 
		inline uint8_t 				size(void)						{ return HeaderSize + dataSize(); } 
    
    //
		static inline uint8_t calcCS(TAppMessage* m)
		{
			uint16_t crc;
			uint8_t c0, c1, sz = m->dataSize();

			crc = UPDATE_CRC16(m->address, 0xFFFF);
			crc = UPDATE_CRC16(m->cmd.b[0], crc);
			crc = UPDATE_CRC16(m->cmd.b[1], crc);

			for(int16_t i=0; i<sz; i++)
				crc = UPDATE_CRC16(m->data[i], crc);

			c0 = crc & 0xFF;
			c1 = (crc >> 8) & 0xFF;
			return c0 ^ c1;
		}
    
    //
		inline void calcCS(void)
		{
			cs = calcCS(this);
		}      
		
    //
    void set(uint8_t addr, uint8_t size, uint16_t param, uint8_t* dptr=0)
		{                  
			if(size > MaxDataSize) size = MaxDataSize;
			address = addr;    
			cmd.cmd.dSizeCode 	= size2dsCode(size);
			cmd.cmd.parameter		= param;
			
			if(dptr  &&  size)
				for(uint16_t i=0; i<size; i++) data[i] = dptr[i];
			
			calcCS();
		}
		
    //
		TAppMessage(void) {}

    //
		TAppMessage(uint8_t addr, uint8_t size, uint16_t param, uint8_t* dptr=0)
		{
			set(addr, size, param, dptr);
		}

    // 
		TAppMessage& operator=(const TAppMessage& c)
		{                                                 
			cs				= c.cs;
			address 	= c.address;
			cmd				= c.cmd;
//			uint8_t sz 	= getDataSize(cmd.cmd.dSizeCode);
			uint8_t sz 	= MaxDataSize;
			for(int16_t i=0; i<sz; i++) data[i] = c.data[i];
			return *this;
		}
	};
  

	//---------------------------------------------------------------------------
  //
  // Пакет канального уровня
  //
	typedef struct tagTPacket
	{
		uint16_t		preamble;		// преамбула
	  TAppMessage msg;			// прикладное сообщение
	  
	  tagTPacket(void) : preamble(LoPreamble) {}

		tagTPacket(uint8_t addr, uint8_t size, uint16_t param, uint8_t* dptr=0) :
			preamble(LoPreamble),
			msg(addr, size, param, dptr)
			{}

		inline uint8_t size(void) { return 2 + msg.size(); } 
	} TPacket;


  //---------------------------------------------------------------------------
	class	DataIO_Lt_Rx
	{
		public:
			// состояние приема пакета
			typedef enum {stWait=0, stPreamble2, stCS, stAddress, stCmd0, stCmd1, stRxData} TRxState;

		protected:
			TQueue<TAppMessage>*		rxQueuePtr;		// внешняя очередь принятых сообщений
		  TAppMessage rxMsg;							// буфер для текущего принимаемого сообщения
			TRxState	state;								// текущее состояние приема пакета
			uint16_t		index;                // текущий индекс записи в packet.p.msg
			int16_t			dataSize;							// длина блока данных, соответствующая cmd
			uint8_t 		rxCS,									// CS текущего принимаемого пакета
								selfAddress;					// адрес устройства

		public:
			DataIO_Lt_Rx(TQueue<TAppMessage>* arxQueuePtr) 
			{
				rxQueuePtr = arxQueuePtr;
			}

 			void configure(uint16_t aselfAddress, uint16_t abroadcastAddress)
 			{
			  selfAddress = aselfAddress;
			}

			// Установить автомат в состояние начала приема заголовка.
			// Вызывается при приеме преамбулы пакета сторонним обработчиком.
			void setPreambleRecieved(void)
			{
				state = stCS;
			}

			// сброс разборщика в исходное состояние -
			// поиск начала пакета; вызывать при инициализации и по таймауту
			inline void startRx(void)
			{
				index = 0;
				state = stWait;    
			}

			// Обработка принятого байта.
			// Возврат:
			//	0  - ожидание начала пакета
			//  1  - прием пакета
			//  2  - принят пакет
			//  3  - принят заголовок пакета DataIOLt  - если установлено enableDataIOLt
			inline int16_t rxHandler(uint8_t b)
			{          
				int16_t rc = 1;
				switch(state)
				{
					default: break;
						
					case stWait:
						if(b == LoPreambleB0) 	// первый байт преамбулы
							state = stPreamble2; 
						rc = 0;
						break;

					case stPreamble2:
						if(b == LoPreambleB1) 	// второй байт преамбулы
							state = stCS;
						else
						{
							startRx();
							if(b == DataIOPreambleB1) return 3;
						}
						break;

					case stCS:
						rxMsg.cs = b;
						state = stAddress;
						break;

					case stAddress:
						if((b & ~DelayAnswerAddr) == selfAddress  ||  b == BroadcastAddress  ||  selfAddress == 0)
						{
							rxMsg.address = b;
							state = stCmd0;
						}
						else startRx();
						break;

					case stCmd0:
						rxMsg.cmd.b[0] = b;
						state = stCmd1;
						break;

					case stCmd1:
						rxMsg.cmd.b[1] = b;
						dataSize = TAppMessage::dataSize(rxMsg.cmd.cmd.dSizeCode);
						if(dataSize < 0)				// некорректный код команды
							startRx();
						else if(dataSize == 0)	// данных нет, сообщение принято - проверить CS
						{      
							if(TAppMessage::calcCS(&rxMsg) == rxMsg.cs)
							{
								rxQueuePtr->push(&rxMsg);
								rc = 2;
							}
							startRx();
						}
						else 										// прием блока данных
						{                 
							index = 0;
							state = stRxData;
						}
						break;

					case stRxData:		// прием данных
						rxMsg.data[index++] = b;
						if(index == dataSize)
						{
							if(TAppMessage::calcCS(&rxMsg) == rxMsg.cs)
							{
								rxQueuePtr->push(&rxMsg);
								rc = 2;
							}
							startRx();
						}        
						break;
				} 
				return rc;
			}
  };    // class DataIO_Lt_Rx

};		  // namespace DataIO_Lt

#endif
