/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

//#include "BankTerminal.h"
//#include "CraftingTool.h"
//#include "CurrentResource.h"
#include "Inventory.h"
#include "Item.h"
//#include "NPCObject.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "FactoryCrate.h"
//#include "UIManager.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "ContainerManager.h"

#include "MessageLib/MessageLib.h"
#include "NetworkManager/Message.h"
#include <boost/lexical_cast.hpp>

#include <cassert>

//======================================================================================================================
//splits a factory crate

void	ObjectController::_handleFactoryCrateSplit(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

}
//======================================================================================================================
//extracts an item out of a factory crate

void	ObjectController::_ExtractObject(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*		playerObject		= dynamic_cast<PlayerObject*>(mObject);
    FactoryCrate*		crate				= dynamic_cast<FactoryCrate*>(gWorldManager->getObjectById(targetId));

    if(!crate)
    {
        DLOG(INFO) << "ObjectController::_ExtractObject: Crate does not exist!";
        return;
    }

    //get the crates containing container  we can use the unified interface thks to virtual functions :)
    TangibleObject* tO = dynamic_cast<TangibleObject* >(gWorldManager->getObjectById(crate->getParentId()));
    if(!tO)
    {
        assert(false && "ObjectController::_ExtractObject inventory must be a tangible object");
        return;
    }

    if(!tO->checkCapacity())
    {
        //check if we can fit an additional item in our inventory
        //TODO: say something
        return;
    }

    //create the new item
    gObjectFactory->requestNewClonedItem(tO,crate->getLinkedObject()->getId(),tO->getId());

    //decrease crate content
    int32 content = crate->decreaseContent(1);
    if(!content)
    {
		TangibleObject* container = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(crate->getParentId()));
		gContainerManager->deleteObject(crate, container);
        
        return;
    }

    if(content < 0)
    {
        assert(false && "ObjectController::_ExtractObject crate must not have negative content");
        return;
    }

    gMessageLib->sendUpdateCrateContent(crate,playerObject);

    return;


}
