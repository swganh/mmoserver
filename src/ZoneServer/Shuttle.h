/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_SHUTTLE_H
#define ANH_ZONESERVER_SHUTTLE_H

#include "CreatureObject.h"
#include "Utils/typedefs.h"

#define SHUTTLE_LANDING_ANIMATION_TIME 25000

enum ShuttleState
{
	ShuttleState_Away			= 0,
	ShuttleState_InPort			= 1,
	ShuttleState_Landing		= 2,
	ShuttleState_AboutBoarding	= 3
};

//=============================================================================

class Shuttle : public CreatureObject
{
	friend class ShuttleFactory;

	public:

		Shuttle();
		~Shuttle();

		void			setAwayInterval(uint32 v){ mAwayInterval = v; }
		uint32			getAwayInterval(){ return mAwayInterval; }
		void			setInPortInterval(uint32 v){ mInPortInterval = v; }
		uint32			getInPortInterval(){ return mInPortInterval; }
		void			setInPortTime(uint32 time){ mInPortTime = time; }
		uint32			getInPortTime(){ return mInPortTime; }
		void			setAwayTime(uint32 time){ mAwayTime = time; }
		uint32			getAwayTime(){ return mAwayTime; }
		void			setLandingTime(uint32 time){ mLandingTime = time; }
		uint32			getLandingTime(){ return mLandingTime; }
		void			setShuttleState(ShuttleState state){ mShuttleState = state; }
		ShuttleState	getShuttleState();
		bool			availableInPort(void);
		void			setCollectorId(uint64 id){ mTicketCollectorId = id; }
		uint64			getCollectorId(){ return mTicketCollectorId; }
		void			useShuttle(PlayerObject* playerObject);
		bool			ticketCollectorEnabled() const;
		void			ticketCollectorEnable();

	private:

		ShuttleState	mShuttleState;
		uint64			mTicketCollectorId;
		uint32			mAwayInterval;
		uint32			mAwayTime;
		uint32			mInPortInterval;
		uint32			mInPortTime;
		uint32			mLandingTime;
		bool			mTicketCollectorEnabled;

};

//=============================================================================

#endif

