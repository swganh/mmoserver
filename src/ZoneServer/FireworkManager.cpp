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
#include "FireworkManager.h"

#include "Item_Enums.h"
#include "WorldManager.h"
#include "PlayerObject.h"
#include "StaticObject.h"
#include "StateManager.h"
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
TangibleObject* FireworkManager::createFirework(uint32 typeId, PlayerObject* player, const glm::vec3& position)
{
    if(!player) return NULL;

	if(player->states.checkState(CreatureState_Swimming))
	{
		//use the system message from suveying as we don't have an appropriate one especiially for this
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "survey_swimming"), player);
        return NULL;
    }

    //this is by definition a nonpersistant object - so move it there
    TangibleObject* firework = new TangibleObject();
    firework->setTangibleGroup(TanGroup_Static);
    //firework->setTangibleType();

    //Make the Player Sit
    player->states.setPosture(CreaturePosture_Crouched);

    // Place the firework 1m in front of the player at the same heading.
    firework->mDirection = player->mDirection;

    firework->mPosition = player->mPosition;
    firework->moveForward(1);

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
			DLOG(WARNING) << "Error creating firework, type:" << typeId;
			return NULL;
		}
	}

	//add it to the world!!!
	gWorldManager->addObject(firework);
	
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
			if((*it)->player->states.getPosture() == CreaturePosture_Crouched)
			{
				gStateManager.setCurrentPostureState((*it)->player, CreaturePosture_Upright);
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
