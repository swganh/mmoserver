/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_SAMPLE_EVENT_H
#define ANH_ZONESERVER_SAMPLE_EVENT_H

#include "Utils/EventHandler.h"


//======================================================================================================================

class SampleEvent : public Anh_Utils::Event
{
	public:

		SampleEvent(SurveyTool* tool,CurrentResource* resource) : mTool(tool),mResource(resource){}

		SurveyTool*			getTool() const { return mTool; }
		CurrentResource*	getResource() const { return mResource; }

	private:

		SurveyTool*			mTool;
		CurrentResource*	mResource;
};


//======================================================================================================================

#endif



