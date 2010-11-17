/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_WOUNDTREATMENT_EVENT_H
#define ANH_ZONESERVER_WOUNDTREATMENT_EVENT_H

#include "Utils/EventHandler.h"

//======================================================================================================================

class WoundTreatmentEvent : public Anh_Utils::Event
{
public:

    WoundTreatmentEvent(uint64 time) : mWoundTreatmentTime(time) {}

    uint64 getWoundTreatmentTime() const {
        return  mWoundTreatmentTime;
    }


private:

    uint64				mWoundTreatmentTime;
};


//======================================================================================================================

#endif



