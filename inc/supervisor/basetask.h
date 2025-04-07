#ifndef __BASETASK_H
#define __BASETASK_H


//#define USE_MESSAGE_PRIORITY

typedef unsigned long TEventMask;
typedef unsigned long TMsgMask;
typedef int	TTaskHandle;
typedef int TPriority;

//-----------------------------------------------------------------------------
class TCMessage
{
private:

public:
	TEventMask	msgGroup;
	long				Param1, Param2;
	short				msgNo;
	bool				discard;

	TCMessage(void) :
		msgGroup(0), Param1(0), Param2(0), msgNo(0), discard(false) {};

	TCMessage(TEventMask g, short no, /*TPriority p,*/ long p1, long p2) :
		msgGroup(g), /*priority(p),*/ Param1(p1), Param2(p2), msgNo(no), discard(false) {};

	TCMessage& operator =(const TCMessage& p)
	{
		msgGroup 	= p.msgGroup;     
		Param1 		= p.Param1;
		Param2 		= p.Param2;
		msgNo 		= p.msgNo;        
		discard 	= p.discard;      
		return *this;
	}
};

//-----------------------------------------------------------------------------

class TSupervisor;

class TBaseTask
{
private:
//	TTaskHandle handle;
	TEventMask  eventMask;
	bool active;           

protected:
	// @29.04.2014 iss513
	unsigned short state;		
	unsigned timer;      

public:
	TBaseTask(/*TTaskHandle Ahandle,*/ TEventMask AEventMask) :
		/*handle(Ahandle),*/ eventMask(AEventMask), active(false) {};

	unsigned long getEventMask() const { return eventMask; }
//	TTaskHandle getHandle() const { return handle; }
  
	inline bool isActive(void)		{return active;}
	
	virtual void activate(void)		{active = true;}
	virtual void deactivate(void)	{active = false;}

	virtual void handleEvent(TCMessage* msg)	{}
	virtual void process(void) = 0;

	unsigned short getState(void) 						{ return state; }
	void 					 setState(unsigned short s) { state = s; }
};

#endif
