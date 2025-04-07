#ifndef __TASKDEF_H
#define __TASKDEF_H

#include <port.h>
#include "events.h"
#include "supervisor/supervisor.h"

static const TMsgMask	TaskMask	=	0xffffffff;

#define DECLARE_TASK(TaskName, msgMask)							\
	class TaskName : public TBaseTask									\
	{																									\
		public: 																				\
			TaskName(void) : TBaseTask(msgMask) {};				\
			virtual void activate(void);									\
			virtual void deactivate(void);								\
			virtual void handleEvent(TCMessage* msg);			\
			virtual void process(void);										\
			virtual void setState(uint16_t);							\
	};                 

//*****************************************************************************
//                        Tasks name declarations
//*****************************************************************************

DECLARE_TASK(TTaskInit, 		TaskMask)
DECLARE_TASK(TTaskMonitor, 	TaskMask)
DECLARE_TASK(TTaskUDP_Service,	grpSystem | grpNetState)    

//*****************************************************************************
//                       Specific task classes
//*****************************************************************************

class TTaskMAC_Service : public TBaseTask
{ 
private:
	uint32_t timer2, timer3;
	bool connected;
	
public:
	TTaskMAC_Service(TEventMask mask) : TBaseTask(mask)
		{};
		
	// TBaseTask::
	virtual void activate(void);
	virtual void deactivate(void);
	virtual void handleEvent(TCMessage* msg);
	virtual void process(void);
	
	void setState(uint16_t newState);
};

//*****************************************************************************
//                              Tasks ID
//*****************************************************************************

const unsigned short task_Monitor						= 0;
const unsigned short task_MAC_Service				= 1;
const unsigned short task_UDP_Service				= 2;
const unsigned short task_Init							= 3;

const unsigned short tasksNum	= task_Init+1;

extern TSupervisor supervisor;		// tasks.cpp

#endif		// TASKDEF_H
