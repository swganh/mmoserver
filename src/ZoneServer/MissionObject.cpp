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

#include "MissionObject.h"
#include "MissionManager.h" //circular dependency ohmy!
#include "NPCObject.h"
#include "PlayerObject.h"
#include "Terminal.h"
#include "WaypointObject.h"
#include "ZoneOpcodes.h"
#include "Buff.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

//=============================================================================

MissionObject::MissionObject() : Object(0,0,"object/mission/shared_mission_object.iff",ObjType_Mission)
{
    mOwner				= NULL;
    mIssuingTerminal	= NULL;
    mName				= "destroy_mission"; //default name common to all mission types
    mNameFile			= "mission/mission_object";
    mDifficulty			= 0;
    mCreator			= L"";
    mReward				= 0;
    mTargetModel		= 0;
    mDetailFile			= "";
    mDetail				= "";
    mTitleFile			= "";
    mTitle				= "";
    mRefreshCount		= 0;
    mMissionType		= 0;
    mTarget				= "";
    mNum				= 0;
    external			= true;
    mWaypoint = new WaypointObject();
    mWaypoint->setWPType(Waypoint_orange);

    mStart.Coordinates = glm::vec3();
    mStart.CellID = 0;
    mStart.PlanetCRC = 0;
    mStartNPC = NULL;

    mDestination.Coordinates = glm::vec3();
    mDestination.CellID = 0;
    mDestination.PlanetCRC = 0;
    mDestinationNPC	 = NULL;

    mInProgress	= false;
    mEntertainingTimer = NULL;
}

//=============================================================================

MissionObject::MissionObject(PlayerObject * owner, uint64 parent_id) : Object(
        getNewMissionId(owner->getMissionIdMask(), owner->getId()),
        parent_id,
        "object/mission/shared_mission_object.iff",
        ObjType_Mission
    )
{
    mOwner				= owner;
    mIssuingTerminal	= NULL;
    mName				= "destroy_mission"; //default name common to all mission types	-- value = "dynmaic mission"
    mNameFile			= "mission/mission_object";
    mDifficulty			= 0;
    mCreator			= L"";
    mReward				= 0;
    mTargetModel		= 0;
    mDetailFile			= "";
    mDetail				= "";
    mTitleFile			= "";
    mTitle				= "";
    mRefreshCount		= 0;
    mMissionType		= 0;
    mTarget				= "";
    mNum				= 0;
    external			= false;
    mWaypoint = new WaypointObject();
    mWaypoint->setId(mId+1);
    mWaypoint->setWPType(Waypoint_orange);

    mStart.Coordinates = glm::vec3();
    mStart.CellID = 0;
    mStart.PlanetCRC = 0;
    mStartNPC = NULL;

    mDestination.Coordinates = glm::vec3();
    mDestination.CellID = 0;
    mDestination.PlanetCRC = 0;
    mDestinationNPC	 = NULL;

    mInProgress	= false;
}
//=============================================================================


MissionObject::~MissionObject()
{
    if(!external)
    {
        freeMissionId(mOwner->getMissionIdMask(), mOwner->getId());
    }

    delete mWaypoint;
}

//=============================================================================

void MissionObject::clear()
{
    mIssuingTerminal	= NULL;
    mDifficulty			= 0;
    mCreator			= L"";
    mReward				= 0;
    mTargetModel		= 0;
    mDetailFile			= "";
    mDetail				= "";
    mTitleFile			= "";
    mTitle				= "";
    mRefreshCount		= 0;
    mMissionType		= 0;
    mTarget				= "";
    mNum				= 0;
    mStart.Coordinates = glm::vec3();
    mStart.CellID = 0;
    mStart.PlanetCRC = 0;
    mStartNPC		= NULL;

    mDestination.Coordinates = glm::vec3();
    mDestination.CellID = 0;
    mDestination.PlanetCRC = 0;
    mDestinationNPC	= NULL;

    mInProgress		=	false;

    return;
}
//=============================================================================


void MissionObject::sendAttributes(PlayerObject* playerObject)
{
    if(playerObject->getConnectionState() != PlayerConnState_Connected)
        return;


    Message*	message;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opAttributeListMessage);
    gMessageFactory->addUint64(mId);
    gMessageFactory->addInt32(4); //attrib count

    //Description
    gMessageFactory->addString("description");
    char buff[255];//crude method but effective for using char %s to wchar_t buffer
    sprintf(buff,"@%s:%s",mDetailFile.getRawData(),mDetail.getRawData());
    BString desc = buff;
    desc.convert(BSTRType_Unicode16);
    gMessageFactory->addString(desc);

    //Creator
    gMessageFactory->addString("@ui_mission:table_creator");
    BString str = mCreator;
    str.convert(BSTRType_Unicode16);
    gMessageFactory->addString(str);

    //Reward
    char meanBuff[255];
    gMessageFactory->addString("@ui_mission:table_payment");
    sprintf(meanBuff,"%d",mReward);

    str = meanBuff;
    str.convert(BSTRType_Unicode16);
    gMessageFactory->addString(str);

    //Difficulty
    gMessageFactory->addString("@ui_mission:table_difficulty");
    sprintf(meanBuff,"%d",mDifficulty);
    str = meanBuff;
    str.convert(BSTRType_Unicode16);

    gMessageFactory->addString(str);

    message = gMessageFactory->EndMessage();

    (playerObject->getClient())->SendChannelAUnreliable(message, playerObject->getAccountId(),CR_Client,8);

    return;
}
//=============================================================================

/*
 * Passing the job off to mission manager to keep all the check
 * functions in one area. This check function is here because
 * of the scheduler.
*/
bool MissionObject::check(uint64 callTime, void *)
{
    return gMissionManager->checkReconMission(this);
}

//=============================================================================




