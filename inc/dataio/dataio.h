// dataio.h   
//
// modified: 13.08.11 (iss32)
//
// 13.08.11
//  - rxHandler возврашщате true при обнаружении конца пакета
//
// 13.04.12  iss32
//  - добавлен счетчик ошибок errorRxHeaderCRC
//
// TODO:
//   - счетчики ошибок layer3
//   + чтение/сброс счетчиков ошибок layer2/3
      
#ifndef __DATAIO_H
#define __DATAIO_H

#include <string.h>

#include <crc16.h>

#include "dataio_types.h"

namespace DataIO
{
	//*****************************************************************************
	//  Уровень 1 - интерфейс с каналом передачи данных
	//*****************************************************************************

	class	DataIOLayer1
	{
		protected:

		public:
			// передача кадра в канал связи
			virtual bool sendFrame(uint8_t* data, uint16_t size) { return false; }
	};


	//*****************************************************************************
	//  Уровень 2 - канальный + сетевой
	//*****************************************************************************

	const uint16_t 	Layer2Preamble			= 0x55aa;
	const uint8_t		Layer2PreambleB0		= 0xaa;
	const uint8_t		Layer2PreambleB1		= 0x55;

	// заголовок пакета уровня 2
	typedef struct tagLayer2Header
	{
		uint16_t	preamble;				// преамбула
		uint8_t		destAddress;		// адрес приемника
		uint8_t		srcAddress;			// адрес источника
		uint16_t	dataSize;				// количество байт данных
		uint16_t	crc16;					// crc заголовка
	}LR2_HEADER;

	//
	typedef union tagLayer2Packet
	{
		struct
		{
			LR2_HEADER	header;
			uint16_t			crc16;										// crc данных
			uint8_t				data[MaxLayer2DataSize];
		}Packet;

		uint8_t	bytes[sizeof(LR2_HEADER) + MaxLayer2DataSize + 2];
	}LR2_PACKET;


  //---------------------------------------------------------------------------

	class	DataIOLayer2
	{
		public:
			// свободен, идет прием данных от LR1, находится готовый пакет
			typedef enum {stFree, stReceive, stPacketReady} TRxBufferState;

		protected:
			uint16_t errors[errorTxFrame+1];

			// внутренние буферы для приема и разбора данных

			struct tagRxBuffer
			{
				LR2_PACKET			packet;
				uint16_t					index;				// текущий индекс записи
				TRxBufferState	state;

				void reset(void) {index = 0; state = stFree; }
			}buffers[2];

			volatile uint16_t activeBuffer;		// индекс текущего буфера, в который принимаются
																			// данные от LR1

			// состояние приема пакета:
			//   0 - ожидание начала пакета
			//   1 - ожидание байта 1 преамбулы
			//   2 - прием заголовка
			//   3 - ожидание байта 0 crc данных
			//   4 - ожидание байта 1 crc данных
			//   5 - прием блока данных
			uint16_t rxPacketState;

			uint16_t rxCrc16;									// текущее значение crc16 для принимаемого пакета

			uint16_t	rxTimeout;

			uint8_t 	selfAddress,						// адрес устройства
							broadcastAddress;				// адрес широковещательной рассылки

			DataIOLayer1* layer1Ptr;

		public:
			//----------  конфигурация, чтение состояния  ---------------------------

 			void configure(uint16_t aselfAddress, uint16_t abroadcastAddress,
 				uint16_t arxTimeout)
 			{
			  selfAddress = aselfAddress;
				broadcastAddress = abroadcastAddress;
				rxTimeout = arxTimeout;
			}

			// сброс разборщика пакетов в исходное состояние -
			// поиск начала пакета; вызывать при инициализации и по таймауту
			inline void resetRxBuffers(void)
			{
				buffers[0].reset();	buffers[1].reset();
				activeBuffer = 0;  	rxPacketState = 0;
			}

			uint16_t getErrors(TErrors i) { return errors [i]; }

			void clearErrors(void)			{ for(int i=0; i<=(int)errorTxFrame; i++) errors[i] = 0; }

      //-----------------------------------------------------------------------

			DataIOLayer2(DataIOLayer1* pLayer1) :
			  activeBuffer(0),
			  rxPacketState(0),
        layer1Ptr(pLayer1)
				{
          resetRxBuffers();	clearErrors();
				}

			//----------  передача  -------------------------------------------------

