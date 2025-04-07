// p_mastertx.cpp
// 
// Реализация отправки запросов к ведомым устройствам
//
// 23.08.22 i54-3-control
// Добавлено формирование исходящих сообщений типа MsgAppType.
//
// TODO:
// + реализация уведомления о приеме ответа при опросе в виде блоков данных - 
//   сейчас реализовано обработка ответа в виде значений отдельных регистров.

#include <stdio.h>
#include "hal_base.h"
#include "supervisor/supervisor.h"
#include "protocol/p_tmastertx.h"

//
void TMasterTx::process(void)
{           
	bool rqAnswer=false;
	
	switch(state)
	{       
		default: break;
		
		// idle, ожидание поступления пакета для передачи
		case stReadyToTx:					
			if(txInterface->getTxFree(id)  &&  !txQueue->empty())
			{
				// получить из очереди очередной запрос и добавить в выходной буфер соотв. сообщение
				TTxRequest* rq = txQueue->front();
				address = rq->address;
				protocol->startTxPacket();
				if(rq->flags & TTxRequest::FRqAnswer) rqAnswer = true;
				addMessage(rq, rq->flags & TTxRequest::FRqAnswer ? true : false);
				txQueue->pop();
					
				// получить из очереди все очередные запросы на тот же адрес
				if(!(rq->flags & TTxRequest::FBreakPacket))
				{
					bool theend;
					do
					{           
						theend = true;          
						if(!txQueue->empty())
						{
							TTxRequest* rq = txQueue->front();
							if(address == rq->address)
							{
								addMessage(rq, rq->flags & TTxRequest::FRqAnswer ? true : false);
								txQueue->pop();
								if(rq->flags & TTxRequest::FRqAnswer) rqAnswer = true;
								if(!(rq->flags & TTxRequest::FBreakPacket)) theend = false;
							}
						}
					}while(!theend  &&  txSize < MaxTxSize);     
				}
				
				// txSize = protocol->getProtocol->finishTxPacket();
				txSize = protocol->sendPacket(address);
				
				// TODO: обработка отказа передачи пакета
				
				if(rqAnswer)
				{        
					timer = now;
					state = stWaitAnswer;			// ожидание ответа от устройства
				}
			}
      break;    
      
		// отработка таймаута ожидания ответа от устройства
		case stWaitAnswer:			
			if(TIMEOUT(timer, timeoutSlaveAnswer))
			{
//				if(callback) callback->proc(0, address, 0); 
				state = stReadyToTx;
				timer = now;
			}
			break;

		// отработка таймаута приема сообщений (ожидание конца пакета)
		case stWaitRxComplete:			
			if(TIMEOUT(timer, timeoutEndPacket))
			{
				state = stReadyToTx;
				timer = now;
			}
			break;
	}
}


//-----------------------------------------------------------------------------
//
void TMasterTx::addMessage(TTxRequest* rq, bool rqAnswer)
{            
	switch(rq->type)
	{
		default: break;
			
		case DataIO::MsgParameterWrite:
			{
				DataIO::TMsgParameterWrite msg;
				msg.id 								= DataIO::MsgParameterWrite;
				msg.sequenceNumber		= rqAnswer ? getSeqNumber() : 0xff;
				msg.parameterNumber 	= rq->parameter0;
				msg.parameterValue 		= rq->parameter1;
				
				protocol->getProtocol()->addMessage((uint8_t*)&msg, 0);
				txSize = protocol->getProtocol()->getTxDataSize();
			}
			break;

		case DataIO::MsgParameterRead:
			{
				DataIO::TMsgParameterRead msg;
				msg.id 								= DataIO::MsgParameterRead;
				msg.sequenceNumber		= rqAnswer ? getSeqNumber() : 0xff;
				msg.parameterNumber 	= rq->parameter0;
				
				protocol->getProtocol()->addMessage((uint8_t*)&msg, 0);
				txSize = protocol->getProtocol()->getTxDataSize();
			}
			break;

		case DataIO::MsgReadBlock:
			{                                             
				DataIO::TMsgReadBlock msg;
				msg.id 								= DataIO::MsgReadBlock;
				msg.sequenceNumber		= rqAnswer ? getSeqNumber() : 0xff;
				msg.base 							= rq->parameter0;
				msg.size 							= rq->parameter1;
				
				protocol->getProtocol()->addMessage((uint8_t*)&msg, 0);
				txSize = protocol->getProtocol()->getTxDataSize(); 
			}
			break;   

		case DataIO::MsgAppType:
			{                                             
				DataIO::TMsgAppTypeHeader msg;
				msg.id								= DataIO::MsgAppType;
				msg.sequenceNumber		= rqAnswer ? getSeqNumber() : 0xff;
			  msg.type							= rq->parameter0 >> 16;
			  msg.dataSize					= rq->parameter0 & 0xFFFF;     
			  
			  // Временная переменная для передачи данных в выходное сообщение.
			  // В Layer3::addMessage() выполняется копирование data в выходное сообщение,
			  // далее не используется.
			  uint32_t data = rq->parameter1;

				protocol->getProtocol()->addMessage((uint8_t*)&msg, (uint8_t*)&data);
				txSize = protocol->getProtocol()->getTxDataSize(); 
			}
			break;   
			
	}
}

//-----------------------------------------------------------------------------
/*	
// Уведомление о приеме данных по обслуживаемому интерфесйу
void TMasterTx::onRxSlaveData(void)
{                                                                
	state = ST_WAIT_END_RX_PACKET;
	timer = now;
}

// Уведомление о приеме пакета от ведомого устройства
// address		адрес вед. устр-ва
void TMasterTx::onRxSlavePacket(uint16_t address)
{                                  
	if(state == ST_WAIT_ANSWER  ||  state == ST_WAIT_END_RX_PACKET)
	{
		state = ST_WAIT_END_RX_PACKET;		// на отработку ожидания завершения приема пакета
		timer = now;
	}
}  */
 
