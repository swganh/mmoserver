/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/
#include "Shuttle.h"
#include "PlayerObject.h"
#include "TicketCollector.h"
#include "TravelMapHandler.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"


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
    mPvPStatus		= CreaturePvPStatus_None;
    mFactionRank	= 0;
}

//=============================================================================

Shuttle::~Shuttle()
{
}

//=============================================================================
//
// Returns the state of the shuttle.
//
ShuttleState Shuttle::getShuttleState()
{
    return mShuttleState;
}

//=============================================================================
//
// returns true if shuttle avaliable in starport. The shuttle is always available in theed spaceport
//
bool Shuttle::availableInPort(void)
{
    BString port("");

    // Some shuttles are not linked to any collectors.
    if (TicketCollector* collector = dynamic_cast<TicketCollector*>(gWorldManager->getObjectById(mTicketCollectorId)))
    {
        port = collector->getPortDescriptor();
    }
    return ((mShuttleState == ShuttleState_InPort) || (port.getCrc() == BString("Theed Spaceport").getCrc()));
}

//=============================================================================

void Shuttle::useShuttle(PlayerObject* playerObject)
{
	if(playerObject->states.getPosture() == CreaturePosture_SkillAnimating)
	{
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), playerObject);
        return;
    }

    TicketCollector* collector = dynamic_cast<TicketCollector*>(gWorldManager->getObjectById(getCollectorId()));

    if(!collector)
    {
        int8 errmsg[128];
        sprintf(errmsg, "No ticket collector on duty error : %" PRIu64, mTicketCollectorId);
        BString u = BString(errmsg);
        u.convert(BSTRType_Unicode16);

        gMessageLib->SendSystemMessage(u.getUnicode16(), playerObject);

        LOG(WARNING) <<  errmsg;

        return;
    }

    ShuttleState shuttleState = mShuttleState;

    if (availableInPort())
    {
        // Override Theed shuttles.
        shuttleState = ShuttleState_InPort;
    }

    switch(shuttleState)
    {
    case ShuttleState_InPort:
    {
        BString port = collector->getPortDescriptor();

        bool noTicket = gTravelMapHandler->findTicket(playerObject,port);

        // in range check
        if(playerObject->getParentId() != getParentId() || (glm::distance(playerObject->mPosition, mPosition) > 25.0f))
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("travel", "boarding_too_far"), playerObject);
            return;
        }

        if(noTicket)
            gMessageLib->SendSystemMessage(::common::OutOfBand("travel", "no_ticket"), playerObject);
        else
            gTravelMapHandler->createTicketSelectMenu(playerObject,this,port);

    }
    break;

    case ShuttleState_Away:
    {
        wchar_t temp[256];
        uint32	minutes = (mAwayInterval - mAwayTime) / 60000;
        uint32	seconds = (60000 - (mAwayTime%60000)) / 1000;

        if(seconds == 60)
            seconds = 0;

        if(minutes > 0)
            swprintf(temp,80,L"The next shuttle will be ready to board in %u minutes %u seconds.",minutes,seconds);
        else
            swprintf(temp,80,L"The next shuttle will be ready to board in %u seconds.",seconds);

        gMessageLib->SendSystemMessage(temp, playerObject);
    }
    break;

    case ShuttleState_Landing:
    {
        gMessageLib->SendSystemMessage(L"The next shuttle is about to begin boarding.", playerObject);
    }
    break;

    case ShuttleState_AboutBoarding:
    {
        gMessageLib->SendSystemMessage(L"The next shuttle is about to begin boarding.", playerObject);
    }

    default:
        break;
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