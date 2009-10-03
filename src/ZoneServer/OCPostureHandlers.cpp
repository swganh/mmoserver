/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "WorldManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/MessageFactory.h"
#include "Common/Message.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "WorldConfig.h"
#include "Item.h"
#include "QTRegion.h"
#include "MathLib/Math.h"
#include "PlayerObject.h"
#include "ZoneTree.h"

//=============================================================================
//
// sit
//

void ObjectController::_handleSitServer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	// FIXME: for now assume only players send chat
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	uint8			currentPosture	= playerObject->getPosture();
	string			data;
	float			chairX,chairY,chairZ;
	uint64			chairCell		= 0;
	uint32			elementCount	= 0;

	// gLogger->logMsg("ObjectController::_handleSitServer: Entering");

	if(playerObject->isConnected())
		gMessageLib->sendHeartBeat(playerObject->getClient());

	// see if we need to get out of sampling mode
	if(playerObject->getSamplingState())
	{
		gMessageLib->sendSystemMessage(playerObject,L"","survey","sample_cancel");
		playerObject->setSamplingState(false);
	}

	message->getStringUnicode16(data);

	playerObject->setPosture(CreaturePosture_Sitting);
	playerObject->getHam()->updateRegenRates();

	// sitting on chair
	if(data.getLength())
	{
		elementCount = swscanf(data.getUnicode16(),L"%f,%f,%f,%lld",&chairX,&chairY,&chairZ,&chairCell);	
		
		if(elementCount == 4)
		{
			// outside
			if(!chairCell)
			{
				if(QTRegion* newRegion = mSI->getQTRegion((double)chairX,(double)chairZ))
				{
					// we didnt change so update the old one
					if((uint32)newRegion->getId() == playerObject->getSubZoneId())
					{
						// this also updates the players position
						newRegion->mTree->updateObject(playerObject,Anh_Math::Vector3(chairX,chairY,chairZ));
					}
					else
					{
						// remove from old
						if(QTRegion* oldRegion = gWorldManager->getQTRegion(playerObject->getSubZoneId()))
						{
							oldRegion->mTree->removeObject(playerObject);
						}

						// update players position
						playerObject->mPosition = Anh_Math::Vector3(chairX,chairY,chairZ);

						// put into new
						playerObject->setSubZoneId((uint32)newRegion->getId());
						newRegion->mTree->addObject(playerObject);
					}				
				}
				else
				{
					// we should never get here !
					gLogger->logMsg("SitOnObject: could not find zone region in map\n");

					// hammertime !
					exit(-1);
				}
			}
			// we are in a cell
			else
			{
				// switch cells, if needed
				if(playerObject->getParentId() != chairCell)
				{
					CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(playerObject->getParentId()));

					if(cell)
						cell->removeChild(playerObject);
					else
						gLogger->logMsgF("Error removing %lld from cell %lld",MSG_NORMAL,playerObject->getId(),playerObject->getParentId());

					playerObject->setParentId(chairCell);

					cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(chairCell));

					if(cell)
						cell->addChild(playerObject);
					else
						gLogger->logMsgF("Error adding %lld to cell %lld",MSG_NORMAL,playerObject->getId(),chairCell);
				}

				playerObject->mPosition = Anh_Math::Vector3(chairX,chairY,chairZ);
			}

			//playerObject->mDirection = Anh_Math::Quaternion();
			playerObject->toggleStateOn(CreatureState_SittingOnChair);

			playerObject->updateMovementProperties();

			// TODO: check if we need to send transforms to others
			if(chairCell)
			{
				gMessageLib->sendDataTransformWithParent(playerObject);
			}
			else
			{
				gMessageLib->sendDataTransform(playerObject);
			}

			gMessageLib->sendUpdateMovementProperties(playerObject);
			gMessageLib->sendPostureAndStateUpdate(playerObject);

			gMessageLib->sendSitOnObject(playerObject);
		}
	}
	// sitting on ground
	else
	{
		gMessageLib->sendPostureUpdate(playerObject);
		gMessageLib->sendSelfPostureUpdate(playerObject);
	}

	//hack-fix clientside bug by manually sending client message
	gMessageLib->sendSystemMessage(playerObject,L"","shared","player_sit");
}

//=============================================================================
//
// stand
//

