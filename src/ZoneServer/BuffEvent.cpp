
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "BuffEvent.h"
#include "Buff.h"
#include <iostream>
		

BuffEvent::BuffEvent(Buff* buff)
{ 
    mBuff = &buff; 
}
		

BuffEvent::~BuffEvent()
{}
		

Buff* BuffEvent::getBuff() const 
{ 
    return *mBuff; 
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
