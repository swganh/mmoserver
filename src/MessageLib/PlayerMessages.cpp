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

#include <boost/lexical_cast.hpp>


#ifdef ERROR
#undef ERROR
#endif

#include "Utils/logger.h"

#include "Common/atMacroString.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

#include "ZoneServer/CharSheetManager.h"
#include "ZoneServer/Conversation.h"
#include "ZoneServer/CraftingTool.h"
#include "ZoneServer/CurrentResource.h"
#include "ZoneServer/Datapad.h"
#include "ZoneServer/ManufacturingSchematic.h"
#include "ZoneServer/NPCObject.h"
#include "ZoneServer/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/UIOpcodes.h"
#include "ZoneServer/WaypointObject.h"
#include "ZoneServer/Wearable.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"


//======================================================================================================================
//
// Player Baselines Type 3
// contain: title, flags(afk,..), matchmaking, borndate, playtime
//

bool MessageLib::sendBaselinesPLAY_3(PlayerObject* playerObject,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(95 + playerObject->getTitle().getLength());

    mMessageFactory->addUint16(0x000b); // unknown
    mMessageFactory->addUint32(0x3f800000); // complexity
    mMessageFactory->addString(BString("string_id_table"));//stf file
    mMessageFactory->addUint32(0); // unknown
    mMessageFactory->addUint16(0); // stf name

    mMessageFactory->addUint32(0); // custom name
    mMessageFactory->addUint32(0); // volume
    mMessageFactory->addUint32(0); // generic int (not used)

    // player flags
    mMessageFactory->addUint32(4); //flaglist size
    mMessageFactory->addUint32(playerObject->getPlayerFlags());
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);

    // profile - matchmaking
    mMessageFactory->addUint32(4); // flaglist size
    mMessageFactory->addUint32(playerObject->getPlayerMatch(0));
    mMessageFactory->addUint32(playerObject->getPlayerMatch(1));
    mMessageFactory->addUint32(playerObject->getPlayerMatch(2));
    mMessageFactory->addUint32(playerObject->getPlayerMatch(3));

    mMessageFactory->addString(playerObject->getTitle());

    // birthdate + totalplaytime
    mMessageFactory->addUint32(playerObject->getBornyear()); // Born year
    mMessageFactory->addUint32(0x000018d8); // Total Playtime in seconds
    mMessageFactory->addUint32(0);

    (targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Player Baselines Type 6
// contain: subzone region_id, csr / developer tag
//

bool MessageLib::sendBaselinesPLAY_6(PlayerObject* playerObject,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(6);

    mMessageFactory->addUint32(7);
    mMessageFactory->addUint16(2);

    mMessageFactory->addUint32(playerObject->getSubZoneId());
    mMessageFactory->addUint8(playerObject->getCsrTag());

    (targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Player Baselines Type 8
// contain: xp, waypoints, forcebar, quests
//

bool MessageLib::sendBaselinesPLAY_8(PlayerObject* playerObject,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    XPList* xpList = playerObject->getXpList();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(8);

    // xp list size
    uint32 xpByteCount = 0;
    XPList::iterator xpIt = xpList->begin();

    uint32 xpListSize = 0;
    while(xpIt != xpList->end())
    {
        //if ((*xpIt).second > 0)	// Only add xptypes that we actually have any xp from.
        //{
        xpByteCount += ((gSkillManager->getXPTypeById((*xpIt).first)).getLength() + 7); // strlen + value + delimiter
        xpListSize++;
        //}
        ++xpIt;
    }

    // waypoint list size
    uint32					waypointsByteCount	= 0;
    Datapad* datapad							= playerObject->getDataPad();
    WaypointList*			waypointList;
    WaypointList::iterator	waypointIt;
    if(datapad) {
        waypointList		= datapad->getWaypoints();
        waypointIt			= waypointList->begin();
    } else { //Crashbug patch: http://paste.swganh.org/viewp.php?id=20100627075254-3882bd68067f13266819ae6d0c4428e4
        LOG(WARNING) << "MessageLib::sendBaselinesPLAY_8: Failed to find datapad for playerId: " <<  playerObject->getId() << ". Did not initialize waypList(s).";
        gWorldManager->addDisconnectedPlayer(playerObject);

        Message* message = mMessageFactory->EndMessage();
        message->setPendingDelete(true);
        return false;
    }

    while(waypointIt != waypointList->end())
    {
        waypointsByteCount += (51 + ((*waypointIt)->getName().getLength() *2));
        ++waypointIt;
    }

    mMessageFactory->addUint32(76 + xpByteCount + waypointsByteCount);
    mMessageFactory->addUint16(7);

    // xp list
    // mMessageFactory->addUint32(xpList->size());
    mMessageFactory->addUint32(xpListSize);
    mMessageFactory->addUint32(playerObject->mXpUpdateCounter);

    xpIt = xpList->begin();

    while(xpIt != xpList->end())
    {
        //if ((*xpIt).second > 0)	// Only add xptypes that we actually have xp from.
        //{
        mMessageFactory->addUint8(0);
        mMessageFactory->addString(gSkillManager->getXPTypeById((*xpIt).first));
        mMessageFactory->addInt32((*xpIt).second);
        //}
        ++xpIt;
    }

    // waypoint list
    mMessageFactory->addUint32(waypointList->size());
    mMessageFactory->addUint32(datapad->mWaypointUpdateCounter);

    waypointIt = waypointList->begin();


    while(waypointIt != waypointList->end())
    {
        WaypointObject* waypoint = (*waypointIt);

        mMessageFactory->addUint8(2);
        mMessageFactory->addUint64(waypoint->getId());
        mMessageFactory->addUint32(0);
        mMessageFactory->addFloat(waypoint->getCoords().x);
        mMessageFactory->addFloat(waypoint->getCoords().y);
        mMessageFactory->addFloat(waypoint->getCoords().z);
        mMessageFactory->addUint64(0);
        mMessageFactory->addUint32(waypoint->getPlanetCRC());//planetcrc
        mMessageFactory->addString(waypoint->getName());
        mMessageFactory->addUint64(waypoint->getId());
        mMessageFactory->addUint8(waypoint->getWPType());
        if(waypoint->getActive())
            mMessageFactory->addUint8(1);
        else
            mMessageFactory->addUint8(0);

        ++waypointIt;
    }

    // current force
    mMessageFactory->addUint32(playerObject->getHam()->getCurrentForce());

    // max force
    mMessageFactory->addUint32(playerObject->getHam()->getMaxForce());

    // unknown
    mMessageFactory->addUint32(5);
    mMessageFactory->addUint64(20);
    mMessageFactory->addUint8(0);
    mMessageFactory->addUint32(5);
    mMessageFactory->addUint64(14);
    mMessageFactory->addUint8(0);

    // quest list
    mMessageFactory->addUint64(0);

    mMessageFactory->addUint32(0);
    mMessageFactory->addUint64(0);
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint16(0);

    (targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Player Baselines Type 9
// contain: skillcommands, crafting info, draftschematics, stomach, language, friend/ignore lists
//

bool MessageLib::sendBaselinesPLAY_9(PlayerObject* playerObject,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    SkillCommandList*		scList		= playerObject->getSkillCommands();
    SchematicsIdList*		schemIdList = playerObject->getSchematicsIdList();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(9);

    // get skillcmds bytecount
    uint32 skillCommandsByteCount = 0;
    SkillCommandList::iterator scIt = scList->begin();

    while(scIt != scList->end())
    {
        skillCommandsByteCount += ((gSkillManager->getSkillCmdById(*scIt)).getLength() + 2);
        ++scIt;
    }

    // get schematics bytecount
    uint32 schematicsByteCount = schemIdList->size() * 8;

    mMessageFactory->addUint32(98 + skillCommandsByteCount + schematicsByteCount);
    mMessageFactory->addUint16(19);

    // skill commands
    mMessageFactory->addUint32(scList->size());
    mMessageFactory->addUint32(playerObject->mSkillCmdUpdateCounter);

    scIt = scList->begin();

    while(scIt != scList->end())
    {
        mMessageFactory->addString(gSkillManager->getSkillCmdById(*scIt));
        ++scIt;
    }

    // crafting experimentation
    mMessageFactory->addUint32(0);

    // crafting stage
    mMessageFactory->addUint32(0);

    // nearest crafting station
    mMessageFactory->addUint64(0);

    // draft schematics


    Datapad* datapad			= playerObject->getDataPad();

    mMessageFactory->addUint32(schemIdList->size());

    datapad->mSchematicUpdateCounter += schemIdList->size();
    mMessageFactory->addUint32(datapad->mSchematicUpdateCounter);

    SchematicsIdList::iterator schemIt = schemIdList->begin();

    while(schemIt != schemIdList->end())
    {
        mMessageFactory->addUint64(*schemIt);
        ++schemIt;
    }


    // experimentation points
    mMessageFactory->addUint32(0);

    // accomplishment counter
    mMessageFactory->addUint32(0);


    // contact lists: these are always 0 here, will get updated by seperate deltas messages
    // after object initialization is complete

    // friends list
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);
    // ignores list
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);


    // current language
    mMessageFactory->addUint32(playerObject->getLanguage());
    // 1 = basic

    // stomach
    Stomach* stomach = playerObject->getStomach();

    mMessageFactory->addUint32(stomach->getFood());
    mMessageFactory->addUint32(stomach->getFoodMax());
    mMessageFactory->addUint32(stomach->getDrink());
    mMessageFactory->addUint32(stomach->getDrinkMax());

    // consumeables
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);

    // unused waypoints list
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);

    // jedi state
    mMessageFactory->addUint32(playerObject->getJediState());

    (targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: Stomach
//

void MessageLib::sendFoodUpdate(PlayerObject* playerObject)
{
    // stomach
    Stomach* stomach = playerObject->getStomach();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(8);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(0x0a);
    mMessageFactory->addUint32(stomach->getFood());

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: Drink
//

void MessageLib::sendDrinkUpdate(PlayerObject* playerObject)
{
    // stomach
    Stomach* stomach = playerObject->getStomach();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(8);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(0x0c);
    mMessageFactory->addUint32(stomach->getDrink());

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);
}

//======================================================================================================================
//
// Player Deltas Type 3
// update: title
//

void MessageLib::sendTitleUpdate(PlayerObject* playerObject)
{
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(6 + playerObject->getTitle().getLength());
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(7);
    mMessageFactory->addString(playerObject->getTitle());

    _sendToInRange(mMessageFactory->EndMessage(),playerObject,5);
}

//======================================================================================================================
//
// Player Deltas Type 3
// update: flags
//

void MessageLib::sendUpdatePlayerFlags(PlayerObject* playerObject)
{
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(24);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(5);

    mMessageFactory->addUint32(4);
    mMessageFactory->addUint32(playerObject->getPlayerFlags());
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);

    _sendToInRange(mMessageFactory->EndMessage(),playerObject,5);
}

//======================================================================================================================
//
// Player Deltas Type 8
// update: waypoints
//

bool MessageLib::sendWaypointsUpdate(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    Datapad* datapad			= playerObject->getDataPad();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(8);

    uint32					waypointsByteCount	= 0;
    WaypointList*			waypointList		= datapad->getWaypoints();
    WaypointList::iterator	waypointIt			= waypointList->begin();

    while(waypointIt != waypointList->end())
    {
        waypointsByteCount += (51 + ((*waypointIt)->getName().getLength() << 1));
        ++waypointIt;
    }

    mMessageFactory->addUint32(12 + waypointsByteCount);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(1);

    // waypoint list
    mMessageFactory->addUint32(waypointList->size());

    datapad->mWaypointUpdateCounter += waypointList->size();
    mMessageFactory->addUint32(datapad->mWaypointUpdateCounter);

    waypointIt = waypointList->begin();

    while(waypointIt != waypointList->end())
    {
        WaypointObject* waypoint = (*waypointIt);

        mMessageFactory->addUint8(0);
        mMessageFactory->addUint64(waypoint->getId());
        mMessageFactory->addUint32(0);
        mMessageFactory->addFloat(waypoint->getCoords().x);
        mMessageFactory->addFloat(waypoint->getCoords().y);
        mMessageFactory->addFloat(waypoint->getCoords().z);
        mMessageFactory->addUint64(0);
        mMessageFactory->addUint32(waypoint->getModelString().getCrc());
        mMessageFactory->addString(waypoint->getName());
        mMessageFactory->addUint64(waypoint->getId());
        mMessageFactory->addUint8(waypoint->getWPType());
        mMessageFactory->addUint8((uint8)waypoint->getActive());

        ++waypointIt;
    }

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 8
// update: waypoints
//

bool MessageLib::sendUpdateWaypoint(WaypointObject* waypoint,ObjectUpdate updateType,PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    uint8 type = 0xFF;
    if(updateType == ObjectUpdateAdd)
        type = 0;

    if(updateType == ObjectUpdateDelete)
        type = 1;

    if(updateType == ObjectUpdateChange)
        type = 2;

    if(type == 0xFF)
        return false;

    Datapad* datapad			= playerObject->getDataPad();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(8);


    mMessageFactory->addUint32(63 + (waypoint->getName().getLength() << 1));
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(1);

    // elements
    mMessageFactory->addUint32(1);
    mMessageFactory->addUint32(++datapad->mWaypointUpdateCounter);

    mMessageFactory->addUint8(type);
    mMessageFactory->addUint64(waypoint->getId());
    mMessageFactory->addUint32(0);
    mMessageFactory->addFloat(waypoint->getCoords().x);
    mMessageFactory->addFloat(waypoint->getCoords().y);
    mMessageFactory->addFloat(waypoint->getCoords().z);
    mMessageFactory->addUint64(0);
    mMessageFactory->addUint32(waypoint->getModelString().getCrc());
    mMessageFactory->addString(waypoint->getName());
    mMessageFactory->addUint64(waypoint->getId());
    mMessageFactory->addUint8(waypoint->getWPType());
    mMessageFactory->addUint8((uint8)waypoint->getActive());

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: skill commands
//

bool MessageLib::sendSkillCmdDeltasPLAY_9(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    // get skillcmds bytecount
    SkillCommandList*		scList					= playerObject->getSkillCommands();
    uint32					skillCommandsByteCount	= 0;
    SkillCommandList::iterator scIt					= scList->begin();

    while(scIt != scList->end())
    {
        skillCommandsByteCount += ((gSkillManager->getSkillCmdById(*scIt)).getLength() + 2);
        ++scIt;
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(15 + skillCommandsByteCount);
    mMessageFactory->addUint16(1);

    mMessageFactory->addUint16(0);
    mMessageFactory->addUint32(scList->size() + 1);

    // playerObject->mSkillCmdUpdateCounter += 1;
    playerObject->mSkillCmdUpdateCounter += scList->size() + 1;
    mMessageFactory->addUint32(playerObject->mSkillCmdUpdateCounter);

    mMessageFactory->addUint8(3);	   //3 for rewrite
    mMessageFactory->addUint16(scList->size());

    scIt = scList->begin();

    while(scIt != scList->end())
    {
        mMessageFactory->addString(gSkillManager->getSkillCmdById(*scIt));
        ++scIt;
    }

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: schematics
//

bool MessageLib::sendSchematicDeltasPLAY_9(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    SchematicsIdList*			sList		= playerObject->getSchematicsIdList();
    SchematicsIdList::iterator	sIt			= sList->begin();
    Datapad* datapad						= playerObject->getDataPad();

    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(1);

    mMessageFactory->addUint16(4);

    mMessageFactory->addUint32(sList->size()+1);

    datapad->mSchematicUpdateCounter += sList->size()+1;

    mMessageFactory->addUint32(datapad->mSchematicUpdateCounter);

    mMessageFactory->addUint8(3);	 //this looks a little strange to me
    // basically I think we are resetting the list (3 = reset all 0 = new list size)
    // and then adding all the entries from scratch
    // however it works nicely :)

    mMessageFactory->addUint16(0);
    uint16 count = 1;

    sIt = sList->begin();

    while(sIt != sList->end())
    {
        mMessageFactory->addUint8(1);
        mMessageFactory->addUint16(count);
        mMessageFactory->addUint64((*sIt));
        count += 1;
        ++sIt;
    }

    Message* data = mMessageFactory->EndMessage();



    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(data->getSize());
    mMessageFactory->addData(data->getData(),data->getSize());

    data->setPendingDelete(true);

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}



//======================================================================================================================
//
// Player Deltas Type 8
// update: xp
//

bool MessageLib::sendXpUpdate(uint32 xpType,PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    // We must be able to set the value of 0, when we use up all xp.

    BString		xpTypeName = gSkillManager->getXPTypeById(xpType);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(8);

    mMessageFactory->addUint32(19 + xpTypeName.getLength());

    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(0);

    mMessageFactory->addUint32(1);
    mMessageFactory->addUint32(++playerObject->mXpUpdateCounter);

    mMessageFactory->addUint8(2);
    mMessageFactory->addString(xpTypeName);
    mMessageFactory->addInt32(playerObject->getXpAmount(xpType));

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 8
// update: xp types
//

bool MessageLib::sendUpdateXpTypes(SkillXpTypesList newXpTypes,uint8 remove,PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    uint32		xpByteCount			= 0;
    SkillXpTypesList::iterator it	= newXpTypes.begin();

    while(it != newXpTypes.end())
    {
        xpByteCount += 7 + gSkillManager->getXPTypeById(*it).getLength();
        ++it;
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(8);

    mMessageFactory->addUint32(12 + xpByteCount);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(0);

    mMessageFactory->addUint32(newXpTypes.size());

    playerObject->mXpUpdateCounter += newXpTypes.size();
    mMessageFactory->addUint32(playerObject->mXpUpdateCounter);

    it = newXpTypes.begin();

    while(it != newXpTypes.end())
    {
        mMessageFactory->addUint8(remove);
        mMessageFactory->addString(gSkillManager->getXPTypeById(*it));
        mMessageFactory->addInt32(0);

        ++it;
    }

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: friend list
//

bool MessageLib::sendFriendListPlay9(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    ContactMap*				friendList	= playerObject->getFriendsList();
    ContactMap::iterator	nameIt		= friendList->begin();

    uint32 friendListByteCount = 0;

    while(nameIt != friendList->end())
    {
        friendListByteCount += ((*nameIt).second.getLength() + 2);
        ++nameIt;
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(15 + friendListByteCount);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(7);


    playerObject->advanceFriendsListUpdateCounter(friendList->size() + 1);

    mMessageFactory->addUint32(friendList->size() + 1);
    mMessageFactory->addUint32(playerObject->getFriendsListUpdateCounter());

    mMessageFactory->addUint8(3);
    mMessageFactory->addUint16(friendList->size());

    nameIt = friendList->begin();

    while(nameIt != friendList->end())
    {
        mMessageFactory->addString((*nameIt).second);
        ++nameIt;
    }

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: ignore list
//

bool MessageLib::sendIgnoreListPlay9(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    ContactMap*				ignoreList	= playerObject->getIgnoreList();
    ContactMap::iterator	nameIt		= ignoreList->begin();

    uint32 ignoreListByteCount = 0;

    while(nameIt != ignoreList->end())
    {
        ignoreListByteCount += (((*nameIt).second).getLength() + 2);
        ++nameIt;
    }

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(15 + ignoreListByteCount);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(8);

    playerObject->advanceIgnoresListUpdateCounter(ignoreList->size() + 1);

    mMessageFactory->addUint32(ignoreList->size() + 1);
    mMessageFactory->addUint32(playerObject->getIgnoresListUpdateCounter());

    mMessageFactory->addUint8(3);
    mMessageFactory->addUint16(ignoreList->size());

    nameIt = ignoreList->begin();

    while(nameIt != ignoreList->end())
    {
        mMessageFactory->addString((*nameIt).second);
        ++nameIt;
    }

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 3
// update: matchmaking
//

bool MessageLib::sendMatchPlay3(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(24);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(6);

    mMessageFactory->addUint32(4); // unknown
    mMessageFactory->addUint32(playerObject->getPlayerMatch(0));
    mMessageFactory->addUint32(playerObject->getPlayerMatch(1));
    mMessageFactory->addUint32(playerObject->getPlayerMatch(2));
    mMessageFactory->addUint32(playerObject->getPlayerMatch(3));

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: crafting stage
//

bool MessageLib::sendUpdateCraftingStage(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(8);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(2);
    mMessageFactory->addUint32(playerObject->getCraftingStage());

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: experimentation flag
//

bool MessageLib::sendUpdateExperimentationFlag(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(8);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint32(playerObject->getExperimentationFlag());

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: crafting stage
//

bool MessageLib::sendUpdateExperimentationPoints(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(8);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(5);
    mMessageFactory->addUint32(playerObject->getExperimentationPoints());

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: nearest station
//

bool MessageLib::sendUpdateNearestCraftingStation(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(12);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(3);
    mMessageFactory->addUint64(playerObject->getNearestCraftingStation());

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: language
//

void MessageLib::sendLanguagePlay9(PlayerObject* playerObject)
{
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(8);
    mMessageFactory->addUint16(1);//one var updated
    mMessageFactory->addUint16(9);//var nr 9 updated

    mMessageFactory->addUint32(playerObject->getLanguage());

    _sendToInRange(mMessageFactory->EndMessage(),playerObject,5);
}

//======================================================================================================================
//
// Player Deltas Type 8
// update: current force
//

bool MessageLib::sendUpdateCurrentForce(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(8);

    mMessageFactory->addUint32(8);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(2);
    mMessageFactory->addUint32(playerObject->getHam()->getCurrentForce());

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================
//
// Player Deltas Type 8
// update: max force
//

bool MessageLib::sendUpdateMaxForce(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(playerObject->getPlayerObjId());
    mMessageFactory->addUint32(opPLAY);
    mMessageFactory->addUint8(8);

    mMessageFactory->addUint32(8);
    mMessageFactory->addUint16(1);
    mMessageFactory->addUint16(3);
    mMessageFactory->addUint32(playerObject->getHam()->getMaxForce());

    (playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

    return(true);
}

//======================================================================================================================


