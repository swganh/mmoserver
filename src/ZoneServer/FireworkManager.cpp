/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "FireworkManager.h"

#include "Item_Enums.h"
#include "WorldManager.h"
#include "PlayerObject.h"
#include "StaticObject.h"
#include "UIManager.h"

#include "MessageLib/MessageLib.h"


FireworkManager*	FireworkManager::mSingleton = NULL;

FireworkManager::~FireworkManager(void)
{
}


//===============================================================================00
//creates the static Object of the firework in the world
//
TangibleObject* FireworkManager::createFirework(uint32 typeId, PlayerObject* player, Anh_Math::Vector3 position)
{
	//this is by definition a nonpersistant object - so move it there 
	TangibleObject* firework = new TangibleObject();
	firework->setTangibleGroup(TanGroup_Static);
	//firework->setTangibleType();

	//Make the Player Sit
	player->setCrouched();

	firework->mPosition.mX = player->mPosition.mX + ( 2 * cos(player->mDirection.getAnglesToSend()));
	firework->mPosition.mY = position.mY;
	firework->mPosition.mZ = player->mPosition.mZ + ( 2 * sin(player->mDirection.getAnglesToSend()));

	firework->mDirection.mX = 0;
	firework->mDirection.mY = 0;
	firework->mDirection.mZ = 0;
	firework->mDirection.mW = 1;
	firework->setId(gWorldManager->getRandomNpId());

	switch(typeId)
	{
	case ItemType_Firework_Type_1: 
		firework->setModelString("object/static/firework/shared_fx_01.iff");
		break;
	case ItemType_Firework_Type_2:
		firework->setModelString("object/static/firework/shared_fx_02.iff");
		break;
	case ItemType_Firework_Type_3: 
		firework->setModelString("object/static/firework/shared_fx_03.iff");
		break;
	case ItemType_Firework_Type_4: 
		firework->setModelString("object/static/firework/shared_fx_04.iff");
		break;
	case ItemType_Firework_Type_5: 
		firework->setModelString("object/static/firework/shared_fx_05.iff");
		break;
	case ItemType_Firework_Type_10: 
		firework->setModelString("object/static/firework/shared_fx_10.iff");
		break;
	case ItemType_Firework_Type_11: 
		firework->setModelString("object/static/firework/shared_fx_11.iff");
		break;
	case ItemType_Firework_Type_18: 
		firework->setModelString("object/static/firework/shared_fx_18.iff");
		break;
	case ItemType_Firework_Show: 
		firework->setModelString("object/static/firework/shared_show_launcher.iff");
		break;

	default:
		{
			gLogger->logMsgF("Error creating firework, type:%u",MSG_NORMAL, typeId);
			return NULL;
		}
	}

	//add it to the world!!!
	gWorldManager->addObject(firework);
	gWorldManager->createObjectinWorld(player,firework);
	
	return firework;
}
