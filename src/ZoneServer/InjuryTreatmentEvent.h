/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_INJURYTREATMENT_EVENT_H
#define ANH_ZONESERVER_INJURYTREATMENT_EVENT_H

#include "Utils/EventHandler.h"




//======================================================================================================================

class InjuryTreatmentEvent : public Anh_Utils::Event
{
	public:

		InjuryTreatmentEvent(uint64 time, uint32 spacer) : mInjuryTreatmentTime(time), mInjuryTreatmentSpacer(spacer){}
		
		uint64 getInjuryTreatmentTime() const {return  mInjuryTreatmentTime;}
		uint32 getInjuryTreatmentSpacer() const {return  mInjuryTreatmentSpacer;}


	private:

		uint64				mInjuryTreatmentTime;
		uint32				mInjuryTreatmentSpacer;
};


//======================================================================================================================

#endif



