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

