// dataio3.h   
//
// 27.06.22  iss53c-p_cp_test1
// В DataIOLayer3 добавлено setRxAppData() - дополнение для работы с пакетами
// сообщений без заголовка пакета - разбор данных в составе пакетов UDP.
//
// 11.05.2017 i52
//  - в DataIOLayer3 добавлен конструктор без аргументов и метод install для передачи указателя
//    на layer2 - для возможности статического размещения DataIOLayer3 как поля класса.
//
// 19.09.2015  ai16
//  - дополнены коды возврата rxHandler для ошибочных ситуаций.
//  - в rxHandler добавлена обработка повторного состояния stPacketReady - возникает
//    при приеме более одного пакета в одном блоке данных.
//
// 21.05.15 (iss32-i7)
//  - Внесены изменения из dataio2.h: исключен layer1, отправка пакета выполняется прикладной программой.
//  - Исключены два приемных буфера в Layer2.
//  - Исключены операции копирования данных, при разборе выполняется получение указателей
//    на внутренний буфер Layer2.
//  - Изменен код возврата Layer3::addMessage.
// 
// 13.08.11
//  - rxHandler возврашщате true при обнаружении конца пакета
//
// 13.04.12  iss32
//  - добавлен счетчик ошибок errorRxHeaderCRC
//
// 23.09.13  iss32-1
//  - в rxHandler всегда возвращался код true - исправлено на возврат rc,
//    возвращает true только при приеме пакета.
//
      
#ifndef __DATAIO3_H
#define __DATAIO3_H

#include <string.h>
#include <crc16.h>
#include "dataio_types.h"
#include "dataio_app_types.h"

namespace DataIO
{
	//*****************************************************************************
	//  Уровень 2 - канальный + сетевой
	//*****************************************************************************

	const uint16_t 	Layer2Preamble				= 0x55AA;
	const uint8_t		Layer2PreambleB0			= 0xAA;
	const uint8_t		Layer2PreambleB1			= 0x55;
	const uint16_t 	Layer2PreambleCRC16		= 0xB75A;		// CRC16 для преамбулы AA 55

	const uint8_t		DataIOLtPreambleB0		= 0xAA;
	const uint8_t		DataIOLtPreambleB1		= 0x99;
	

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