void ObjectController::_handleStand(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	// gLogger->logMsg("ObjectController::_handleStand: Entering");

	// FIXME: for now assume only players send chat
	PlayerObject*	playerObject = dynamic_cast<PlayerObject*>(mObject);

	if(playerObject->isConnected())
		gMessageLib->sendHeartBeat(playerObject->getClient());

	// see if we need to get out of sampling mode
	if(playerObject->getSamplingState())
	{
		gMessageLib->sendSystemMessage(playerObject,L"","survey","sample_cancel");
		playerObject->setSamplingState(false);
	}

	//Get whether player is seated on a chair before we toggle it

	// Can not compare bitwise data with equality... the test below will only work if ALL other states = 0.
	// bool IsSeatedOnChair = (playerObject->getState() == CreatureState_SittingOnChair);
	bool IsSeatedOnChair = playerObject->checkState(CreatureState_SittingOnChair);

	// reset sitonchair state
	playerObject->toggleStateOff(CreatureState_SittingOnChair);
	playerObject->setPosture(CreaturePosture_Upright);
	playerObject->getHam()->updateRegenRates();
	playerObject->updateMovementProperties();

	gMessageLib->sendUpdateMovementProperties(playerObject);
	gMessageLib->sendPostureAndStateUpdate(playerObject);
	gMessageLib->sendSelfPostureUpdate(playerObject);

	//if player is seated on an a chair, hack-fix clientside bug by manually sending client message
	if(IsSeatedOnChair)
	{
		gMessageLib->sendSystemMessage(playerObject,L"","shared","player_stand");
	}
}

//=============================================================================
//
// prone
//

void ObjectController::_handleProne(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);

	if(playerObject->isConnected())
		gMessageLib->sendHeartBeat(playerObject->getClient());

	// see if we need to get out of sampling mode
	if(playerObject->getSamplingState())
	{
		gMessageLib->sendSystemMessage(playerObject,L"","survey","sample_cancel");
		playerObject->setSamplingState(false);
	}


	//Get whether player is seated on a chair before we toggle it

	// Can not compare bitwise data with equality... the test below will only work if ALL other states = 0.
	// bool IsSeatedOnChair = (playerObject->getState() == CreatureState_SittingOnChair);
	bool IsSeatedOnChair = playerObject->checkState(CreatureState_SittingOnChair);

	playerObject->setPosture(CreaturePosture_Prone);
	playerObject->getHam()->updateRegenRates();
	playerObject->toggleStateOff(CreatureState_SittingOnChair);
	playerObject->updateMovementProperties();

	gMessageLib->sendUpdateMovementProperties(playerObject);
	gMessageLib->sendPostureAndStateUpdate(playerObject);
	gMessageLib->sendSelfPostureUpdate(playerObject);
	
	//if player is seated on an a chair, hack-fix clientside bug by manually sending client message
	if(IsSeatedOnChair)
	{
		gMessageLib->sendSystemMessage(playerObject,L"","shared","player_prone");
	}
}

//=============================================================================
//
// kneel
//

void ObjectController::_handleKneel(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject = dynamic_cast<PlayerObject*>(mObject);

	if(playerObject->isConnected())
		gMessageLib->sendHeartBeat(playerObject->getClient());

	//Get whether player is seated on a chair before we toggle it
	// Can not compare bitwise data with equality... the test below will only work if ALL other states = 0.
	// bool IsSeatedOnChair = (playerObject->getState() == CreatureState_SittingOnChair);
	bool IsSeatedOnChair = playerObject->checkState(CreatureState_SittingOnChair);

	playerObject->setPosture(CreaturePosture_Crouched);
	playerObject->getHam()->updateRegenRates();
	playerObject->toggleStateOff(CreatureState_SittingOnChair);
	playerObject->updateMovementProperties();

	gMessageLib->sendUpdateMovementProperties(playerObject);
	gMessageLib->sendPostureAndStateUpdate(playerObject);
	gMessageLib->sendSelfPostureUpdate(playerObject);
		
	//if player is seated on an a chair, hack-fix clientside bug by manually sending client message
	if(IsSeatedOnChair)
	{
		gMessageLib->sendSystemMessage(playerObject,L"","shared","player_kneel");
	}
}

//=============================================================================



