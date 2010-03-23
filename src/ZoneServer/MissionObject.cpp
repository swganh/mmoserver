/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MissionObject.h"
#include "MissionManager.h" //circular dependency ohmy!
#include "NPCObject.h"
#include "PlayerObject.h"
#include "Terminal.h"
#include "WaypointObject.h"
#include "ZoneOpcodes.h"

#include "Common/Message.h"
#include "Common/MessageFactory.h"
#include "MathLib/Quaternion.h"

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

	mStart.Coordinates = Anh_Math::Vector3();
	mStart.CellID = 0;
	mStart.PlanetCRC = 0;
	mStartNPC = NULL;

	mDestination.Coordinates = Anh_Math::Vector3();
	mDestination.CellID = 0;
	mDestination.PlanetCRC = 0;	
	mDestinationNPC	 = NULL;

	mInProgress	= false;
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

	mStart.Coordinates = Anh_Math::Vector3();
	mStart.CellID = 0;
	mStart.PlanetCRC = 0;
	mStartNPC = NULL;

	mDestination.Coordinates = Anh_Math::Vector3();
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
	mStart.Coordinates = Anh_Math::Vector3();
	mStart.CellID = 0;
	mStart.PlanetCRC = 0;
	mStartNPC		= NULL;

	mDestination.Coordinates = Anh_Math::Vector3();
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
	string desc = buff;
	desc.convert(BSTRType_Unicode16);
	gMessageFactory->addString(desc);

	//Creator
	gMessageFactory->addString("@ui_mission:table_creator");
	string str = mCreator; 
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




