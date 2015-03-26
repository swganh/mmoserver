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

#include "ZoneServer/MissionObject.h"

#include "ZoneServer/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/WaypointObject.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"



#include "Common/byte_buffer.h"
#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

using common::ByteBuffer;

//
// Mission Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesMISO_3(MissionObject* missionObject,PlayerObject* targetObject)
{
    if(!missionObject || !targetObject) return false;
    else if(!targetObject->isConnected()) return false;

    Message* message;

    //####
    //HEAD
    //####
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(missionObject->getId());
    mMessageFactory->addUint32(opMISO);
    mMessageFactory->addUint8(3);

    //158
    uint32 size = 158+missionObject->getNameFile().getLength() + missionObject->getName().getLength();
    size += missionObject->getDetailFile().getLength() + missionObject->getDetail().getLength();
    size += missionObject->getTitleFile().getLength() + missionObject->getTitle().getLength();
    size += (missionObject->getWaypoint()->getName().getLength()*2);
    size += missionObject->getCreator().getLength()*2;
    size += missionObject->getTarget().getLength();


    mMessageFactory->addUint32(size); //size
    mMessageFactory->addUint16(17);  //Opperand/Variable count

    //####
    //BODY
    //####

    // 0 -- Complexity
    mMessageFactory->addFloat(1.0f); //4

    // 1 -- Object Name
    mMessageFactory->addString(missionObject->getNameFile()); //STF - ASCII
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(missionObject->getName()); //Name - ASCII +8 =12

    //2 -- Custom name
    mMessageFactory->addUint32(0); //UNICODE string 16

    //3 -- Unknown
    mMessageFactory->addUint32(0);	//20 volume

    //4 -- Unknown
    mMessageFactory->addUint32(0);//24 generic int

    //5 -- Difficulty
    mMessageFactory->addUint32(missionObject->getDifficulty());//28

    //6 -- Destination/End Location
    mMessageFactory->addFloat(missionObject->getDestination().Coordinates.x); //X
    mMessageFactory->addFloat(missionObject->getDestination().Coordinates.y); //Y
    mMessageFactory->addFloat(missionObject->getDestination().Coordinates.z); //Z
    mMessageFactory->addUint64(0); //Possibly a cell id
    mMessageFactory->addUint32(missionObject->getDestination().PlanetCRC); //Planet CRC +24 = 52

    //7 -- Creator
    //mMessageFactory->addUint32(0);
    BString data = missionObject->getCreator();
    data.convert(BSTRType_Unicode16);
    mMessageFactory->addString(data); //UNICODE 56

    //8 -- Reward
    mMessageFactory->addUint32(missionObject->getReward());//60

    //9 -- Start Location
    mMessageFactory->addFloat(missionObject->getStart().Coordinates.x); //X
    mMessageFactory->addFloat(missionObject->getStart().Coordinates.y); //Y
    mMessageFactory->addFloat(missionObject->getStart().Coordinates.z); //Z
    mMessageFactory->addUint64(0); //Possibly a cell id
    mMessageFactory->addUint32(missionObject->getStart().PlanetCRC); //Planet CRC +24 =84

    //10 -- Target Model
    mMessageFactory->addUint32(missionObject->getTargetModel());//88

    //11 -- Description
    mMessageFactory->addString(missionObject->getDetailFile());  //STF - ASCII
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(missionObject->getDetail()); //Name - ASCII +8 =96

    //12 -- Title
    mMessageFactory->addString(missionObject->getTitleFile()); //STF - ASCII
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(missionObject->getTitle()); //Name - ASCII +8 = 104

    //13 -- Refresh Count
    mMessageFactory->addUint32(missionObject->getRefreshCount());//108

    //14 -- Mission Type
    mMessageFactory->addUint32(missionObject->getMissionType());//112

    //15 -- Target Name
    //mMessageFactory->addUint16(0);
    mMessageFactory->addString(missionObject->getTarget()); // "@STF:Name" format - ASCII//114

    //16 -- Mission Waypoint
    mMessageFactory->addUint32(0);
    mMessageFactory->addFloat(missionObject->getWaypoint()->getCoords().x);  //X
    mMessageFactory->addFloat(missionObject->getWaypoint()->getCoords().y);  //Y
    mMessageFactory->addFloat(missionObject->getWaypoint()->getCoords().z);  //Z
    mMessageFactory->addUint64(0); //Possible Cell ID
    mMessageFactory->addUint32(missionObject->getWaypoint()->getPlanetCRC()); //Planet CRC
    data = missionObject->getWaypoint()->getName();
    data.convert(BSTRType_Unicode16);
    mMessageFactory->addString(data); //Waypoint name "@STF:Name" format - UNICODE
    mMessageFactory->addUint64(missionObject->getWaypoint()->getId()); //Waypoint ID (MISO3 ID +1)
    mMessageFactory->addUint8(missionObject->getWaypoint()->getWPType());  //Waypoint Type
    mMessageFactory->addUint8(missionObject->getWaypoint()->getActive());  //Activated Flag +42 =156
    mMessageFactory->addUint32(0);

    message = mMessageFactory->EndMessage();

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return true;
}

