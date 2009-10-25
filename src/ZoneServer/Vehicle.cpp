/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Vehicle.h"
#include "CreatureObject.h"
#include "IntangibleObject.h"
#include "PlayerObject.h"
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


Vehicle::Vehicle() : IntangibleObject(),
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

Vehicle::~Vehicle()
{
	if(mBody)
	{

		gWorldManager->destroyObject(mBody);
		//delete mBody;
	}
}

//=============================================================================
//handles the radial selection

void Vehicle::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{

	if(dynamic_cast<PlayerObject*>(srcObject))
	{
		switch(messageType)
		{
			case radId_vehicleGenerate:
			{
				call();
			}
			break;

			case radId_vehicleStore:
			{
				store();
			}
			break;

			default:
			{
				gLogger->logMsgF("Vehicle::Error: unknown radial selection: %d", MSG_NORMAL, messageType);
			}
			break;
		}
	}
}

//=============================================================================
//handles the radial selection
void Vehicle::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	RadialMenu* radial = new RadialMenu();

	radial->addItem(1,0,radId_examine,radAction_Default);
	radial->addItem(2,0,radId_itemDestroy,radAction_Default);

	if(mBody)
	{
		radial->addItem(3,0,radId_vehicleStore,radAction_ObjCallback,"@pet/pet_menu:menu_store");
	}
	else
	{
		radial->addItem(3,0,radId_vehicleGenerate,radAction_ObjCallback,"@pet/pet_menu:menu_call");
	}

	mRadialMenu = RadialMenuPtr(radial);

}


//===============================================================================================
//spawns the physical body (CretureObject)

void Vehicle::call()
{
	if(mBody)
	{   //Destory the old body before creating a new one
		store();
	}

	if(mOwner->isConnected() && !mOwner->checkIfMountCalled())
	{

		// create the vehicle creature

		mBody = new CreatureObject();

		string cust;
		cust.initRawBSTR((int8*)Swoop_Customization,BSTRType_ANSI);
		mBody->setCustomizationStr(cust.getAnsi());
		mBody->setCreoGroup(CreoGroup_Vehicle);
		mBody->setTypeOptions(0x1080);
		mBody->setMoodId(0);
		mBody->setCL(0);

		mBody->setId(mId + 1);	// Vehicles are created by the vehiclefactory with +2 step for IDs

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

		// todo: Write an algorthim that guarentees the mount will appear in-front of the player
		// mugz raw idea :
		// vehicle.x = player.x + (distance * cos(player.heading * RADIANT))
		// vehicle.z = player.z + (distance * sin(player.heading * RADIANT))

		mBody->mPosition.mX = mOwner->mPosition.mX + 2;
		mBody->mPosition.mY = mOwner->mPosition.mY;
		mBody->mPosition.mZ = mOwner->mPosition.mZ + 2;

		// add to world
		gWorldManager->addObject(mBody);


		//spawn it for everyone in range
		PlayerObjectSet*			inRangePlayers	= mOwner->getKnownPlayers();
		PlayerObjectSet::iterator	it				= inRangePlayers->begin();
		while(it != inRangePlayers->end())
		{
			PlayerObject* targetObject = (*it);
			gMessageLib->sendCreateObject(mBody,targetObject);
			targetObject->addKnownObjectSafe(mBody);
			mBody->addKnownObjectSafe(targetObject);
			++it;
		}
		gLogger->logMsgF("void Vehicle::call() creating vehicle with id %"PRId64"", MSG_HIGH, mBody->getId());
		gMessageLib->sendCreateObject(mBody,mOwner);
		mOwner->addKnownObjectSafe(mBody);
		mBody->addKnownObjectSafe(mOwner);

		//gMessageLib->sendOwnerUpdateCreo3(mOwner);
		gMessageLib->sendUpdateTransformMessage(mBody);



		mOwner->setMountCalled(true);
	}

	return;
}


//===============================================================================================
//stores the physical body
void Vehicle::store()
{
	if(!mBody)
	{
		gLogger->logMsg("Vehicle::store() Error: Store was called for a nonexistant body object!");
		return;
	}

	if(!mOwner)
	{
		gLogger->logMsg("Vehicle::store() couldnt find owner");
		return;
	}

	// todo auto dismount
	if(mOwner->checkIfMounted())
	{
		dismountPlayer();
	}

	if(!mOwner->checkIfMountCalled())
	{
		gLogger->logMsg("Vehicle::store() Mount wasnt called !!!");
		return;
	}

	//the body is a creature_object!!!
	gMessageLib->sendDestroyObject_InRangeofObject(mBody);

	mOwner->setMount(NULL);


	mOwner->setMounted(false);
	mOwner->setMountCalled(false);

	// finally unload & destroy the vehicle creature
	gWorldManager->destroyObject(mBody);

	// null the reference
	mBody = NULL;

}


//===============================================================================================
//dismount the owner from the physical body

void Vehicle::dismountPlayer()
{
 	if(!mBody)
	{
		gLogger->logMsg("Vehicle::dismountPlayer() no Vehicle Body!!!");
		return;
	}

	if(!mOwner->checkIfMounted())
	{
		gLogger->logMsg("Vehicle::dismountPlayer() not mounted");
		return;
	}

	gMessageLib->sendContainmentMessage_InRange(mOwner->getId(),mId, 4, mOwner);

	//For safe measures make the player equipped by nothing
	gMessageLib->sendContainmentMessage_InRange(mOwner->getId(), 0, 0xffffffff, mOwner);

	mBody->toggleStateOff(CreatureState_MountedCreature);
	mOwner->toggleStateOff(CreatureState_RidingMount);
	gMessageLib->sendStateUpdate(mBody);
	gMessageLib->sendStateUpdate(mOwner);

	mOwner->setMounted(false);

}

//===============================================================================================
//mount the owner on the physical body
//

void Vehicle::mountPlayer()
{
	if(!mBody)
	{
		gLogger->logMsg("Vehicle::mountPlayer() no Vehicle Body!!!");
		 return;
	}

	CreatureObject* body = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(this->getId()+1));
	if(!body)
	{
		gLogger->logMsg("Vehicle::mountPlayer() no Vehicle Body by Id :(!!!");
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
			gLogger->logMsg("Vehicle::mountPlayer() getObjects in Range :: PlayerObject invalid!!!");
			continue;
		}

		PlayerObject* tested = gWorldManager->getPlayerByAccId((*it)->getAccountId());
		if (!tested)
		{
			++it;
			gLogger->logMsg("Vehicle::mountPlayer() getObjects in Range :: PlayerObject invalid!!!");
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
