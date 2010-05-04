/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include <math.h>

#include "VehicleController.h"
#include "CreatureObject.h"
#include "IntangibleObject.h"
#include "PlayerObject.h"
#include "MountObject.h"
#include "Heightmap.h"
#include "Object.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"


//very temp
unsigned char Swoop_Customization[99] =	{ 0x61, 0x00, 0x02, 0x11, 0xC3, //customization data
0x9D, 0x28, 0xC3, 0xBF, 0x01, 0xC3, 0x9E, 0xC3, 0x8D, 0xC3, 0xBF, 0x01,
0xC3, 0xA2, 0x1E, 0xC3, 0xBF, 0x01, 0xC3, 0x9B, 0x1D, 0xC3, 0xBF, 0x01,
0xC3, 0x98, 0x2D, 0xC3,	0xBF, 0x01, 0xC3, 0x9A, 0x63, 0xC3, 0xBF, 0x01,
0xC3, 0x99, 0x05, 0xC3, 0xBF, 0x01, 0xC3, 0xA1, 0x19, 0xC3, 0xBF, 0x01,
0xC3, 0xA0, 0x14, 0xC3, 0xBF, 0x01, 0xC3, 0x9F, 0x05, 0xC3, 0xBF, 0x01,
0xC3, 0x9C, 0x46, 0xC3, 0xBF, 0x01, 0xC3, 0x95, 0x19, 0xC3, 0xBF, 0x01,
0x01, 0x04, 0xC3, 0x96, 0x67, 0xC3, 0xBF, 0x01, 0x22, 0x11, 0xC3, 0x97,
0xC3, 0x9B, 0xC3, 0xBF, 0x01, 0x02, 0x11, 0xC3, 0xBF, 0x03 };


VehicleController::VehicleController() : IntangibleObject(),
mTypesId(0),
mHitPointLoss(0), //amount of hitpoints lost during travel
mInclineAcceleration(0),
mFlatAcceleration(0),
mOwner(0)
{
	mType		= ObjType_Intangible;
	mItnoGroup	= ItnoGroup_Vehicle;
	mName		= L"";
	mBody		= NULL;
}

VehicleController::~VehicleController()
{
	if(mBody)
	{
		gWorldManager->destroyObject(mBody);
		//delete mBody;
	}
}

//=============================================================================
//handles the radial selection

void VehicleController::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{

	if(dynamic_cast<PlayerObject*>(srcObject))
	{
		switch(messageType)
		{
			case radId_VehicleGenerate:
			case radId_VehicleStore:
			{
        // If a body for the VehicleController exists then store it, if it doesn't then call it.
        if (mBody) {
          store();
        } else {
				  call();
        }
			}
			break;

			default:
			{
				gLogger->logMsgF("VehicleController::Error: unknown radial selection: %d", MSG_NORMAL, messageType);
			}
			break;
		}
	}
}

//=============================================================================
//handles the radial selection
void VehicleController::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
  mRadialMenu.reset(new RadialMenu());
  
	mRadialMenu->addItem(1, 0, radId_VehicleGenerate, radAction_ObjCallback, "@pet/pet_menu:menu_call");
	mRadialMenu->addItem(2, 0, radId_itemDestroy, radAction_Default);
	mRadialMenu->addItem(3, 0, radId_examine, radAction_Default);
}


//===============================================================================================
//spawns the physical body (CreatureObject)

void VehicleController::call()
{
	if(mBody)
	{   //Destory the old body before creating a new one
		gLogger->logMsgF("void VehicleController::call() body already exists", MSG_HIGH);
    return;
	}

	if(mOwner->checkIfMountCalled())
	{
		gLogger->logMsgF("void VehicleController::call() mount already called", MSG_HIGH);
		return;

	}

	if(!mOwner->isConnected() || mOwner->isDead() || !mOwner->getHam()->checkMainPools(1,1,1))
	{
		return;
	}

	// create the VehicleController creature
	gLogger->logMsgF("void VehicleController::call() create new body", MSG_HIGH);

	mBody = new MountObject();

	string cust;
	cust.initRawBSTR((int8*)Swoop_Customization,BSTRType_ANSI);
	mBody->setCustomizationStr(cust.getAnsi());
	mBody->setCreoGroup(CreoGroup_Vehicle);
	mBody->setTypeOptions(0x1080);
	mBody->setMoodId(0);
	mBody->setCL(0);

	mBody->setId(mId + 1);	// VehicleControllers are created by the VehicleControllerfactory with +2 step for IDs

	setBodyId(mBody->getId());

	//mBody->setId(gWorldManager->getRandomNpId());
	mBody->setPetController(this->getId());

	mBody->setOwner(mOwner->getId());
	mBody->setParentId(0);
	mBody->setModelString(mPhysicalModel);
	mBody->setSpeciesGroup(mNameFile.getAnsi());
	mBody->setSpeciesString(mName.getAnsi());
	mBody->setPosture(0);
	mBody->setScale(1.0f);


	std::string con = this->getAttribute<std::string>("condition");
	mBody->getHam()->setPropertyValue(HamBar_Health, HamProperty_CurrentHitpoints,atoi(con.substr(0,con.find_first_of("/")).c_str()));
	mBody->getHam()->setPropertyValue(HamBar_Health, HamProperty_MaxHitpoints,atoi(con.substr(con.find_first_of("/")+1,con.find_first_of("/")).c_str()));

	mOwner->setMount(mBody);
	mOwner->setMounted(false);
	mOwner->setMountCalled(false);

	// Set default direction and position for the body.
	mBody->mDirection = mOwner->mDirection;
    mBody->mPosition = mOwner->mPosition;

	// Move it forward 2 meters
    mBody->moveForward(2);
	
	// And drop it a little below the terrain to allow the client to normalize it.
	mBody->mPosition.y = Heightmap::Instance()->getHeight(mBody->mPosition.x, mBody->mPosition.z) - 0.3f;

    // Finally rotate it perpendicular to the player.
    mBody->rotateRight(90.0f);

	//we still get nan's here occasionally
	//which will assert our quadtree

	if(_isnan(mBody->mPosition.x))
		mBody->mPosition.x = mOwner->mPosition.x;
	
	if(_isnan(mBody->mPosition.y))
		mBody->mPosition.y = mOwner->mPosition.y;

	if(_isnan(mBody->mPosition.z))
		mBody->mPosition.z = mOwner->mPosition.z;
		

	// add to world
	if(!gWorldManager->addObject(mBody))
	{
		gLogger->logMsgF("void VehicleController::call() creating VehicleController with id % "PRIu64" failed : couldnt add to world", MSG_HIGH, mBody->getId());
		SAFE_DELETE(mBody);
		return;
	}

	gWorldManager->createObjectinWorld(mOwner,mBody);	

	//gMessageLib->sendOwnerUpdateCreo3(mOwner);
	gMessageLib->sendUpdateTransformMessage(mBody);


	mOwner->setMountCalled(true);
	
	return;
}