//======================================================================================================================
//
// Mission Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesMISO_6(MissionObject* missionObject,PlayerObject* targetObject)
{
    if(!missionObject || !targetObject) return false;
    else if(!targetObject->isConnected()) return false;

    Message* message;

    //####
    //HEAD
    //####
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(missionObject->getId());
    mMessageFactory->addUint32(opMISO);
    mMessageFactory->addUint8(6);

    mMessageFactory->addUint32(15); //size
    mMessageFactory->addUint16(2);  //Opperand/Variable count

    //####
    //BODY
    //####
    mMessageFactory->addUint64(0x86);
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint8(0);

    message = mMessageFactory->EndMessage();

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return true;
}

//======================================================================================================================
//
// Mission Baselines Type 8
// contain: unknown
//

bool MessageLib::sendBaselinesMISO_8(MissionObject* missionObject,PlayerObject* targetObject)
{
    if(!missionObject || !targetObject) return false;
    else if(!targetObject->isConnected()) return false;

    Message* message;

    //####
    //HEAD
    //####
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(missionObject->getId());
    mMessageFactory->addUint32(opMISO);
    mMessageFactory->addUint8(8);
    mMessageFactory->addUint32(2);
    mMessageFactory->addUint16(0);  //Opperand count


    //####
    //BODY
    //####
    mMessageFactory->addUint32(0);

    message = mMessageFactory->EndMessage();

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return true;
}

//======================================================================================================================
//
// Mission Baselines Type 9
// contain: unknown
//

bool MessageLib::sendBaselinesMISO_9(MissionObject* missionObject,PlayerObject* targetObject)
{
    if(!missionObject || !targetObject) return false;
    else if(!targetObject->isConnected()) return false;

    Message* message;

    //####
    //HEAD
    //####
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(missionObject->getId());
    mMessageFactory->addUint32(opMISO);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(2);  //Opperand count


    //####
    //BODY
    //####
    mMessageFactory->addUint16(0);

    message = mMessageFactory->EndMessage();

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return true;
}

//======================================================================================================================

