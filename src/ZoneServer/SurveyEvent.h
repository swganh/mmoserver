/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_SURVEY_EVENT_H
#define ANH_ZONESERVER_SURVEY_EVENT_H

#include "Utils/EventHandler.h"
#include "SurveyTool.h"
#include "CurrentResource.h"


//======================================================================================================================

class SurveyEvent : public Anh_Utils::Event
{
	public:

		SurveyEvent(SurveyTool* tool,CurrentResource* resource) : mTool(tool),mResource(resource){}

		SurveyTool*			getTool() const { return mTool; }
		CurrentResource*	getResource() const { return mResource; }

	private:

		SurveyTool*			mTool;
		CurrentResource*	mResource;
};


//======================================================================================================================

#endif



