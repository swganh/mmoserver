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

#include "ZoneServer/Deed.h"
#include "ZoneServer/ResourceCategory.h"
#include "ZoneServer/ResourceManager.h"
#include "ZoneServer/ResourceType.h"

#include "ZoneServer/HarvesterObject.h"
#include "ZoneServer/FactoryObject.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "ZoneServer/ObjectControllerOpcodes.h"


#ifdef ERROR
#undef ERROR
#endif

#include "Utils/logger.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"


//======================================================================================================================
//
// Building Baselines Type 3
// contain: name,
//

bool MessageLib::sendBaselinesHINO_3(HarvesterObject* harvester,PlayerObject* player)
{
    if(!(player->isConnected()))
        return(false);

    Message* message;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint16(16);
    mMessageFactory->addFloat(1.0);
    mMessageFactory->addString(harvester->getNameFile());
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(harvester->getName());

    BString name;
    name = harvester->getCustomName();
    name.convert(BSTRType_Unicode16);

    mMessageFactory->addString(name.getUnicode16());

    mMessageFactory->addUint32(1);//volume (in inventory)
    mMessageFactory->addUint16(0);//customization
    mMessageFactory->addUint32(0);//list
    mMessageFactory->addUint32(0);//list

    if(harvester->getActive())
        mMessageFactory->addUint8(1);//optionsbitmask - 1 = active
    else
        mMessageFactory->addUint8(0);//optionsbitmask - vendor etc harvester running

    mMessageFactory->addUint8(0);//optionsbitmask - vendor etc harvester running
    mMessageFactory->addUint8(0);//optionsbitmask - vendor etc harvester running
    mMessageFactory->addUint8(0);//optionsbitmask - vendor etc harvester running

    mMessageFactory->addUint32(0);//timer

    //08
    mMessageFactory->addUint32(harvester->getDamage());//condition damage

    //09
    mMessageFactory->addUint32(harvester->getMaxCondition());   //maxcondition

    //A
    mMessageFactory->addUint8(harvester->getActive());//??

    //B
    mMessageFactory->addUint8(harvester->getActive());//active flag

    mMessageFactory->addFloat(0);//power reserve
    mMessageFactory->addFloat(0);//power cost
    mMessageFactory->addFloat(0);//
    mMessageFactory->addFloat(0);//




    message = mMessageFactory->EndMessage();

    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(harvester->getId());
    mMessageFactory->addUint32(opHINO);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(message->getSize());
    mMessageFactory->addData(message->getData(),message->getSize());

    newMessage = mMessageFactory->EndMessage();
    message->setPendingDelete(true);

    (player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Building Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesHINO_6(HarvesterObject* harvester,PlayerObject* player)
{
    if(!(player->isConnected()))
        return(false);

    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(harvester->getId());
    mMessageFactory->addUint32(opHINO);
    mMessageFactory->addUint8(6);

    mMessageFactory->addUint32(14);
    mMessageFactory->addUint16(2);	// unknown
    mMessageFactory->addUint16(0); // unknown
    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);

    newMessage = mMessageFactory->EndMessage();

    (player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Building Baselines Type 7
// contain: unknown
//

bool MessageLib::sendBaselinesHINO_7(HarvesterObject* harvester,PlayerObject* player)
{
    if(!(player->isConnected()))
        return(false);

    ResourceCategory*	mainCat = gResourceManager->getResourceCategoryById(harvester->getResourceCategory());
    ResourceList		resourceList;

    mainCat->getResources(resourceList,true);

    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint8(15);
    mMessageFactory->addUint8(0);
    mMessageFactory->addUint8(1);	//


    //=====================================
    //start with the resource IDS
    harvester->setUpdateCounter(resourceList.size());

    mMessageFactory->addUint32(resourceList.size());
    mMessageFactory->addUint32(harvester->getUpdateCounter());

    ResourceList::iterator resourceIt = resourceList.begin();

    while(resourceIt != resourceList.end() && (*resourceIt) != NULL)
    {
        Resource* tmpResource = (*resourceIt);

        mMessageFactory->addUint64(tmpResource->getId());

        ++resourceIt;
    }


    //=====================================
    //resource IDS a second time ... ???
    harvester->setUpdateCounter(resourceList.size());

    mMessageFactory->addUint32(resourceList.size());
    mMessageFactory->addUint32(harvester->getUpdateCounter());

    resourceIt = resourceList.begin();

    while(resourceIt != resourceList.end() && (*resourceIt) != NULL)
    {
        Resource* tmpResource = (*resourceIt);

        mMessageFactory->addUint64(tmpResource->getId());

        ++resourceIt;
    }



    //=====================================
    //resource names
    mMessageFactory->addUint32(resourceList.size());
    mMessageFactory->addUint32(harvester->getUpdateCounter());

    resourceIt = resourceList.begin();

    while(resourceIt != resourceList.end() && (*resourceIt) != NULL)
    {
        Resource* tmpResource = (*resourceIt);
        mMessageFactory->addString(tmpResource->getName().getAnsi());

        ++resourceIt;
    }



    //=====================================
    //resource types
    mMessageFactory->addUint32(resourceList.size());
    mMessageFactory->addUint32(harvester->getUpdateCounter());

    resourceIt = resourceList.begin();

    while(resourceIt != resourceList.end() && (*resourceIt) != NULL)
    {
        Resource* tmpResource = (*resourceIt);
        mMessageFactory->addString((tmpResource->getType())->getDescriptor().getAnsi());

        ++resourceIt;
    }

    mMessageFactory->addUint64(harvester->getCurrentResource());//current Res Id harvesting

    mMessageFactory->addUint8(harvester->getActive());//on off status flag
    mMessageFactory->addUint32(uint32(harvester->getSpecExtraction()));//hopper capacity

    mMessageFactory->addFloat(harvester->getSpecExtraction());//spec rate
    mMessageFactory->addFloat(harvester->getCurrentExtractionRate());//current rate

    mMessageFactory->addFloat(harvester->getCurrentHopperSize());//current hopper size
    mMessageFactory->addUint32((uint32)harvester->getHopperSize());//max Hoppersize

    mMessageFactory->addUint8(1);//	  hopper update flag


    HResourceList* rList = harvester->getResourceList();
    harvester->setRListUpdateCounter(rList->size());

    mMessageFactory->addUint32(rList->size());//listsize hopper contents
    mMessageFactory->addUint32(harvester->getRListUpdateCounter());//updatecounter hopper contents

    //harvester->setRListUpdateCounter(harvester->getRListUpdateCounter()+rList->size());

    HResourceList::iterator it = rList->begin();
    while (it != rList->end())
    {
        mMessageFactory->addUint64((*it).first);//
        mMessageFactory->addFloat((*it).second);//
        it++;
    }

    uint8 condition = ((harvester->getMaxCondition()-harvester->getDamage())/(harvester->getMaxCondition()/100));
    mMessageFactory->addUint8(condition);//	  condition
    //float condition = (float)((harvester->getMaxCondition()-harvester->getDamage())/(harvester->getMaxCondition()/100));
    //mMessageFactory->addFloat((float)2.0);//condition);//	  condition

    //mMessageFactory->addUint64(0);//

    newMessage = mMessageFactory->EndMessage();


    //now add the data to the Baselines header
    Message* completeMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(harvester->getId());
    mMessageFactory->addUint32(opHINO);
    mMessageFactory->addUint8(7);

    mMessageFactory->addUint32(newMessage->getSize());//ByteCount
    mMessageFactory->addData(newMessage->getData(),newMessage->getSize());
    completeMessage = mMessageFactory->EndMessage();


    //Important! -> never leave a message undeleted
    newMessage->setPendingDelete(true);

    (player->getClient())->SendChannelA(completeMessage, player->getAccountId(), CR_Client, 5);
    harvester->setUpdateCounter(harvester->getUpdateCounter()+1);

    return(true);
}


//======================================================================================================================
//
// Building Baselines Type 3
// contain: name,
//

bool MessageLib::sendBaselinesINSO_3(FactoryObject* factory,PlayerObject* player)
{
    if(!(player->isConnected()))
        return(false);

    Message* message;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint16(16);
    mMessageFactory->addFloat(1.0);
    mMessageFactory->addString(factory->getNameFile());
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(factory->getName());

    BString name;
    name = factory->getCustomName();
    name.convert(BSTRType_Unicode16);

    mMessageFactory->addString(name.getUnicode16());

    mMessageFactory->addUint32(1);//volume (in inventory)
    mMessageFactory->addUint16(0);//customization
    mMessageFactory->addUint32(0);//list
    mMessageFactory->addUint32(0);//list

    if(factory->getActive())
        mMessageFactory->addUint8(1);//optionsbitmask - 1 = active
    else
        mMessageFactory->addUint8(0);//optionsbitmask - vendor etc harvester running

    mMessageFactory->addUint8(0);//optionsbitmask - vendor etc harvester running
    mMessageFactory->addUint8(0);//optionsbitmask - vendor etc harvester running
    mMessageFactory->addUint8(0);//optionsbitmask - vendor etc harvester running

    mMessageFactory->addUint32(0);//timer

    //08
    mMessageFactory->addUint32(factory->getDamage());//condition damage

    //09
    mMessageFactory->addUint32(factory->getMaxCondition());   //maxcondition

    //A
    mMessageFactory->addUint8(factory->getActive());//??

    //B
    mMessageFactory->addUint8(factory->getActive());//active flag

    mMessageFactory->addFloat(0);//power reserve
    mMessageFactory->addFloat(0);//power cost
    mMessageFactory->addFloat(0);//
    mMessageFactory->addFloat(0);//




    message = mMessageFactory->EndMessage();

    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(factory->getId());
    mMessageFactory->addUint32(opINSO);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(message->getSize());
    mMessageFactory->addData(message->getData(),message->getSize());

    newMessage = mMessageFactory->EndMessage();
    message->setPendingDelete(true);

    (player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Installation Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesINSO_6(FactoryObject* factory,PlayerObject* player)
{

    if(!(player->isConnected()))
        return(false);

    Message* message;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(16);
    mMessageFactory->addUint64(0);
    mMessageFactory->addUint64(0);
    mMessageFactory->addUint64(0);
    mMessageFactory->addUint64(0);
    mMessageFactory->addUint64(0);


    message = mMessageFactory->EndMessage();

    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(factory->getId());
    mMessageFactory->addUint32(opINSO);
    mMessageFactory->addUint8(6);

    mMessageFactory->addUint32(message->getSize());
    mMessageFactory->addData(message->getData(),message->getSize());

    newMessage = mMessageFactory->EndMessage();
    message->setPendingDelete(true);

    (player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

    return(true);
}

bool MessageLib::sendBaselinesINSO_3(PlayerStructure* structure,PlayerObject* player)
{
    if(!(player->isConnected()))
        return(false);

    Message* message;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint16(16);
    mMessageFactory->addFloat(1.0);
    mMessageFactory->addString(structure->getNameFile());
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(structure->getName());

    BString name;
    name = structure->getCustomName();
    name.convert(BSTRType_Unicode16);

    mMessageFactory->addString(name.getUnicode16());

    mMessageFactory->addUint32(1);//volume (in inventory)
    mMessageFactory->addUint16(0);//customization
    mMessageFactory->addUint32(0);//list
    mMessageFactory->addUint32(0);//list
    mMessageFactory->addUint32(0);//list

    mMessageFactory->addUint32(0);//timer

    //08
    mMessageFactory->addUint32(structure->getDamage());//condition damage

    //09
    mMessageFactory->addUint32(structure->getMaxCondition());   //maxcondition

    //
    mMessageFactory->addUint64(0);//timer
    mMessageFactory->addUint64(0);//timer
    mMessageFactory->addUint64(0);//timer
    mMessageFactory->addUint64(0);//timer
    mMessageFactory->addUint64(0);//timer





    message = mMessageFactory->EndMessage();

    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(structure->getId());
    mMessageFactory->addUint32(opINSO);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(message->getSize());
    mMessageFactory->addData(message->getData(),message->getSize());

    newMessage = mMessageFactory->EndMessage();
    message->setPendingDelete(true);

    (player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Installation Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesINSO_6(PlayerStructure* structure,PlayerObject* player)
{

    if(!(player->isConnected()))
        return(false);

    Message* message;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(16);
    mMessageFactory->addUint64(0);
    mMessageFactory->addUint64(0);
    mMessageFactory->addUint64(0);
    mMessageFactory->addUint64(0);
    mMessageFactory->addUint64(0);


    message = mMessageFactory->EndMessage();

    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(structure->getId());
    mMessageFactory->addUint32(opINSO);
    mMessageFactory->addUint8(6);

    mMessageFactory->addUint32(message->getSize());
    mMessageFactory->addData(message->getData(),message->getSize());

    newMessage = mMessageFactory->EndMessage();
    message->setPendingDelete(true);

    (player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

    return(true);
}



//======================================================================================================================
//send Harvester Name update
//======================================================================================================================

void MessageLib::sendNewHarvesterName(PlayerStructure* harvester)
{
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(harvester->getId());
    mMessageFactory->addUint32(opHINO);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(8 + (harvester->getCustomName().getLength()*2));
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(2);
    //Unicode
    BString name;
    name = harvester->getCustomName();
    name.convert(BSTRType_Unicode16);

    mMessageFactory->addString(name.getUnicode16());

    _sendToInRange(mMessageFactory->EndMessage(),harvester,5);
}

//======================================================================================================================
//send Harvester Name update
//======================================================================================================================

void MessageLib::sendOperateHarvester(PlayerStructure* harvester,PlayerObject* player)
{
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opOperateHarvester);
    mMessageFactory->addUint64(harvester->getId());


    (player->getClient())->SendChannelA(mMessageFactory->EndMessage(), player->getAccountId(), CR_Client, 5);
    //_sendToInRange(mMessageFactory->EndMessage(),harvester,5);
}

//======================================================================================================================
//send Harvester ResourceData
//======================================================================================================================

void MessageLib::sendHarvesterResourceData(PlayerStructure* structure,PlayerObject* player)
{
    HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);
    if(!harvester)
    {
        return;
    }

    ResourceCategory*	mainCat = gResourceManager->getResourceCategoryById(harvester->getResourceCategory());
    ResourceList		resourceList;

    float posX, posZ, ratio;

    posX	= harvester->mPosition.x;
    posZ	= harvester->mPosition.z;

    mainCat->getResources(resourceList,true);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opObjControllerMessage);
    mMessageFactory->addUint32(0x0000000B);
    mMessageFactory->addUint32(opHarvesterResourceData);
    mMessageFactory->addUint64(player->getId());
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint64(harvester->getId());


    ResourceList::iterator resourceIt = resourceList.begin();

    //browse ressource and get only those with more than 0%
    while(resourceIt != resourceList.end() && (*resourceIt) != NULL)
    {
        Resource* tmpResource = (*resourceIt);

        CurrentResource* cR = reinterpret_cast<CurrentResource*>(tmpResource);
        //resource = reinterpret_cast<CurrentResource*>(gResourceManager->getResourceByNameCRC(resourceName.getCrc()));
        if(!cR)
        {
            ratio = 0;
        }
        else
        {
            ratio	= (cR->getDistribution((int)posX + 8192,(int)posZ + 8192)*100);
        }
        //remove less than 1% resources from the list
        if(ratio < 1)
        {
            resourceIt = resourceList.erase(resourceIt);

        }
        else
        {
            ++resourceIt;
        }
    }
    //Send ressource data size
    mMessageFactory->addUint32(resourceList.size());

    //Reloop across resource list cause list size have to be sent before ressource data
    resourceIt = resourceList.begin();
    while (resourceIt != resourceList.end() && (*resourceIt) != NULL)
    {
        Resource* tmpResource = (*resourceIt);
        CurrentResource* cR = reinterpret_cast<CurrentResource*>(tmpResource);
        ratio	= (cR->getDistribution((int)posX + 8192,(int)posZ + 8192)*100);
        if(ratio>100)
        {
            ratio=100;
        }
        mMessageFactory->addUint64(tmpResource->getId());
        mMessageFactory->addString(tmpResource->getName());
        mMessageFactory->addString((tmpResource->getType())->getDescriptor());
        mMessageFactory->addUint8((uint8)ratio);
        ++resourceIt;
    }
    (player->getClient())->SendChannelA(mMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);
}

//======================================================================================================================
//send current Resource Update
//======================================================================================================================

void MessageLib::sendCurrentResourceUpdate(HarvesterObject* harvester, PlayerObject* player)
{
    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(1);	//1 updated var
    mMessageFactory->addUint16(5);	//var Nr 5
    mMessageFactory->addUint64(harvester->getCurrentResource());
    Message* fragment = mMessageFactory->EndMessage();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(harvester->getId());
    mMessageFactory->addUint32(opHINO);
    mMessageFactory->addUint8(7);
    mMessageFactory->addUint32(fragment->getSize());
    mMessageFactory->addData(fragment->getData(),fragment->getSize());

    fragment->setPendingDelete(true);

    (player->getClient())->SendChannelA(mMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);
}

//======================================================================================================================
//send current ExtractionRate update
//======================================================================================================================

void MessageLib::sendCurrentExtractionRate(HarvesterObject* harvester, PlayerObject* player)
{
    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(1);	//1 updated var
    mMessageFactory->addUint16(9);	//var Nr 9
    mMessageFactory->addFloat(harvester->getCurrentExtractionRate());
    Message* fragment = mMessageFactory->EndMessage();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(harvester->getId());
    mMessageFactory->addUint32(opHINO);
    mMessageFactory->addUint8(7);
    mMessageFactory->addUint32(fragment->getSize());
    mMessageFactory->addData(fragment->getData(),fragment->getSize());

    fragment->setPendingDelete(true);

    (player->getClient())->SendChannelA(mMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);
}

//======================================================================================================================
//turn the harvester on
//======================================================================================================================

void MessageLib::sendHarvesterActive(HarvesterObject* harvester)
{
    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(1);	//1 updated var
    mMessageFactory->addUint16(6);	//var Nr 9
    mMessageFactory->addUint8(harvester->getActive());
    Message* fragment = mMessageFactory->EndMessage();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(harvester->getId());
    mMessageFactory->addUint32(opHINO);
    mMessageFactory->addUint8(7);
    mMessageFactory->addUint32(fragment->getSize());
    mMessageFactory->addData(fragment->getData(),fragment->getSize());

    fragment->setPendingDelete(true);

    _sendToInRange(mMessageFactory->EndMessage(),harvester,5);

    //now send the bitmask change to play & stop the animation
    SendUpdateHarvesterWorkAnimation(harvester);

    //(player->getClient())->SendChannelA(mMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);

}


//=======================================================================================================================
//
// sends the relevant delta to the client to update hopper contents
//

void MessageLib::SendHarvesterHopperUpdate(HarvesterObject* harvester, PlayerObject* player)
{

    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(3);	//2 updated vars
    mMessageFactory->addUint16(12);	//var Nr 12 = hopper update flag
    mMessageFactory->addUint8(1);

    mMessageFactory->addUint16(13);	//var Nr 12 = hopper resourcelist

    HResourceList*	hRList = harvester->getResourceList();
    harvester->setRListUpdateCounter(harvester->getRListUpdateCounter() + hRList->size());

    DLOG(INFO) << "adding update Counter  ID " << harvester->getRListUpdateCounter();

    mMessageFactory->addUint32(hRList->size());
    mMessageFactory->addUint32(harvester->getRListUpdateCounter());

    mMessageFactory->addUint8(3);
    mMessageFactory->addUint16(hRList->size());

    HResourceList::iterator it = hRList->begin();

    while(it != hRList->end())
    {
        mMessageFactory->addUint64((*it).first);
        mMessageFactory->addFloat((*it).second);
        //mMessageFactory->addFloat((float)harvester->getRListUpdateCounter());
        it++;
    }

    mMessageFactory->addUint16(10);	//var Nr 12 = hopper update flag
    mMessageFactory->addFloat(harvester->getCurrentHopperSize());

    Message* fragment = mMessageFactory->EndMessage();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(harvester->getId());
    mMessageFactory->addUint32(opHINO);
    mMessageFactory->addUint8(7);
    mMessageFactory->addUint32(fragment->getSize());
    mMessageFactory->addData(fragment->getData(),fragment->getSize());

    fragment->setPendingDelete(true);
    (player->getClient())->SendChannelA(mMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);

}


//======================================================================================================================
//
// sends the Hopper List for a structure
//

bool MessageLib::sendHopperList(PlayerStructure* structure, PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opSendPermissionList);
    mMessageFactory->addUint32(structure->getStrucureHopperList().size() );

    BString name;
    BStringVector vector = 	structure->getStrucureHopperList();
    BStringVector::iterator it = vector.begin();
    while(it != vector.end())
    {
        name = (*it);
        name.convert(BSTRType_Unicode16);
        mMessageFactory->addString(name);

        it++;
    }

    mMessageFactory->addUint32(0); // ???
    //mMessageFactory->addUint16(0);	// unknown
    name = "HOPPER";
    name.convert(BSTRType_Unicode16);
    mMessageFactory->addString(name);
    mMessageFactory->addUint32(0); // ???

    newMessage = mMessageFactory->EndMessage();

    (playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

    structure->resetStructureHopperList();

    return(true);
}


//======================================================================================================================
//
//======================================================================================================================

void MessageLib::sendResourceEmptyHopperResponse(PlayerStructure* structure,PlayerObject* player, uint32 amount, uint8 b1, uint8 b2)
{
    HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);
    if(!harvester)
    {
        return;
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opObjControllerMessage);
    mMessageFactory->addUint32(0x0000000B);
    mMessageFactory->addUint32(opResourceEmptyHopperResponse);
    mMessageFactory->addUint64(player->getId());
    mMessageFactory->addUint32(amount);
    mMessageFactory->addUint32(opResourceEmptyHopper);
    mMessageFactory->addUint8(b1);
    mMessageFactory->addUint8(b2);

    (player->getClient())->SendChannelA(mMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);
}

//=======================================================================================================================
//
// sends the relevant delta to the client to update the working animation
//

void MessageLib::SendUpdateFactoryWorkAnimation(FactoryObject* factory)
{

    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(1);	//2 updated vars
    mMessageFactory->addUint16(6);	//var Nr 6 = bitmask ( harvester work animation)

    if(factory->getActive())
        mMessageFactory->addUint8(1);//optionsbitmask - 1 = active
    else
        mMessageFactory->addUint8(0);//optionsbitmask - vendor etc harvester running

    mMessageFactory->addUint8(0);//optionsbitmask - vendor etc harvester running
    mMessageFactory->addUint16(0);//optionsbitmask - vendor etc harvester running



    Message* fragment = mMessageFactory->EndMessage();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(factory->getId());
    mMessageFactory->addUint32(opINSO);
    mMessageFactory->addUint8(3);
    mMessageFactory->addUint32(fragment->getSize());
    mMessageFactory->addData(fragment->getData(),fragment->getSize());

    fragment->setPendingDelete(true);
    _sendToInRange(mMessageFactory->EndMessage(),factory,5);
    //(player->getClient())->SendChannelA(mMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);

}

//=======================================================================================================================
//
// sends the relevant delta to the client to update the working animation
//

void MessageLib::SendUpdateHarvesterWorkAnimation(HarvesterObject* harvester)
{

    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(1);	//2 updated vars
    mMessageFactory->addUint16(6);	//var Nr 6 = bitmask ( harvester work animation)

    if(harvester->getActive())
        mMessageFactory->addUint8(1);//optionsbitmask - 1 = active
    else
        mMessageFactory->addUint8(0);//optionsbitmask - vendor etc harvester running

    mMessageFactory->addUint8(0);//optionsbitmask - vendor etc harvester running
    mMessageFactory->addUint16(0);//optionsbitmask - vendor etc harvester running



    Message* fragment = mMessageFactory->EndMessage();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(harvester->getId());
    mMessageFactory->addUint32(opHINO);
    mMessageFactory->addUint8(3);
    mMessageFactory->addUint32(fragment->getSize());
    mMessageFactory->addData(fragment->getData(),fragment->getSize());

    fragment->setPendingDelete(true);
    _sendToInRange(mMessageFactory->EndMessage(),harvester,5);
    //(player->getClient())->SendChannelA(mMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);

}

//======================================================================================================================
//send current Resource Update
//======================================================================================================================

void MessageLib::sendHarvesterCurrentConditionUpdate(PlayerStructure* structure)
{
    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(1);	//1 updated var
    mMessageFactory->addUint16(8);	//var Nr 5
    mMessageFactory->addUint32(structure->getDamage());
    Message* fragment = mMessageFactory->EndMessage();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(structure->getId());
    mMessageFactory->addUint32(opHINO);
    mMessageFactory->addUint8(3);
    mMessageFactory->addUint32(fragment->getSize());
    mMessageFactory->addData(fragment->getData(),fragment->getSize());

    fragment->setPendingDelete(true);

    _sendToInRange(mMessageFactory->EndMessage(),structure,5);
}