		uint8_t	bytes[sizeof(LR2_HEADER) + MaxLayer2DataSize + 2];	// sizeof(Packet)
	}LR2_PACKET;

  //---------------------------------------------------------------------------
	class	DataIOLayer2
	{
		public:
			// состояние приема пакета:
			//   0 - ожидание начала пакета
			//   1 - ожидание байта 2 преамбулы
			//   2 - прием заголовка
			//   3 - ожидание байта 0 crc данных
			//   4 - ожидание байта 1 crc данных
			//   5 - прием блока данных
			typedef enum {stWait=0, stPreamble2, stRxHeader, stCRC0, stCRC1, stRxData, stPacketReady} TRxState;

		protected:
			uint16_t errors[errorTxFrame+1];

			LR2_PACKET	packet;

			TRxState		state;							// текущее состояние приема пакета

			uint16_t 	rxCrc16,								// текущее значение crc16 для принимаемого пакета
							index;                  // текущий индекс записи в packet.bytes

			uint8_t 	selfAddress,						// адрес устройства
							broadcastAddress;				// адрес широковещательной рассылки  
							
			bool		enableDataIOLt;					// разрешить выход из обработчика по приему преамбулы пакета DataIOLt

		public:
			//----------  конфигурация, чтение состояния  ---------------------------

 			void configure(uint16_t aselfAddress, uint16_t abroadcastAddress)
 			{
			  selfAddress = aselfAddress;
				broadcastAddress = abroadcastAddress;
			}

			// сброс разборщика пакетов в исходное состояние -
			// поиск начала пакета; вызывать при инициализации и по таймауту
			inline void startRx(void)
			{
				index = 0;
				state = stWait;    
			}

			uint16_t getErrors(TErrors i) { return i <= errorTxFrame ? errors [i] : 0; }

			void clearErrors(void)			{ for(int i=0; i<=(int)errorTxFrame; i++) errors[i] = 0; }
			
			// Установить разрешение обработки преамбулы пакета DataIOLt
			void enableDataIOLt_PreambleProcess(bool v)
			{
				enableDataIOLt = v;
			}        

			// Установить автомат в состояние начала приема заголовка.
			// Вызывается при приеме преамбулы пакета сторонним обработчиком.
			void setPreambleRecieved(void)
			{
				index = 2;
				rxCrc16 = Layer2PreambleCRC16;
				state = stRxHeader;
			}

      //-----------------------------------------------------------------------

			DataIOLayer2(void)
			{                     
				enableDataIOLt = false;
        startRx();
        clearErrors();
			}

			//----------  передача  -------------------------------------------------

			// Завершение формирования исходящего пакета.
			// p2 - пакет LR2 с заполненными полями Packet.data, Packet.header.dataSize,
			// Дописывает поля header.srcAddress, header.destAddress, 
			// вычисляет CRC и заносит в header.crc16, crc16.
			void finishTxPacket(uint8_t destAddress, LR2_PACKET* p2)
			{
				p2->Packet.header.preamble 		= Layer2Preamble;
				p2->Packet.header.srcAddress 	= selfAddress;
				p2->Packet.header.destAddress = destAddress;
				p2->Packet.header.crc16 =
					calcBlockCRC16i((uint8_t*)(&p2->Packet.header), sizeof(LR2_HEADER)-sizeof(uint16_t), 0xffff);
				p2->Packet.crc16 =
				 	calcBlockCRC16i(p2->Packet.data, p2->Packet.header.dataSize, 0xffff);
			}


			//----------  чтение данных  --------------------------------------------

			// Возвращает количество байт данных, доступных для чтения.			
			inline uint16_t getRxDataSize(void)
			{
				return state == stPacketReady ? index : 0;
			}

			inline uint16_t getRxIndex(void) { return index; };

			// Получение принятых данных.
			// Копирует принятые данные в data, возвращает количество байт данных.
			// В destAddress srcAddress записывает соотв. поля из пакета LR2.
			inline uint16_t getRxData(uint8_t* data, uint16_t* destAddress, uint16_t* srcAddress)
			{
/*				if(buffers[inactiveBuffer].state == stPacketReady)
				{
					*destAddress = buffers[inactiveBuffer].packet.Packet.header.destAddress;
					*srcAddress  = buffers[inactiveBuffer].packet.Packet.header.srcAddress;
					memcpy(data, buffers[inactiveBuffer].packet.Packet.data,
					             buffers[inactiveBuffer].packet.Packet.header.dataSize);
					buffers[inactiveBuffer].state = stFree;
					return buffers[inactiveBuffer].packet.Packet.header.dataSize;
				} */
				return 0;
			}

			// Получение принятых данных.
			// В data заносится указатель на _блок_данных_пакета_ во внутреннем буфере.
			// В destAddress и srcAddress заносятся соотв. значения из принятого пакета.
			// Возвращает размер _блока_данных_.
			inline uint16_t getRxDataPtr(uint8_t** data, uint16_t* destAddress, uint16_t* srcAddress)
			{
				if(state != stPacketReady) return 0;
				*destAddress = packet.Packet.header.destAddress;
				*srcAddress  = packet.Packet.header.srcAddress;
				*data 			 = packet.Packet.data;
				return packet.Packet.header.dataSize;
			}
      
			// Обработка принятого байта.
			// Возврат:
			//	0  - ожидание начала пакета
			//  1  - прием пакета
			//  2  - принят пакет
			//  3  - принят заголовок пакета DataIOLt  - если установлено enableDataIOLt
			// -1  - ошибка crc блока данных            
			// -2  - ошибка формата заголовка
			// -3  - чужой адрес получателя
			// -4  - ошибка crc заголовка
			// -99 - ошибка логики работы автомата
			inline int16_t rxHandler(uint8_t b)
			{          
				int16_t rc = 1;
				switch(state)
				{
					default: 
						rc = -99;
	          startRx();
						break;
						
					case stPacketReady:				// пакет принят
						rc = 2; 
						break;

					case stRxData:		// прием данных
						packet.bytes[index++] = b;
						rxCrc16 = UPDATE_CRC16(b, rxCrc16);     
						if(index == packet.Packet.header.dataSize + sizeof(LR2_HEADER) + sizeof(uint16_t))
						{
							if(packet.Packet.crc16 == rxCrc16)		// пакет принят
							{						
								state = stPacketReady;
								rc = 2;
							}
							else 																	// ошибка crc блока данных
							{
								errors[errorRxDataCRC]++;
			          startRx();
			          rc = -1;
							}
						}        
						break;

					case stWait:
						if(b == Layer2PreambleB0) 	// первый байт преамбулы
						{
							packet.bytes[0] = b;
							state = stPreamble2;
						}       
						rc = 0;
						break;

					case stPreamble2:
						if(b == Layer2PreambleB1) 	// второй байт преамбулы
						{
							packet.bytes[1] = b;
							index = 2;
							rxCrc16 = Layer2PreambleCRC16;
							state = stRxHeader; 
						}                                                                                    
						else 
						{
							startRx();
							if(enableDataIOLt  &&  b == DataIOLtPreambleB1) 	 // второй байт преамбулы пакета протокола DataIOLt
								rc = 3;
						}
						break;

					case stRxHeader:		// прием заголовка
						packet.bytes[index] = b;

            // подсчет crc заголовка, исключая поле crc16
						if(index < sizeof(LR2_HEADER) - 2)
						  rxCrc16 = UPDATE_CRC16(b, rxCrc16);

            index++;
						
						// принят заголовок, проверка crc и адреса приемника
						if(index == sizeof(LR2_HEADER))
						{
							if(packet.Packet.header.dataSize > MaxLayer2DataSize)
							{
			          startRx();
			          rc = -2;
			         }
							else
							{
								uint16_t address = packet.Packet.header.destAddress;
								if(rxCrc16 == packet.Packet.header.crc16)
								{
									if(address == selfAddress  ||  address == broadcastAddress)
										state = stCRC0;			// адрес наш или широковещательный - продолжаем прием
									else  
									{
					          startRx();
					          rc = -3;
					        }
								}
								else
								{  
									errors[errorRxHeaderCRC]++;
				          startRx();
				          rc = -4;
								}
							}
						}
						break;

					case stCRC0:		// прием байта 0 crc данных
						packet.bytes[index++] = b;
						state = stCRC1;
						break;

					case stCRC1:		// прием байта 1 crc данных
						packet.bytes[index++] = b;
						state = stRxData;
						rxCrc16 = 0xFFFF;				// инициализировать crc перед расчетом crc данных
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
			
			LR2_PACKET	*txPacket;		// буфер для передаваемого пакета
			uint16_t			txIndex;			// текущий индекс в txPacket.Packet.data
			
			uint8_t 			*rxDataPtr;		// указатель на пакет в буфере Layer2
			uint16_t			rxIndex,			// текущий индекс в rxData
									rxSize;				// количество байт данных в rxData
									
			uint16_t			maxTxAppSize;		// макс. размер прикладных данных в выходном пакете
			
		public:

			DataIOLayer3(DataIOLayer2 *lr2) :
				lr2Ptr(lr2),
				maxTxAppSize(MaxLayer2DataSize)
				{}

			DataIOLayer3(void) : 
				lr2Ptr(0),
				maxTxAppSize(MaxLayer2DataSize)
				{}      
				
			void install(DataIOLayer2 *lr2)
			{
				lr2Ptr = lr2;
			}                                                     
			
			// Устанавливает макс. размер данных для исходящего пакета
			void setTxAppSize(uint16_t sz)
			{
				maxTxAppSize = sz;
			}

			uint32_t getTxAppSize(void)
			{
				return maxTxAppSize;
			}
			   
			//---------  передача  --------------------------------------------------
			
			// Возвращает размер свободного места в выходном буфере
//			inline uint16_t getTxBufferFree(void) { return MaxLayer2DataSize - txIndex; }
			inline uint16_t getTxBufferFree(void) { return maxTxAppSize - txIndex; }

			// Возвращает текущее число байт в выходном буфере 
			inline uint16_t getTxDataSize(void) { return txIndex; }

			// начать формирование выходного пакета
			void startTxPacket(LR2_PACKET* buffer)
			{                        
				txIndex = 0;                                  
				txPacket = buffer;
			}
			
			// Добавить сообщение в пакет для передачи.
			//
			// msgPtr - указатель на структуру одного из типов:
			//          MsgParameterWrite, MsgParameterRead, MsgParameterValue, MsgUserType
			//          Для сообщений MsgParameter.. структура содержит всё сообщение,
			//          для MsgUserType - заголовок сообщения
			// data   - указатель на блок данных для UserType; может располагаться в памяти
			//          независимо от заголовка сообщения
			//
			// возврат:  0 - сообщение добавлено в пакет
			//          -1 - нет места в буфере
			//          -2 - неизвестный тип сообщения
			int16_t addMessage(uint8_t* msgPtr, uint8_t* data)
			{       
				uint16_t size;
				
				switch(*msgPtr)
				{
					default: return -2;
					
					case MsgParameterWrite:		size = sizeof(TMsgParameterWrite);	break;
					case MsgParameterRead:		size = sizeof(TMsgParameterRead);		break;
					case MsgParameterValue:		size = sizeof(TMsgParameterValue);	break;
					case MsgReadBlock:				size = sizeof(TMsgReadBlock);				break;
					
					case MsgAppType:
						size = sizeof(TMsgAppTypeHeader) + ((TMsgAppTypeHeader*)msgPtr)->dataSize;
						if(size > getTxBufferFree()) return -1;

						memcpy(txPacket->Packet.data + txIndex, msgPtr, sizeof(TMsgAppTypeHeader));
						txIndex += sizeof(TMsgAppTypeHeader);

						if(((TMsgAppTypeHeader*)msgPtr)->dataSize)
						{
							memcpy(txPacket->Packet.data + txIndex, data, ((TMsgAppTypeHeader*)msgPtr)->dataSize);
							txIndex += ((TMsgAppTypeHeader*)msgPtr)->dataSize;
						}
						return 0;
				}

				if(size > getTxBufferFree()) return -1;

				memcpy(txPacket->Packet.data + txIndex, msgPtr, size);
				txIndex += size;
				return 0;
			}

			// Завершение формирования пакета для передачи.
			// В пакет записывается длина данных и общая длина пакета и рассчитываются crc16.
			// Возвращает общий размер пакета.
			uint16_t finishTxPacket(uint8_t destAddress)
			{
				if(txIndex)
				{
					txPacket->Packet.header.dataSize = txIndex;
					lr2Ptr->finishTxPacket(destAddress, txPacket);
					txIndex = 0;
					return sizeof(LR2_HEADER) + sizeof(uint16_t) + txPacket->Packet.header.dataSize;
				} 
				else return 0;
			}

			//---------  прием  -----------------------------------------------------

			// получение данных от нижележащего уровня протокола
			// возвращает количество байт данных в приемном буфере
			// xxAddress - адреса получателя и отправителя из пакета 2-го уровня
			inline uint16_t readPacket(uint16_t* destAddress, uint16_t* srcAddress)
			{
				rxIndex = 0;
				return rxSize = lr2Ptr->getRxDataPtr(&rxDataPtr, destAddress, srcAddress);
			}
			
			// возвращает количество байт данных в приемном буфере
			inline uint16_t	rxDataSize(void) { return rxSize > rxIndex  ?  rxSize - rxIndex  :  0; }
			
			// получение очередного сообщения из приемного буфера
			// msgIdPtr - принимает идентификатор сообщения
			// msgPtr		- указатель, в котором возвращается адрес начала сообщения в локальном буфере
			// возврат:  0 - Ok
			//          -1 - нет данных для обработки
			//          -2 - неизвестный идентификатор сообщения
			//          -3 - некорректное значение поля size сообщения;
			//               при этом в msgIdPtr заносится указатель на заголовок
			//               сообщения, но копирование данных не выполняется
			int16_t getMessagePtr(uint8_t* msgIdPtr, uint8_t** msgPtr)
			{                 
				uint16_t ds = rxDataSize();
				if(ds == 0) return -1;
				uint16_t size;

				switch(rxDataPtr[rxIndex])		// message Id
				{
					default: return -2;
					case MsgParameterWrite:		size = sizeof(TMsgParameterWrite);	break;
					case MsgParameterRead:		size = sizeof(TMsgParameterRead);		break;
					case MsgParameterValue:		size = sizeof(TMsgParameterValue);	break;
					case MsgReadBlock:				size = sizeof(TMsgReadBlock);				break;
					
					case MsgAppType:                 
						size = sizeof(TMsgAppTypeHeader) + ((TMsgAppTypeHeader*)&rxDataPtr[rxIndex])->dataSize;
						break;
				}

				if(size > ds) return -3;
				*msgIdPtr 	= rxDataPtr[rxIndex];
				*msgPtr 		= rxDataPtr + rxIndex;
				rxIndex 	 += size;
				return 0;
			}      
			
			
			//
			// Дополнение для работы с пакетами сообщений без заголовка пакета.
			// Передает указатель на внешний буфер с пакетом для последующего разбора.
			//
			// buffer			указатель на буфер, содержащий сообщения.
			// n					число байт данных в буфере.
			//
			void setRxAppData(uint8_t* buffer, uint16_t n)
			{                        
				rxDataPtr = buffer;
				rxIndex = 0;
				rxSize = n;
			}
	};
};		  // namespace DataIO

#endif
