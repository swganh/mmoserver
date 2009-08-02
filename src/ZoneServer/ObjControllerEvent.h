/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_OBJCONTROLLER_EVENT_H
#define ANH_OBJCONTROLLER_EVENT_H

#include "Utils/typedefs.h"
#include "Utils/EventHandler.h"


//======================================================================================================================

class ObjControllerEvent
{
	public:

		ObjControllerEvent(const uint64 executionTime,Anh_Utils::Event* event) : mExecutionTime(executionTime),mEvent(event){}

		virtual ~ObjControllerEvent()
		{
			delete(mEvent);
		}

		virtual inline uint64				getExecutionTime() const { return mExecutionTime; }
		virtual inline void					setExecutionTime(uint64 time){ mExecutionTime = time; }

		virtual inline Anh_Utils::Event*	getEvent() const { return mEvent; }
		virtual inline void					setEvent(Anh_Utils::Event* event){ mEvent = event; }

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

