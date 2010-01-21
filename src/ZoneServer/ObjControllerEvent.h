/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_OBJCONTROLLER_EVENT_H
#define ANH_OBJCONTROLLER_EVENT_H

#include "Utils/typedefs.h"

namespace Anh_Utils
{
    class Event;
}

//======================================================================================================================

class ObjControllerEvent
{
public:
    ObjControllerEvent(const uint64 executionTime, Anh_Utils::Event* event);

    virtual ~ObjControllerEvent();

    virtual uint64 getExecutionTime() const;
    virtual void setExecutionTime(uint64 time);

    virtual Anh_Utils::Event* getEvent() const;
    virtual void setEvent(Anh_Utils::Event* event);

protected:
    uint64				mExecutionTime;
    Anh_Utils::Event*	mEvent;
};

//===============================================================================================================

class CompareEvent
{
public:
    bool operator () (const ObjControllerEvent* left, const ObjControllerEvent* right)
    {
        return(left->getExecutionTime() > right->getExecutionTime());
    } 
};

//===============================================================================================================

#endif

