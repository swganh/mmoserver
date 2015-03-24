/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "ZoneServer/GameSystemManagers/Spatial Index Manager/SpatialIndexManager.h"

#include <cassert>
#include "NetworkManager/MessageFactory.h"
#include "MessageLib/MessageLib.h"

#include "ZoneServer/GameSystemManagers/Structure Manager/BuildingObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/PlayerStructure.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/FactoryCrate.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/HouseObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/CellObject.h"
#include "ZoneServer/GameSystemManagers/Container Manager/ContainerManager.h"
#include "ZoneServer/Objects/Datapad.h"

#include "ZoneServer/Objects/Inventory.h"
#include "ZoneServer/Objects/IntangibleObject.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/ManufacturingSchematic.h"
#include "ZoneServer/GameSystemManagers/Mission Manager/MissionObject.h"
#include "ZoneServer/GameSystemManagers/Mission Manager/MissionBag.h"
#include "ZoneServer/Objects/MountObject.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"

#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceContainer.h"
#include "ZoneServer/Objects/Tangible Object/TangibleObject.h"
#include "ZoneServer/Objects/VehicleController.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "ZoneServer/ZoneServer.h"


//======================================================================================================================
//
// send the matching object creates
//
bool SpatialIndexManager::sendCreateObject(Object* object,PlayerObject* player)
{
<<<<<<< HEAD:src/ZoneServer/GameSystemManagers/Spatial Index Manager/SpatialIndexMessagingServices.cpp
    //DLOG(info) << "SpatialIndexManager::sendCreateObject: create :" << object->getId() << "for :" << player->getId();
	
=======
    //DLOG(INFO) << "SpatialIndexManager::sendCreateObject: create :" << object->getId() << "for :" << player->getId();

>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/SpatialIndexMessagingServices.cpp
    if(!object)
    {
        DLOG(INFO) << "Attempting sendCreateObject on an invalid object instance";
        return false;
    }

	// skip, if its static
    if(object->getId() <= 4294967296)        {
        //skip statics
        return false;
    }


	switch(object->getObjectType())
    {
	case SWG_CREATURE:
		{
			CreatureObject* targetCreature = dynamic_cast<CreatureObject*>(object);
			if(!targetCreature)	{
				DLOG(info) << "SpatialIndexManager::sendCreateObject : invalid Object couldn cast creature";
				return false;
			}
			
			return gMessageLib->sendCreateCreature(targetCreature,player);

		}
		break;
	
	case SWG_PLAYER :
		{
			PlayerObject* ghost = dynamic_cast<PlayerObject*>(object);
			if(!ghost)	{
				DLOG(info) << "SpatialIndexManager::sendCreateObject : invalid Object couldn cast player";
				return false;
			}

			return gMessageLib->sendCreateGhost(ghost, player);

		}
		break;
	}


    switch(object->getType())
    {
    

    // players
    /*case ObjType_Player:
    {
        // send creates to each other
        if(PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(object))        {
            gMessageLib->sendCreatePlayer(targetPlayer, player);
			//sendCreatePlayer(targetPlayer,player);
        }
    }
        
    break;
	*/
	case ObjType_Intangible:
	{
		IntangibleObject* intangible = dynamic_cast<IntangibleObject*>(object);
		if(intangible)	{
			gMessageLib->sendCreateInTangible(intangible, player);
		}

	}
	break;

    // tangibles
    case ObjType_Tangible:
    {
       
        TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);
        sendCreateTangible(tangibleObject, player, false);
    }
    break;

    // buildings
    case ObjType_Building:
    {

        {
            if(BuildingObject* building = dynamic_cast<BuildingObject*>(object))
            {
                gMessageLib->sendCreateBuilding(building,player);
            }
        }
    }
    break;

    case ObjType_Structure:
    {
        // skip, if its static

        {
            if(PlayerStructure* structure = dynamic_cast<PlayerStructure*>(object))
            {
                gMessageLib->sendCreateStructure(structure,player);
            }
        }
    }
    break;

    // unknown types
    default:
    {
<<<<<<< HEAD:src/ZoneServer/GameSystemManagers/Spatial Index Manager/SpatialIndexMessagingServices.cpp
        DLOG(info) << "MessageLib::createObject: Unhandled object type: " << object->getType();
		return false;
=======
        DLOG(INFO) << "MessageLib::createObject: Unhandled object type: " << object->getType();
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/SpatialIndexMessagingServices.cpp
    }
    break;
    }

    return true;
}

