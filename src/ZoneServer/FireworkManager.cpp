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

class FireworkEvent
{
public:
	TangibleObject* firework;
	uint64 timeFired; //Time the firework was fired.
	bool playerToldToStand;
	PlayerObject* player; //Person who fired the Firework
};

FireworkManager::~FireworkManager(void)
{
	std::list<FireworkEvent*>::iterator it=fireworkEvents.begin();
	std::list<FireworkEvent*>::iterator fEnd = fireworkEvents.end();
	while( it != fEnd)
	{
		if(*it)
			delete *it;
		it = fireworkEvents.erase(it);
	}
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

	//The placement formulas may require *slight* tweaking as they don't place directly in front of the player
	//in the same spot every time.
	firework->mPosition.mX = player->mPosition.mX + ( cos(player->mDirection.getAnglesToSend()));
	firework->mPosition.mY = position.mY;
	firework->mPosition.mZ = player->mPosition.mZ + ( sin(player->mDirection.getAnglesToSend()));

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
	
	FireworkEvent* fevent = new FireworkEvent;

	//Setup the Manager Class
	fevent->firework = firework;
	fevent->player = player;
	fevent->playerToldToStand = false;
	fevent->timeFired = gWorldManager->GetCurrentGlobalTick();

	this->fireworkEvents.push_back(fevent);

	return firework;
}

void FireworkManager::Process()
{
	//This iterates all fired fireworks and keeps everything spiffy.

	std::list<FireworkEvent*>::iterator it=this->fireworkEvents.begin();
	std::list<FireworkEvent*>::iterator fEnd = fireworkEvents.end();

	//We can do this outside the while...We likely won't get a vastly different value while in it anyway.
	uint64 currentTime = gWorldManager->GetCurrentGlobalTick();
	while( it != fEnd)
	{
		if(*it && (currentTime - (*it)->timeFired) > 2000 && (*it)->playerToldToStand == false) //2 sec
		{
			if((*it)->player->getPosture() == CreaturePosture_Crouched)
			{
				(*it)->player->setUpright();
				(*it)->playerToldToStand = true;
			}
			++it;
		}
		else if(*it && (currentTime - (*it)->timeFired) > 25000) //25 sec (about the time of a firework)
		{
			gWorldManager->destroyObject((*it)->firework);

			delete *it;
			it = fireworkEvents.erase(it);
		}
		else
		{
			++it;
		}
	}

}