			// Передача пакета 3-го уровня.
			// p2 - пакет LR2 с заполненными полями
			// Packet.header.destAddress, Packet.data, Packet.header.dataSize,
			// Дописывает поля header.srcAddress, header.crc16, crc16 и передает
			// кадр для передачи на уровень 1.
			// Возврат:	true	- Ok
			//					false - ошибка передачи
			bool sendPacket(LR2_PACKET* p2)
			{
				p2->Packet.header.preamble = Layer2Preamble;
				p2->Packet.header.srcAddress = selfAddress;
				p2->Packet.header.crc16 =
					calcBlockCRC16i((uint8_t*)(&p2->Packet.header), sizeof(LR2_HEADER)-sizeof(uint16_t), 0xffff);
				p2->Packet.crc16 =
				 	calcBlockCRC16i(p2->Packet.data, p2->Packet.header.dataSize, 0xffff);

				if(layer1Ptr->sendFrame(p2->bytes, sizeof(LR2_HEADER) + sizeof(uint16_t) + p2->Packet.header.dataSize) == false)
				{
					errors[errorTxFrame]++;	return false;
				}              
				else return true;
			}


			//----------  чтение данных  --------------------------------------------

			// Возвращает количество байт данных, доступных для чтения.			
			inline uint16_t getRxDataSize(void)
			{
				uint16_t inactiveBuffer = activeBuffer^1;

				return buffers[inactiveBuffer].state == stPacketReady ?
							 buffers[inactiveBuffer].packet.Packet.header.dataSize : 0;
			}

			// Получение принятых данных.
			// Копирует принятые данные в data, возвращает количество байт данных.
			// В destAddress srcAddress записывает соотв. поля из пакета LR2.
			inline uint16_t getRxData(uint8_t* data, uint16_t* destAddress, uint16_t* srcAddress)
			{
				uint16_t inactiveBuffer = activeBuffer^1;

				if(buffers[inactiveBuffer].state == stPacketReady)
				{
					*destAddress = buffers[inactiveBuffer].packet.Packet.header.destAddress;
					*srcAddress  = buffers[inactiveBuffer].packet.Packet.header.srcAddress;
					memcpy(data, buffers[inactiveBuffer].packet.Packet.data,
					             buffers[inactiveBuffer].packet.Packet.header.dataSize);
					buffers[inactiveBuffer].state = stFree;
					return buffers[inactiveBuffer].packet.Packet.header.dataSize;
				}
				return 0;
			}

