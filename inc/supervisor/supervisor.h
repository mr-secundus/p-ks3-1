// supervisor.h
//
// Для использования сообщений в проекте необходимо определить USE_MESSAGES.
// Иначе очередь сообщений не размещеается и не определяются функции для работы с сообщениями.
// 
// При использовании сообщений конструктору TSupervisor передается адрес байтового буфера
// для очереди сообщений и макс. размер очереди.
//

#ifndef __SUPERVISOR_H
#define __SUPERVISOR_H

#include "basetask.h"
#if defined USE_MESSAGES
	#include "tqueue.h"
#endif

class TSupervisor
{
	protected:
		TBaseTask** tasks;
		unsigned short numTasks;
#if defined USE_MESSAGES
		TQueue<TCMessage> msgQueue;
#endif
	
	public :
		
#if defined USE_MESSAGES
		TSupervisor(TBaseTask** aTasks, unsigned short n, unsigned char* messageBuffer, unsigned short messagesQueueSize) :
			tasks(aTasks),
			numTasks(n),
      msgQueue(messageBuffer, messagesQueueSize)
			{};
#else
		TSupervisor(TBaseTask** aTasks, unsigned short n) :
			tasks(aTasks),
			numTasks(n)
			{};
#endif

		bool isTaskActive(unsigned short taskId);
		void activateTask(unsigned short taskId);
		void deactivateTask(unsigned short taskId);
		void processTasks(void);

		unsigned short getTaskState(unsigned short taskId);
		void setTaskState(unsigned short taskId, unsigned short state);

#if defined USE_MESSAGES
		void processMessages(void);

		friend bool sendMessage(TEventMask msgGroup, int msgNo, long Param1, long Param2);
#endif
};

#if defined USE_MESSAGES
	bool sendMessage(TEventMask msgGroup, int msgNo, long Param1=0, long Param2=0);
#endif

#endif		// __SUPERVISOR_H