//======================================================================================================================
//
// create tangible descendant Objects in the world
//
bool SpatialIndexManager::sendCreateTangible(TangibleObject* tangibleObject,PlayerObject* targetObject, bool sendchildren)
{

    if(ResourceContainer* resContainer = dynamic_cast<ResourceContainer*>(tangibleObject))
    {
        return gMessageLib->sendCreateResourceContainer(resContainer,targetObject);
    }
    else if(FactoryCrate* crate = dynamic_cast<FactoryCrate*>(tangibleObject))
    {
        return sendCreateFactoryCrate(crate,targetObject);
    }
    else if(tangibleObject->getTangibleGroup() == TanGroup_Static)
    {
        return gMessageLib->sendCreateStaticObject(tangibleObject,targetObject);
    }

    uint64 parentId = tangibleObject->getParentId();

    gMessageLib->sendCreateTano(tangibleObject, targetObject);

    /*

    //now check whether we have children!!!
    ObjectIDList*			ol = tangibleObject->getObjects();
    ObjectIDList::iterator	it = ol->begin();

    while(it != ol->end())
    {
    	TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById((*it)));
    	if(!tO)
    	{
    		DLOG(INFO) << "Unable to find object with ID " << (*it);
    		it++;
    		continue;
    	}

    	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetObject->getId()));
    	sendCreateObject(tO,player,false);
    	it++;
    }
    */

    return(true);
}

//======================================================================================================================
//
// create tangible
//
void SpatialIndexManager::sendCreateTangible(TangibleObject* tangibleObject, ObjectList*	knownPlayers, bool sendchildren)
{
    ObjectList::iterator it = knownPlayers->begin();

    while(it != knownPlayers->end())
    {
        PlayerObject* targetObject = dynamic_cast<PlayerObject*>(*it);

        sendCreateTangible(tangibleObject, targetObject, sendchildren);

        ++it;
    }
}