			// Обработка принятого байта.
//			inline void rxHandler(uint8_t b)
			inline bool rxHandler(uint8_t b)
			{          
				bool rc = false;
				switch(rxPacketState)
				{
					case 5:		// прием данных
						buffers[activeBuffer].packet.Packet.data[buffers[activeBuffer].index++] = b;
						rxCrc16 = UPDATE_CRC16(b, rxCrc16);     
						if(buffers[activeBuffer].packet.Packet.header.dataSize == buffers[activeBuffer].index)
						{
							if(buffers[activeBuffer].packet.Packet.crc16 == rxCrc16)
							{						
								// пакет принят
								buffers[activeBuffer].state = stPacketReady;
								activeBuffer ^= 1;
								if(buffers[activeBuffer].state != stFree)	errors[errorLossRxPacket]++;
								buffers[activeBuffer].state = stReceive;
								buffers[activeBuffer].index = 0;                                     
								rxPacketState = 0;    
								rc = true;
							}
							else // ошибка crc блока данных
							{
								errors[errorRxDataCRC]++;
								buffers[activeBuffer].index = 0;                                     
								rxPacketState = 0;
							}
						}        
						break;

					case 0:
						if(b == Layer2PreambleB0) 	// первый байт преамбулы
						{
							buffers[activeBuffer].packet.bytes[0] = b;
							buffers[activeBuffer].index++;
							rxCrc16 = UPDATE_CRC16(b, 0xffff);
							rxPacketState++;
						}
						break;

					case 1:
						if(b == Layer2PreambleB1) 	// второй байт преамбулы
						{
							buffers[activeBuffer].packet.bytes[1] = b;
							buffers[activeBuffer].index++;
							rxCrc16 = UPDATE_CRC16(b, rxCrc16);
							rxPacketState++;
						}
						else
						{
							rxPacketState = 0;
							buffers[activeBuffer].index = 0;
						}
						break;

					case 2:		// прием заголовка
						buffers[activeBuffer].packet.bytes[buffers[activeBuffer].index] = b;

            // подсчет crc заголовка, исключая поле crc16
						if(buffers[activeBuffer].index < sizeof(LR2_HEADER) - 2)
						  rxCrc16 = UPDATE_CRC16(b, rxCrc16);

            buffers[activeBuffer].index++;
						
						// принят заголовок, проверка crc и адреса приемника
						if(buffers[activeBuffer].index == sizeof(LR2_HEADER))
						{
							if(buffers[activeBuffer].packet.Packet.header.dataSize > MaxLayer2DataSize)
							{
								rxPacketState = 0;								// иначе начать прием следующего пакета
								buffers[activeBuffer].index = 0;
							}
							else
							{
								uint16_t address = buffers[activeBuffer].packet.Packet.header.destAddress;
/*								if(rxCrc16 == buffers[activeBuffer].packet.Packet.header.crc16  &&
	  							 (address == selfAddress  ||  address == broadcastAddress))
									rxPacketState++;	// crc ok; адрес наш или широковещательный - 
								else								// продолжаем прием
								{
									rxPacketState = 0;								// иначе начать прием следующего пакета
									buffers[activeBuffer].index = 0;
								} */
								// @13.04.12 iss32
								if(rxCrc16 == buffers[activeBuffer].packet.Packet.header.crc16)
								{
									if(address == selfAddress  ||  address == broadcastAddress)
										rxPacketState++;	// адрес наш или широковещательный - 
									else									// продолжаем прием
									{
										rxPacketState = 0;								// иначе начать прием следующего пакета
										buffers[activeBuffer].index = 0;
									}
								}
								else
								{  
									errors[errorRxHeaderCRC]++;
									rxPacketState = 0;									// иначе начать прием следующего пакета
									buffers[activeBuffer].index = 0;
								}
							}
						}
						break;

					case 3:		// прием байта 0 crc данных
						buffers[activeBuffer].packet.Packet.crc16 = b;
						rxPacketState = 4;
						break;

					case 4:		// прием байта 1 crc данных
						buffers[activeBuffer].packet.Packet.crc16 |= b << 8;
						rxPacketState = 5;
						rxCrc16 = 0xffff;			// инициализировать crc перед расчетом crc данных
						buffers[activeBuffer].index = 0;		// index'ом считаем количество байт данных
						break;
				} 
				return rc;
			}
  };    // class DataIOLayer2


	//*****************************************************************************
	//  Уровень 3 - представление данных
	//*****************************************************************************
	
	//
	// Передача/прием данных в виде сообщений фиксированных и прикладных типов.
	//
	// Передача: отдельные сообщения заносятся через addMessage во внутренний буфер,
	// затем вызовом sendPacket пакет передается на уровень 2 и т.д.
	//
	// Прием: через вызов readPacket происходит обращение к уровню 2 - при наличии
	// блок данных копируется в буфер, затем через вызовы getMessage последовательно
	// считываются сообщения из блока.
	//	
	class DataIOLayer3
	{
		protected:                                             
			DataIOLayer2	*lr2Ptr;
			
			LR2_PACKET	txPacket;			// буфер для передаваемого пакета
			uint16_t			txIndex;			// текущий индекс в txPacket.Packet.data
			
			uint8_t 			rxData[MaxLayer2DataSize];	// приемный буфер
			uint16_t			rxIndex,			// текущий индекс в rxData
									rxSize;				// количество байт данных в rxData
			
		public:

			DataIOLayer3(DataIOLayer2 *lr2) :
				lr2Ptr(lr2)
				{}
			   
			//---------  конфигурация, чтение состояния  ----------------------------

			//---------  передача  --------------------------------------------------
			
			// Сброс выходного буфера
			void resetTxBuffer(void) { txIndex = 0; };

			// Возвращает размер свободного места в выходном буфере
			inline uint16_t getTxBufferFree(void) { return MaxLayer2DataSize - txIndex; }

			// Возвращает текущее число байт в выходном буфере 
			inline uint16_t getTxDataSize(void) { return txIndex; }

