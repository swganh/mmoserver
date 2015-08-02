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

#include "CraftingTool.h"
#include "CurrentResource.h"
#include "Inventory.h"
#include "Item.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "ResourceContainer.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "ContainerManager.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "NetworkManager/Message.h"

#include <boost/lexical_cast.hpp>

#include <cassert>

//======================================================================================================================
//
// transfers resources across resource containers
//

void ObjectController::_handleResourceContainerTransfer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*		playerObject		= dynamic_cast<PlayerObject*>(mObject);
    ResourceContainer*	selectedContainer	= dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(targetId));

    if(selectedContainer)
    {
        BString dataStr;

        message->getStringUnicode16(dataStr);
        dataStr.convert(BSTRType_ANSI);

        BStringVector dataElements;

        uint16 elementCount = dataStr.split(dataElements,' ');

        if(!elementCount)
        {
            DLOG(INFO) << "ObjectController::_handleResourceContainerTransfer: Error in requestStr";
            return;
        }

        ResourceContainer* targetContainer = dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(boost::lexical_cast<uint64>(dataElements[0].getAnsi())));

        if(targetContainer && targetContainer->getResourceId() == selectedContainer->getResourceId())
        {
            uint32	targetAmount	= targetContainer->getAmount();
            uint32	selectedAmount	= selectedContainer->getAmount();
            uint32	maxAmount		= targetContainer->getMaxAmount();
            uint32	newAmount;

            // all fits
            if((newAmount = targetAmount + selectedAmount) <= maxAmount)
            {
                // update target container
                targetContainer->setAmount(newAmount);

                gMessageLib->sendResourceContainerUpdateAmount(targetContainer,playerObject);

                mDatabase->executeSqlAsync(NULL,NULL,"UPDATE %s.resource_containers SET amount=%u WHERE id=%" PRIu64 "",mDatabase->galaxy(),newAmount,targetContainer->getId());

                // delete old container
				TangibleObject* container = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(selectedContainer->getParentId()));
				gContainerManager->deleteObject(selectedContainer, container);
                
            }
            // target container full, update both contents
            else if(newAmount > maxAmount)
            {
                uint32 selectedNewAmount = newAmount - maxAmount;
                targetContainer->setAmount(maxAmount);
                selectedContainer->setAmount(selectedNewAmount);

                gMessageLib->sendResourceContainerUpdateAmount(targetContainer,playerObject);
                gMessageLib->sendResourceContainerUpdateAmount(selectedContainer,playerObject);

                mDatabase->executeSqlAsync(NULL,NULL,"UPDATE %s.resource_containers SET amount=%u WHERE id=%" PRIu64 "",mDatabase->galaxy(),maxAmount,targetContainer->getId());
                
                mDatabase->executeSqlAsync(NULL,NULL,"UPDATE %s.resource_containers SET amount=%u WHERE id=%" PRIu64 "",mDatabase->galaxy(),selectedNewAmount,selectedContainer->getId());
                
            }
        }
    }
}

//======================================================================================================================
//
// splits up a resource container
//

void ObjectController::_handleResourceContainerSplit(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*		playerObject		= dynamic_cast<PlayerObject*>(mObject);
    ResourceContainer*	selectedContainer	= dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(targetId));

	if(!selectedContainer)
    {
        DLOG(INFO) << "ObjectController::_handleResourceContainerSplit: Container does not exist!";
        return;
    }

    BString dataStr;

    message->getStringUnicode16(dataStr);
    dataStr.convert(BSTRType_ANSI);

    BStringVector dataElements;
    uint16 elementCount = dataStr.split(dataElements,' ');

    if(!elementCount)
    {
        DLOG(INFO) << "ObjectController::_handleResourceContainerSplit: Error in requestStr";
        return;
    }

    uint32	splitOffAmount	= boost::lexical_cast<uint32>(dataElements[0].getAnsi());
    uint64	parentId		= boost::lexical_cast<uint64>(dataElements[1].getAnsi());


    TangibleObject* parentContainer = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(parentId));
    if(!parentContainer)
    {
        assert(false && "ObjectController::_handleResourceContainerSplitresourcecontainers parent does not exist");
        return;
    }

    //res container is 1 slot
    if(!parentContainer->checkCapacity(1,playerObject))
    {
        //check if we can fit an additional item in our inventory
        //sends sysmessage automatically
        //gMessageLib->SendSystemMessage(::common::OutOfBand("container_error_message", "container3"), playerObject);
        return;
    }
    // update selected container contents
    selectedContainer->setAmount(selectedContainer->getAmount() - splitOffAmount);
    mDatabase->executeSqlAsync(NULL,NULL,"UPDATE %s.resource_containers SET amount=%u WHERE id=%" PRIu64 "",mDatabase->galaxy(),selectedContainer->getAmount(),selectedContainer->getId());

    gMessageLib->sendResourceContainerUpdateAmount(selectedContainer,playerObject);

    gObjectFactory->requestNewResourceContainer(parentContainer,(selectedContainer->getResource())->getId(),parentId,99,splitOffAmount);
}

