
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ObjControllerEvent.h"
#include "Utils/EventHandler.h"

ObjControllerEvent::ObjControllerEvent(const uint64 executionTime, Anh_Utils::Event* event) 
: mExecutionTime(executionTime)
, mEvent(event)
{}

ObjControllerEvent::~ObjControllerEvent()
{
    delete(mEvent);
}

uint64 ObjControllerEvent::getExecutionTime() const 
{ 
    return mExecutionTime; 
}

void ObjControllerEvent::setExecutionTime(uint64 time)
{ 
    mExecutionTime = time; 
}

Anh_Utils::Event* ObjControllerEvent::getEvent() const 
{ 
    return mEvent; 
}

void ObjControllerEvent::setEvent(Anh_Utils::Event* event)
{ 
    mEvent = event; 
}

