
#include "supervisor/supervisor.h"

#if defined USE_MESSAGES
	extern TSupervisor supervisor;
#endif


//*****************************************************************************
//
void TSupervisor::processTasks(void)
{                                  
	for(short i=0; i<numTasks; i++)
		if(tasks[i]->isActive())
			tasks[i]->process();
}

void TSupervisor::activateTask(unsigned short taskId)
{                    
	//if(taskId >= numTasks) abort();
	if(!tasks[taskId]->isActive()) tasks[taskId]->activate();
#if defined DEBUG_TASKS
	puts("+"); putd(taskId); puts(" ");
#endif

}

void TSupervisor::deactivateTask(unsigned short taskId)
{                    
	//if(taskId >= numTasks) abort();
	tasks[taskId]->deactivate();
#if defined DEBUG_TASKS
	puts("-"); putd(taskId); puts(" ");
#endif
}

bool TSupervisor::isTaskActive(unsigned short taskId)
{
	return tasks[taskId]->isActive();
}

unsigned short TSupervisor::getTaskState(unsigned short taskId)
{
	return tasks[taskId]->getState();
}

void TSupervisor::setTaskState(unsigned short taskId, unsigned short state)
{
	tasks[taskId]->setState(state);
}


#if defined USE_MESSAGES
//
void TSupervisor::processMessages()
{
	TCMessage msg;

	while(!msgQueue.empty())
	{
		msg = *msgQueue.front();			

		for(short i=0; i<numTasks && !msg.discard; i++)
		{
			if(tasks[i]->isActive() &&
				 ((tasks[i]->getEventMask() & msg.msgGroup) /* || (msg.msgGroup & EV_SYSTEM)*/)
				)
				tasks[i]->handleEvent(&msg);
		}                        
		msgQueue.pop();		// remove message from queue
	}
}

//
bool sendMessage(TEventMask msgGroup, int msgNo, long Param1, long Param2)
{
	TCMessage msg(msgGroup, msgNo, Param1, Param2);
	if(!supervisor.msgQueue.full())
	{
		supervisor.msgQueue.push(&msg);
		return true;
	}
	else return false;
}
#endif		// USE_MESSAGES

