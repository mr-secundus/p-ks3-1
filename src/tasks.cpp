#include <port.h>
#include "cfg/defines.h"
#include "tasks.h"
#include "taskdef.h"

//*****************************************************************************

static TTaskMAC_Service		tsMAC_Service(grpSystem | grpNet);
static TTaskUDP_Service		tsUDP_Service;
static TTaskMonitor				tsMonitor;               
static TTaskInit					tsInit;							     

static TBaseTask*		tasks[tasksNum] = 
{                                   
  &tsMonitor,       
	&tsMAC_Service,
	&tsUDP_Service,         
  //---- tsInit должна быть последней в списке
  &tsInit
};

static unsigned char messageBuffer[sizeof(TCMessage)*MESSAGE_QUEUE_SIZE];

TSupervisor supervisor(tasks, tasksNum, messageBuffer, MESSAGE_QUEUE_SIZE);

//*****************************************************************************

void initTasks(void)
{      
	supervisor.activateTask(task_Init);
}

void processTasks(void)
{                                   
	supervisor.processTasks();
	supervisor.processMessages();
}              

TBaseTask* getTaskPtr(uint16_t taskID)
{
	return taskID < tasksNum ? tasks[taskID] : 0;
}

