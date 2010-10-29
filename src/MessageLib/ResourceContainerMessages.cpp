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

#include "MessageLib.h"

#include "ZoneServer/CurrentResource.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/ResourceContainer.h"
#include "ZoneServer/ResourceManager.h"
#include "ZoneServer/ResourceType.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"



#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

#include <boost/lexical_cast.hpp>

//======================================================================================================================
//
// Resource Container Baselines Type 3
// contain: name,type,amount
//

bool MessageLib::sendBaselinesRCNO_3(ResourceContainer* resourceContainer,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    Message*	message;
    BString		resourceNameFile = "kb/kb_resources_n";
    BString		resourceTypeName = ((resourceContainer->getResource())->getType())->getTypeName();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(resourceContainer->getId());
    mMessageFactory->addUint32(opRCNO);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(61 + resourceNameFile.getLength() + resourceTypeName.getLength());
    mMessageFactory->addUint16(13);
    mMessageFactory->addFloat(0);
    mMessageFactory->addString(resourceNameFile);
    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addString(resourceTypeName);

    mMessageFactory->addUint32(0);//customname
    mMessageFactory->addUint32(1);	// volume

    mMessageFactory->addUint16(0);//ascii customization

    // unknown list			   probably inherited from tano the customization crc of special attachments like scope etc
    // so most likely not related to resources
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);

    mMessageFactory->addUint8(0);//OptionsBitmask figure the bitmap out!!!!! 4 is insured
    mMessageFactory->addUint8(0);//OptionsBitmask figure the bitmap out!!!!!
    mMessageFactory->addUint8(0);//OptionsBitmask figure the bitmap out!!!!!
    mMessageFactory->addUint8(0);//OptionsBitmask figure the bitmap out!!!!!
    //4 is insured

    //timer incap (unused)
    mMessageFactory->addUint32(0);	// unknown
    //cond damage
    mMessageFactory->addUint32(0);
    //max condition
    mMessageFactory->addUint32(100);

    //unknown - bitmask?????
    mMessageFactory->addUint8(0);
    mMessageFactory->addUint32(resourceContainer->getAmount());
    mMessageFactory->addUint64((resourceContainer->getResource())->getId());

    message = mMessageFactory->EndMessage();

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    Resource* resource = gResourceManager->getResourceById((resourceContainer->getResource())->getId());

    if(resource != NULL)
    {
        // this info gets automatically requested by the client every time we create the resource from scratch
        // it will NOT be requested when we load the resource on login / inventory creation
        // this makes me think whether we do something wrong there ???
        //should we end the inventory baseline and THEN send the inventory content ????
        // however this will fix the greenbar problem while crafting !!!
        resource->sendAttributes(targetObject);
    }

    return(true);
}

//======================================================================================================================
//
// Resource Container Baselines Type 6
// contain: type description, max amount
//

bool MessageLib::sendBaselinesRCNO_6(ResourceContainer* resourceContainer,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    Message*	message;
    BString		unknownStr = "";
    BString		resourceTypeDescriptor = (((resourceContainer->getResource())->getType())->getDescriptor());
    BString		resourceName = (resourceContainer->getResource())->getName().getAnsi();
    resourceName.convert(BSTRType_Unicode16);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(resourceContainer->getId());
    mMessageFactory->addUint32(opRCNO);
    mMessageFactory->addUint8(6);

    mMessageFactory->addUint32(32 + (unknownStr.getLength() << 1) + resourceTypeDescriptor.getLength() + (resourceName.getLength() << 1));
    mMessageFactory->addUint16(5);
    mMessageFactory->addFloat(1.0);	// unknown
    mMessageFactory->addString(unknownStr);
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(unknownStr);
    mMessageFactory->addUint32(100000); // max stack size ?
    mMessageFactory->addString(resourceTypeDescriptor);
    mMessageFactory->addString(resourceName);


    message = mMessageFactory->EndMessage();

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Resource Container Baselines Type 8
// contain: unknown
//

bool MessageLib::sendBaselinesRCNO_8(ResourceContainer* resourceContainer,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    Message*	message;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(resourceContainer->getId());
    mMessageFactory->addUint32(opRCNO);
    mMessageFactory->addUint8(8);

    mMessageFactory->addUint32(2);
    mMessageFactory->addUint16(0);

    message = mMessageFactory->EndMessage();

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Resource Container Baselines Type 9
// contain: unknown
//

bool MessageLib::sendBaselinesRCNO_9(ResourceContainer* resourceContainer,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    Message*	message;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(resourceContainer->getId());
    mMessageFactory->addUint32(opRCNO);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(2);
    mMessageFactory->addUint16(0);

    message = mMessageFactory->EndMessage();

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Resource Container Deltas Type 3
// update: amount
//

bool MessageLib::sendResourceContainerUpdateAmount(ResourceContainer* resourceContainer,PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(resourceContainer->getId());
    mMessageFactory->addUint32(opRCNO);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(8);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(11);
    mMessageFactory->addUint32(resourceContainer->getAmount());

    newMessage = mMessageFactory->EndMessage();

    (playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

    return(true);
}