			// Добавить сообщение в пакет для передачи.
			//
			// msgPtr - указатель на структуру одного из типов:
			//          MsgParameterWrite, MsgParameterRead, MsgParameterValue, MsgUserType
			//          Для сообщений MsgParameter.. структура содержит всё сообщение,
			//          для MsgUserType - заголовок сообщения
			// data   - указатель на блок данных для UserType; может располагаться в памяти
			//          независимо от заголовка сообщения
			//
			// возврат: 1 - сообщение добавлено в пакет
			//          0 - нет места в буфере или неизвестный тип сообщения
			bool addMessage(uint8_t* msgPtr, uint8_t* data)
			{       
				uint16_t size;
				
				switch(*msgPtr)
				{
					default: return false;
					
					case MsgParameterWrite:		size = sizeof(TMsgParameterWrite);	break;
					case MsgParameterRead:		size = sizeof(TMsgParameterRead);		break;
					case MsgParameterValue:		size = sizeof(TMsgParameterValue);	break;
					
					case MsgAppType:
						size = sizeof(TMsgAppTypeHeader) + ((TMsgAppTypeHeader*)msgPtr)->dataSize;
						if(size > getTxBufferFree()) return false;

						memcpy(txPacket.Packet.data + txIndex, msgPtr, sizeof(TMsgAppTypeHeader));
						txIndex += sizeof(TMsgAppTypeHeader);

						if(((TMsgAppTypeHeader*)msgPtr)->dataSize)
						{
							memcpy(txPacket.Packet.data + txIndex, data, ((TMsgAppTypeHeader*)msgPtr)->dataSize);
							txIndex += ((TMsgAppTypeHeader*)msgPtr)->dataSize;
						}
						return true;
				}

				if(size > getTxBufferFree()) return false;

				memcpy(txPacket.Packet.data + txIndex, msgPtr, size);
				txIndex += size;
				return true;
			}

			// Передача сформированного пакета.
			// После успешной передачи выходной буфер сбрасывается.
			// destAddress - адрес приемника
			// возврат:  0 - Ok
			//          -1 - ошибка передачи пакета через канал связи
			//          -2 - нет данных для передачи
			int16_t sendPacket(uint16_t destAddress)
			{
				if(txIndex)
				{
					txPacket.Packet.header.destAddress = 	destAddress;
					txPacket.Packet.header.dataSize = 		txIndex;
					if(lr2Ptr->sendPacket(&txPacket))
					{
						txIndex = 0;	return 0;
					}
					else return -1;
				}
				else return -2;
			}

			//---------  прием  -----------------------------------------------------

			// получение данных от нижележащего уровня протокола
			// возвращает количество байт данных в приемном буфере
			// xxAddress - адреса получателя и отправителя из пакета 2-го уровня
			inline uint16_t readPacket(uint16_t* destAddress, uint16_t* srcAddress)
			{
				rxIndex = 0;
				return rxSize = lr2Ptr->getRxData(rxData, destAddress, srcAddress);
			}
			
			// возвращает количество байт данных в приемном буфере
			inline uint16_t	rxDataSize(void) { return rxSize - rxIndex; }
			
			// получение очередного сообщения из приемного буфера
			// msgIdPtr - принимает идентификатор сообщения
			// data 		- буфер для приема сообщения
			// возврат:  0 - Ok
			//          -1 - нет данных для обработки
			//          -2 - неизвестный идентификатор сообщения
			//          -3 - некорректное значение поля size сообщения;
			//               при этом в msgIdPtr заносится указатель на заголовок
			//               сообщения, но копирование данных не выполняется
			int16_t getMessage(uint8_t* msgIdPtr, uint8_t* data)
			{
				if(rxDataSize() == 0) return -1;
				
				uint16_t size;
				uint8_t id = rxData[rxIndex];
				
				switch(id)
				{
					default: return -2;
					
					case MsgParameterWrite:		size = sizeof(TMsgParameterWrite);	break;
					case MsgParameterRead:		size = sizeof(TMsgParameterRead);		break;
					case MsgParameterValue:		size = sizeof(TMsgParameterValue);	break;
					
					case MsgAppType:                 
						size = sizeof(TMsgAppTypeHeader) + ((TMsgAppTypeHeader*)&rxData[rxIndex])->dataSize;
						break;
				}
				
				*msgIdPtr = rxData[rxIndex];

				if(size > rxDataSize()) return -3;
				
				memcpy(data, rxData + rxIndex, size);
				rxIndex += size;
				return 0;
			}
	};
};		  // namespace DataIO

#endif
