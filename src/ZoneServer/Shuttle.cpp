/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "Shuttle.h"
#include "PlayerObject.h"
#include "TicketCollector.h"
#include "TravelMapHandler.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "MathLib/Quaternion.h"

//=============================================================================

Shuttle::Shuttle()
: CreatureObject()
, mAwayTime(0)
, mInPortTime(0)
, mLandingTime(0)
, mTicketCollectorEnabled(false)
{
	mCreoGroup		= CreoGroup_Shuttle;
	mState			= 0;
	mMoodId			= 0;
	mCL				= 0;
	mShuttleState	= ShuttleState_InPort;
	// mPvPStatus		= 0;
	mPvPStatus		= CreaturePvPStatus_None;
	mFactionRank	= 0;
}

//=============================================================================

Shuttle::~Shuttle()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the state of the shuttle.
///////////////////////////////////////////////////////////////////////////////////////////////////

ShuttleState Shuttle::getShuttleState()
{
	return mShuttleState;
}
//=============================================================================

//returns true if shuttle avaliable in starport. Tthe shuttle is always available in theed spaceport

bool Shuttle::avaliableInPort(void)
{
	string port("");
	TicketCollector* collector = (TicketCollector*) gWorldManager->getObjectById(getCollectorId());

	// Some shuttles are not linked to any collectors.
	if (collector)
	{
		port = collector->getPortDescriptor();
	}
	return ((mShuttleState == ShuttleState_InPort) || (port.getCrc() == BString("Theed Spaceport").getCrc()));
}
//=============================================================================

void Shuttle::useShuttle(PlayerObject* playerObject)
{
	if(playerObject->getPosture() == CreaturePosture_SkillAnimating)
	{
		gMessageLib->sendSystemMessage(playerObject,L"You cannot do this at this time.");
		return;
		//gEntertainerManager->stopEntertaining(player);
	}

	TicketCollector* collector = (TicketCollector*) gWorldManager->getObjectById(getCollectorId());

	if(!collector)
	{
		int8 sql[128];
		sprintf(sql,"No ticket collector on duty error : %"PRIu64,getCollectorId());
		string u = BString(sql);
		u.convert(BSTRType_Unicode16);
		gMessageLib->sendSystemMessage(playerObject,u.getUnicode16());
		gLogger->logMsgF(sql,MSG_HIGH);
		gLogger->logMsg("\n");
		return;
	}
	ShuttleState shuttleState = getShuttleState();
	if (avaliableInPort())
	{
		// Override Theed shuttles.
		shuttleState = ShuttleState_InPort;
	}

	switch(shuttleState)
	{
		case ShuttleState_InPort:
		{
			string port = collector->getPortDescriptor();

			bool noTicket = gTravelMapHandler->findTicket(playerObject,port);

			// in range check
			if(playerObject->getParentId() != getParentId() || !playerObject->mPosition.inRange2D(mPosition,25.0f))
			{
				gMessageLib->sendSystemMessage(playerObject,L"","travel","boarding_too_far");
				return;
			}

			if(noTicket)
				gMessageLib->sendSystemMessage(playerObject,L"","travel","no_ticket");
			else
				gTravelMapHandler->createTicketSelectMenu(playerObject,this,port);

		}
		break;

		case ShuttleState_Away:
		{
			if(playerObject->isConnected())
			{
				string	awayMsg = string(BSTRType_Unicode16,256);
				uint32	minutes = (getAwayInterval() - getAwayTime()) / 60000;
				uint32	seconds = (60000 - (getAwayTime()%60000)) / 1000;

				if(seconds == 60)
					seconds = 0;

				if(minutes > 0)
				{
					awayMsg.setLength(swprintf(awayMsg.getUnicode16(),80,L"The next shuttle will be ready to board in %u minutes %u seconds.",minutes,seconds));
				}
				else
					awayMsg.setLength(swprintf(awayMsg.getUnicode16(),80,L"The next shuttle will be ready to board in %u seconds.",seconds));

				gMessageLib->sendSystemMessage(playerObject,awayMsg);
			}
		}
		break;

		case ShuttleState_Landing:
		{
			//no stf available :(
			gMessageLib->sendSystemMessage(playerObject,L"The next shuttle is about to begin boarding.");
			//gMessageLib->sendSystemMessage(playerObject,L"The shuttle is about to land.");
		}
		break;

		case ShuttleState_AboutBoarding:
		{
			//no stf available :(
			gMessageLib->sendSystemMessage(playerObject,L"The next shuttle is about to begin boarding.");
		}

		default:break;
	}
}

//=============================================================================
bool Shuttle::ticketCollectorEnabled() const
{
	return mTicketCollectorEnabled;
}

//=============================================================================
void Shuttle::ticketCollectorEnable()
{
	mTicketCollectorEnabled = true;
}