//===============================================================================================
//stores the physical body
void VehicleController::store()
{
	if(!mBody)
	{
		gLogger->logMsg("VehicleController::store() Error: Store was called for a nonexistant body object!");
		return;
	}

	if(!mOwner || mOwner->isDead() || !mOwner->getHam()->checkMainPools(1,1,1))
	{
		gLogger->logMsg("VehicleController::store() couldnt find owner");
		return;
	}

	// todo auto dismount
	if(mOwner->checkIfMounted())
	{
		dismountPlayer();
	}

	if(!mOwner->checkIfMountCalled())
	{
		gLogger->logMsg("VehicleController::store() Mount wasnt called !!!");
		return;
	}

	//the body is a creature_object!!!
	gMessageLib->sendDestroyObject_InRangeofObject(mBody);

	mOwner->setMount(NULL);


	mOwner->setMounted(false);
	mOwner->setMountCalled(false);

	// finally unload & destroy the VehicleController creature
	gWorldManager->destroyObject(mBody);

	// null the reference
	mBody = NULL;

}


//===============================================================================================
//dismount the owner from the physical body

void VehicleController::dismountPlayer()
{
 	if(!mBody)
	{
		gLogger->logMsg("VehicleController::dismountPlayer() no VehicleController Body!!!");
		return;
	}

	if(!mOwner->checkIfMounted())
	{
		gLogger->logMsg("VehicleController::dismountPlayer() not mounted");
		return;
	}

	gMessageLib->sendContainmentMessage_InRange(mOwner->getId(),mId, 4, mOwner);

	//For safe measures make the player equipped by nothing
	gMessageLib->sendContainmentMessage_InRange(mOwner->getId(), 0, 0xffffffff, mOwner);

	mBody->toggleStateOff(CreatureState_MountedCreature);

	mOwner->toggleStateOff(CreatureState_RidingMount);
	mOwner->setPosture(CreaturePosture_Upright);

	gMessageLib->sendStateUpdate(mBody);
	gMessageLib->sendStateUpdate(mOwner);

	mOwner->setMounted(false);

}

//===============================================================================================
//mount the owner on the physical body
//

void VehicleController::mountPlayer()
{
	if(!mBody)
	{
		gLogger->logMsg("VehicleController::mountPlayer() no VehicleController Body!!!");
		 return;
	}

	CreatureObject* body = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(this->getId()+1));
	if(!body)
	{
		gLogger->logMsg("VehicleController::mountPlayer() no VehicleController Body by Id :(!!!");
		return;
	}

	//Make the mount equip the player

	gMessageLib->sendContainmentMessage_InRange(mOwner->getId(), mBody->getId(), 0xffffffff, mOwner);
	gMessageLib->sendUpdateTransformMessage(mBody);

	mOwner->toggleStateOn(CreatureState_RidingMount);
	mBody->toggleStateOn(CreatureState_MountedCreature);

	gMessageLib->sendStateUpdate(mOwner);
	gMessageLib->sendStateUpdate(mBody);

	mOwner->setMounted(true);

	// TEST ERUPTOR
	// This will ensure the Swoop is seen by other players.
	// I'm sure this can be solved by identifying what's missing, instead of doing the complete sequense below.
	// But as a starter, lets test this solution first.
	PlayerObjectSet* inRangePlayers	= mOwner->getKnownPlayers();
	PlayerObjectSet::iterator it = inRangePlayers->begin();

	while (it != inRangePlayers->end())
	{
		if (!(*it))
		{
			++it;
			gLogger->logMsg("VehicleController::mountPlayer() getObjects in Range :: PlayerObject invalid!!!");
			continue;
		}

		PlayerObject* tested = gWorldManager->getPlayerByAccId((*it)->getAccountId());
		if (!tested)
		{
			++it;
			gLogger->logMsg("VehicleController::mountPlayer() getObjects in Range :: PlayerObject invalid!!!");
			continue;
		}

		if ((tested->isConnected()) && (tested->getClient()))
		{
			gMessageLib->sendCreateObject(mBody, *it);
			gMessageLib->sendContainmentMessage(mOwner->getId(), mBody->getId(), 0xffffffff, *it);
		}
		++it;
	}

}

//===============================================================================================
