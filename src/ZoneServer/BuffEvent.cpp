#include "BuffEvent.h"
#include "Buff.h"
#include <iostream>
BuffEvent::~BuffEvent(void)
{
}
bool BuffEvent::Tick(uint64 CurrentTime, void *ref)
{
	Buff* temp = *mBuff;
	std::cout<< "Event has Ptr " << temp << std::endl;
	
	bool bReturn = true;
	//bool bReturn = temp->UpdateTick(CurrentTime);
	
	//Do any final cleanup Here

	return bReturn;
}