//======================================================================================================================
//
// create player
//
bool SpatialIndexManager::sendCreatePlayer(PlayerObject* playerObject,PlayerObject* targetObject)
{
	
    if(!playerObject->GetCreature()->registerWatcher(targetObject))	{
		LOG(info) << "SpatialIndexManager::sendCreatePlayer : " << playerObject->getId() << "already known";
	}
	
	playerObject->registerWatcher(targetObject);

	gMessageLib->sendCreatePlayer(playerObject, targetObject);
	

	playerObject->GetCreature()->ViewObjects(playerObject->GetCreature(), 0, true, [&] (Object* object) {
		if(!object->registerWatcher(playerObject))	{

			LOG(info) << "SpatialIndexManager::sendCreatePlayer : " << object->getId() << "already known";
			return;
		}

		TangibleObject* tangible = dynamic_cast<TangibleObject*>(object);
		if(tangible)	{
		
			LOG(info) << "creating : " << object->getId() << " " << object->GetTemplate();
			sendCreateTangible(tangible,targetObject);
		}
		
		IntangibleObject* intangible = dynamic_cast<IntangibleObject*>(object);
		if(intangible && (intangible->getId() != playerObject->getId()))	{
			LOG(info) << "creating intangible : " << object->getId() << " " << object->GetTemplate();
			gMessageLib->sendCreateInTangible(intangible, targetObject);
		}
	
	});

	//playerObject->setLoadState(LoadState_Loaded);


	/*
    // tangible objects
    if(TangibleObject* hair = dynamic_cast<TangibleObject*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hair)))
    {
        //only create the hair as the helmet will be created at a different time
        if(hair->getTangibleType() == TanType_Hair)
        {
            sendCreateTangible(hair,targetObject);
        }
    }

    if(targetObject == playerObject)
    {

        // create inventory and contents
        Inventory* inventory = playerObject->getInventory();
        if(inventory)
        {
            sendInventory(playerObject);
        }

        // mission bag
        if(TangibleObject* missionBag = dynamic_cast<TangibleObject*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Mission)))
        {
            gMessageLib->sendCreateTano(missionBag,playerObject);

            //Now iterate through the missions and create them clientside
            MissionBag* mbag = dynamic_cast<MissionBag*> (missionBag);
            MissionList::iterator it = mbag->getMissions()->begin();
            while(it != mbag->getMissions()->end())
            {
                MissionObject* mission = dynamic_cast<MissionObject*>(*it);
                gMessageLib->sendCreateObjectByCRC(mission, targetObject, false);
                gMessageLib->sendContainmentMessage(mission->getId(), mbag->getId(), 0xffffffff, targetObject);
                gMessageLib->sendBaselinesMISO_3(mission, targetObject);
                gMessageLib->sendBaselinesMISO_6(mission, targetObject);
                gMessageLib->sendBaselinesMISO_8(mission, targetObject);
                gMessageLib->sendBaselinesMISO_9(mission, targetObject);
                gMessageLib->sendEndBaselines(mission->getId(), targetObject);
                ++it;
            }

        }

        // datapad
        Datapad* datapad			= playerObject->getDataPad();
        if(datapad)
        {
            //would be nice to use the tangibles objectcontainer for the datapad
            //need to get missionobjects intangibles, Man Schematics, waypoints and stuff in though, so better do it manually
            gMessageLib->sendCreateTano(datapad,playerObject);

            //now iterate through the schematics and create them clientside
            Datapad* dpad = dynamic_cast<Datapad*> (datapad);

            ManufacturingSchematicList*	manufacturingSchematics = dpad->getManufacturingSchematics();
            ManufacturingSchematicList::iterator it = manufacturingSchematics->begin();

            while(it != manufacturingSchematics->end())
            {
                gMessageLib->sendCreateManufacturingSchematic((dynamic_cast<ManufacturingSchematic*>(*it)),playerObject ,false);
                ++it;
            }

            //Send player's intangibles vehicles,pets,droids...etc
            DataList* intangibles = dpad->getData();
            DataList::iterator ite = intangibles->begin();

            while(ite != intangibles->end())
            {
                if(IntangibleObject* itno = dynamic_cast<IntangibleObject*>(*ite))
                {
                    gMessageLib->sendCreateInTangible(itno, dpad->getId(), playerObject);

                    //dont add it to the MainObjectMap
                    //gWorldManager->addObject(itno,true);

                    switch(itno->getItnoGroup())
                    {
                    case ItnoGroup_Vehicle:
                    {
                        // set Owner for vehicles
                        if(VehicleController* vehicle = dynamic_cast<VehicleController*>(itno))
                        {
                            vehicle->set_owner(playerObject);
                        }
                    }
                    break;

                    default:
                        break;
                    }
                }

                ++ite;
            }

            //Should send accepted missions here

        }
    }
    else
    {
        //sendEndBaselines(playerObject->getId(),targetObject);
        sendEquippedItems(playerObject,targetObject);
    }


    gMessageLib->sendUpdatePvpStatus(playerObject,targetObject);

    if(targetObject == playerObject)
    {
        // We are actually sending this info from CharacterLoginHandler::handleDispatchMessage at the opCmdSceneReady event.
        // sendFriendListPlay9(playerObject);
        // sendIgnoreListPlay9(playerObject);

        //request the GRUP baselines from chatserver if grouped
        if(playerObject->getGroupId() != 0)
        {
            gMessageLib->sendIsmGroupBaselineRequest(playerObject);
        }
    }

    //Player mounts
    if(playerObject->checkIfMountCalled())
    {
        if(playerObject->getMount())
        {
            sendCreateObject(playerObject->getMount(),targetObject,false);
            if(playerObject->checkIfMounted())
            {
                gMessageLib->sendContainmentMessage(playerObject->getId(), playerObject->getMount()->getId(), 0xffffffff, targetObject);
            }
        }
    }
	*/
    return(true);
}

<<<<<<< HEAD:src/ZoneServer/GameSystemManagers/Spatial Index Manager/SpatialIndexMessagingServices.cpp
=======
//======================================================================================================================
//
// create the inventory contents for its owner
//
void SpatialIndexManager::sendInventory(PlayerObject* playerObject)
{

    Inventory*	inventory	= playerObject->getInventory();

    //to stop the server from crashing.
    if(!inventory)
    {
        assert(false && "SpatialIndexManager::sendInventory cannot find inventory");
        return;
    }

    inventory->setTypeOptions(256);

    //todo - just use sendcreate tangible and have it send the children, too!!!!

    // create the inventory
    gMessageLib->sendCreateObjectByCRC(inventory,playerObject,false);
    gMessageLib->sendContainmentMessage(inventory->getId(),inventory->getParentId(),4,playerObject);
    gMessageLib->sendBaselinesTANO_3(inventory,playerObject);
    gMessageLib->sendBaselinesTANO_6(inventory,playerObject);

    gMessageLib->sendEndBaselines(inventory->getId(),playerObject);

    // create objects contained *always* even if already registered
    // register them as necessary
    // please note that they need to be created even if already registered (client requirement)
    inventory->registerStatic(playerObject);
    playerObject-> registerStatic(inventory);

    ObjectIDList* invObjects		= inventory->getObjects();
    ObjectIDList::iterator objIt	= invObjects->begin();

    while(objIt != invObjects->end()) 
    {
        Object* object = gWorldManager->getObjectById((*objIt));
        if(TangibleObject* tangible = dynamic_cast<TangibleObject*>(object))
        {
            sendCreateTangible(tangible,playerObject);
            
			//TODO make this static as its si independent??
            gContainerManager->registerPlayerToContainer(tangible,playerObject);//eventually move the registration to the factory ???
        }

        //sendCreateObject(object,playerObject,false);
        ++objIt;
    }

    //creating the equipped Objects isnt technically part of the inventory ...
    ObjectList* invEquippedObjects		= playerObject->getEquipManager()->getEquippedObjects();
    ObjectList::iterator objEIt			= invEquippedObjects->begin();

    while(objEIt != invEquippedObjects->end())
    {
        if(TangibleObject* tangible = dynamic_cast<TangibleObject*>(*objEIt))
        {
            sendCreateTangible(tangible,playerObject);
            //TODO make this static as its si independent ???
            gContainerManager->registerPlayerToContainer(tangible,playerObject);//eventually move the registration to the factory
        }

        ++objEIt;
    }

}