bool MessageLib::sendMISO_Delta(MissionObject* missionObject,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))	return(false);
    if(!missionObject || !targetObject) return false;

    int update_count = 0;
    ByteBuffer body;

    //NOTE: Do not mess with the order these are in. Being mildly AR I
    //      arranged them in order according to vID and it stopped working!

    //NOTE
    // use messages as bytebuffers!

    //vID 1 - Name
    if(strcmp(missionObject->getNameFile().getRawData(),"mission/mission_object"))
    {
        update_count++;
        body.write<uint16_t>(0x01);
        body.write<std::string>(missionObject->getNameFile().getRawData());
        body.write<uint32_t>(0);
        body.write<std::string>(missionObject->getName().getRawData());
    }

    //vID 11 - Desciption
    if(missionObject->getDetail().getDataLength() > 0)
    {
        update_count++;
        body.write<uint16_t>(0x0B);
        body.write<std::string>(missionObject->getDetailFile().getRawData());
        body.write<uint32_t>(0);
        body.write<std::string>(missionObject->getDetail().getRawData());
    }

    //vID 5 - Difficulty
    if(missionObject->getDifficulty() > 0)
    {
        update_count++;
        body.write<uint16_t>(0x05);
        body.write<uint32_t>(missionObject->getDifficulty());
    }

    //vID 6 - mission start
    if(missionObject->getStart().PlanetCRC > 0)
    {
        update_count++;
        body.write<uint16_t>(0x06);
        body.write<float>(missionObject->getStart().Coordinates.x);
        body.write<float>(missionObject->getStart().Coordinates.y);
        body.write<float>(missionObject->getStart().Coordinates.z);
        body.write<uint64_t>(missionObject->getStart().CellID);
        body.write<uint32_t>(missionObject->getStart().PlanetCRC);
    }

    //vID 7 - Creator
    if(missionObject->getCreator().getDataLength())
    {
        update_count++;
        body.write<uint16_t>(0x07);
        BString ha(missionObject->getCreator());
        ha.convert(BSTRType_Unicode16);
        body.write<std::wstring>(ha.getUnicode16());
    }

    //vID 14 - Mission Type
    if(missionObject->getMissionType() > 0)
    {
        update_count++;
        body.write<uint16_t>(0x0E);
        body.write<uint32_t>(missionObject->getMissionType());
    }

    //vID 8	- payment
    if(missionObject->getReward() > 0)
    {
        update_count++;
        body.write<uint16_t>(0x08);
        body.write<uint32_t>(missionObject->getReward());
    }

    //vID 9 - mission location
    if(missionObject->getDestination().PlanetCRC > 0)
    {
        update_count++;
        body.write<uint16_t>(0x09);
        body.write<float>(missionObject->getDestination().Coordinates.x);
        body.write<float>(missionObject->getDestination().Coordinates.y);
        body.write<float>(missionObject->getDestination().Coordinates.z);
        body.write<uint64_t>(missionObject->getDestination().CellID);
        body.write<uint32_t>(missionObject->getDestination().PlanetCRC);
    }

    //vID 10 - Target Model
    if(missionObject->getTargetModel() > 0)
    {
        update_count++;
        body.write<uint16_t>(0x0A);
        body.write<uint32_t>(missionObject->getTargetModel());
    }

    //vID 12 - Title
    if(missionObject->getTitle().getDataLength() > 0)
    {
        update_count++;
        body.write<uint16_t>(0x0C);
        body.write<std::string>(missionObject->getTitleFile().getRawData());
        body.write<uint32_t>(0);
        body.write<std::string>(missionObject->getTitle().getRawData());
    }

    //vID 13 - Refresh Count
    if(missionObject->getRefreshCount() > 0)
    {
        update_count++;
        body.write<uint16_t>(0x0D);
        body.write<uint32_t>(missionObject->getRefreshCount()); //The count given by the terminal
    }

    //vID 15 - Target Name
    if(missionObject->getTarget().getDataLength() > 0)
    {
        update_count++;
        body.write<uint16_t>(0x0F);
        body.write<std::string>(missionObject->getTarget().getRawData());
    }

    //vID 16 - Mission Waypoint
    if(missionObject->getWaypoint()->getPlanetCRC() > 0)
    {
        update_count++;
        body.write<uint16_t>(0x10);
        body.write<uint32_t>(0);
        body.write<float>(missionObject->getWaypoint()->getCoords().x);  //X
        body.write<float>(missionObject->getWaypoint()->getCoords().y);  //Y
        body.write<float>(missionObject->getWaypoint()->getCoords().z);  //Z
        body.write<uint64_t>(0); //Possible Cell ID
        body.write<uint32_t>(missionObject->getWaypoint()->getPlanetCRC()); //Planet CRC
        //
        BString ha(missionObject->getWaypoint()->getName());
        ha.convert(BSTRType_Unicode16);
        body.write<std::wstring>(ha.getUnicode16());
        body.write<uint64_t>(missionObject->getWaypoint()->getId());	 //waypoint id
        body.write<uint8_t>(missionObject->getWaypoint()->getWPType());   //waypoint type
        body.write<uint8_t>(missionObject->getWaypoint()->getActive());	 //activated flag
    }

    //Nothing to update
    if(body.size() <= 0)
    {
        return false;
    }

    //####
    //HEAD
    //####
    Message* message;
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(missionObject->getId());
    mMessageFactory->addUint32(opMISO);
    mMessageFactory->addUint8(3);
    mMessageFactory->addUint32(body.size()+2);
    mMessageFactory->addUint16(update_count);
    mMessageFactory->addData(body.data(),(uint16)body.size());


    message = mMessageFactory->EndMessage();
    //gLogger->hexDump(message->getData(),message->getSize());

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return true;
}

//======================================================================================================================

bool MessageLib::sendMissionGenericResponse(MissionObject* missionObject,PlayerObject* targetObject)
{
    if(!missionObject || !targetObject) return false;
    else if(!targetObject->isConnected()) return false;

    Message* message;
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opObjControllerMessage);
    mMessageFactory->addUint32(0x0000001B);
    mMessageFactory->addUint32(opMissionGenericResponse);
    mMessageFactory->addUint64(missionObject->getOwner()->getId());
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint64(missionObject->getId());
    mMessageFactory->addUint8(0x01);
    mMessageFactory->addInt8(0x04);

    message = mMessageFactory->EndMessage();

    targetObject->getClient()->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return true;
}

//======================================================================================================================

bool MessageLib::sendMissionAbort(MissionObject* missionObject,PlayerObject* targetObject)
{
    if(!missionObject || !targetObject) return false;
    else if(!targetObject->isConnected()) return false;

    Message* message;
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opObjControllerMessage);
    mMessageFactory->addUint32(0x0000000B);
    mMessageFactory->addUint32(opMissionAbort);
    mMessageFactory->addUint64(missionObject->getOwner()->getPlayerObjId());
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint64(missionObject->getId());

    message = mMessageFactory->EndMessage();

    targetObject->getClient()->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return true;
}
//======================================================================================================================

bool MessageLib::sendMissionComplete(PlayerObject* targetObject)
{

    if(!targetObject) return false;
    else if(!targetObject->isConnected()) return false;

    Message* message;
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opObjControllerMessage);
    mMessageFactory->addUint32(0x00000083);
    mMessageFactory->addUint32(opMissionComplete);
    mMessageFactory->addUint64(targetObject->getPlayerObjId());
    mMessageFactory->addUint64(0);
    mMessageFactory->addUint16(0);
    mMessageFactory->addUint8(0);

    message = mMessageFactory->EndMessage();

    targetObject->getClient()->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return true;
}


//======================================================================================================================