//======================================================================================================================
//
// send creates of the equipped items from player to player
// iterates all inventory items of the source and sends creates to the target
//
bool SpatialIndexManager::sendEquippedItems(PlayerObject* srcObject,PlayerObject* targetObject)
{
    ObjectList*				invObjects		= srcObject->getEquipManager()->getEquippedObjects();
    ObjectList::iterator	invObjectsIt	= invObjects->begin();

    while(invObjectsIt != invObjects->end())
    {
        // items
        if(Item* item = dynamic_cast<Item*>(*invObjectsIt))
        {
            if(item->getParentId() == srcObject->getId())
            {
                sendCreateTangible(item,targetObject);
            }
            else
            {
                DLOG(INFO) << "MessageLib send equipped objects: Its not equipped ... " << item->getId();
            }
        }

        ++invObjectsIt;
    }

    return(true);
}
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/SpatialIndexMessagingServices.cpp

//======================================================================================================================
//
// create factory crate
//
bool SpatialIndexManager::sendCreateFactoryCrate(FactoryCrate* crate,PlayerObject* targetObject)
{


    gMessageLib->sendCreateObjectByCRC(crate,targetObject);

    uint64 parentId = crate->getParentId();

    gMessageLib->sendContainmentMessage(crate->getId(),parentId,0xffffffff,targetObject);

    gMessageLib->sendBaselinesTYCF_3(crate,targetObject);
    gMessageLib->sendBaselinesTYCF_6(crate,targetObject);

    gMessageLib->sendBaselinesTYCF_8(crate,targetObject);
    gMessageLib->sendBaselinesTYCF_9(crate,targetObject);

    //check for our linked item and create it
    ObjectIDList*			ol = crate->GetObjectData();
    ObjectIDList::iterator	it = ol->begin();

    while(it != ol->end())
    {
        TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById((*it)));
        if(!tO)
        {
            DLOG(INFO) << "Unable to find object with ID " <<  (*it);
            continue;
        }

        //PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetObject->getId()));
        gMessageLib->sendCreateTano(tO,targetObject);

        it++;
    }

    gMessageLib->sendEndBaselines(crate->getId(),targetObject);

    //now get the contained tangible and create it
    //sendCreateTangible();

    return(true);
}

//======================================================================================================================
//
//make sure we scale down viewing range under high load
//returns the amount of fields in our viewing range
uint32 SpatialIndexManager::_GetMessageHeapLoadGridRange()
{
    //when changing the viewrange we need to alter ALL objects
    return VIEWRANGE;

    //so keep the same until implemented

    float divider = 4/VIEWRANGE;

    uint32 heapWarningLevel = gMessageFactory->HeapWarningLevel();

    uint32 view = VIEWRANGE;

    //make sure we dont crash our heap
    if(gMessageFactory->getHeapsize() > 90.0)
        return 0; //means just the grid the player is in is returned

    //just send everything we have
    if(heapWarningLevel < 3)
        return view;
    else if(heapWarningLevel < 4)
        view = (uint32) floor((divider * 3));
    else if (heapWarningLevel < 5)
    {
        view = (uint32) floor((divider * 2));
    }
    else if (heapWarningLevel < 8)
    {
        view = (uint32) floor((divider * 1));
    }
    else if (heapWarningLevel <= 10)
    {
        view = 0;
    }
    else if (heapWarningLevel > 10)
        return (uint32) 0;

    if(view > VIEWRANGE)
        view = VIEWRANGE;

    return view;
}
