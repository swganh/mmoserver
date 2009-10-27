/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

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

#include "LogManager/LogManager.h"

#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"



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
	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(missionObject->getId()); 
	gMessageFactory->addUint32(opMISO);
	gMessageFactory->addUint8(3);  
	
	//158
	uint32 size = 158+missionObject->getNameFile().getLength() + missionObject->getName().getLength();
	size += missionObject->getDetailFile().getLength() + missionObject->getDetail().getLength();
	size += missionObject->getTitleFile().getLength() + missionObject->getTitle().getLength();
	size += (missionObject->getWaypoint()->getName().getLength()*2);
	size += missionObject->getCreator().getLength()*2;
	size += missionObject->getTarget().getLength();


	gMessageFactory->addUint32(size); //size 
	gMessageFactory->addUint16(17);  //Opperand/Variable count

	//####
	//BODY
	//####

	// 0 -- Complexity
	gMessageFactory->addFloat(1.0f); //4

    // 1 -- Object Name
	gMessageFactory->addString(missionObject->getNameFile()); //STF - ASCII
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(missionObject->getName()); //Name - ASCII +8 =12

	//2 -- Custom name
	gMessageFactory->addUint32(0); //UNICODE string 16

	//3 -- Unknown
	gMessageFactory->addUint32(0);	//20 volume

	//4 -- Unknown
	gMessageFactory->addUint32(0);//24 generic int

	//5 -- Difficulty
	gMessageFactory->addUint32(missionObject->getDifficulty());//28

	//6 -- Destination/End Location
	gMessageFactory->addFloat(missionObject->getDestination().Coordinates.mX); //X
	gMessageFactory->addFloat(missionObject->getDestination().Coordinates.mY); //Y
	gMessageFactory->addFloat(missionObject->getDestination().Coordinates.mZ); //Z
	gMessageFactory->addUint64(0); //Possibly a cell id
	gMessageFactory->addUint32(missionObject->getDestination().PlanetCRC); //Planet CRC +24 = 52

	//7 -- Creator
	//gMessageFactory->addUint32(0);
	string data = missionObject->getCreator();
	data.convert(BSTRType_Unicode16);
	gMessageFactory->addString(data); //UNICODE 56

	//8 -- Reward
	gMessageFactory->addUint32(missionObject->getReward());//60

	//9 -- Start Location
	gMessageFactory->addFloat(missionObject->getStart().Coordinates.mX); //X
	gMessageFactory->addFloat(missionObject->getStart().Coordinates.mY); //Y
	gMessageFactory->addFloat(missionObject->getStart().Coordinates.mZ); //Z
	gMessageFactory->addUint64(0); //Possibly a cell id
	gMessageFactory->addUint32(missionObject->getStart().PlanetCRC); //Planet CRC +24 =84

	//10 -- Target Model
	gMessageFactory->addUint32(missionObject->getTargetModel());//88

	//11 -- Description
	gMessageFactory->addString(missionObject->getDetailFile());  //STF - ASCII
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(missionObject->getDetail()); //Name - ASCII +8 =96

	//12 -- Title
	gMessageFactory->addString(missionObject->getTitleFile()); //STF - ASCII
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(missionObject->getTitle()); //Name - ASCII +8 = 104

	//13 -- Refresh Count
	gMessageFactory->addUint32(missionObject->getRefreshCount());//108

	//14 -- Mission Type
	gMessageFactory->addUint32(missionObject->getMissionType());//112

	//15 -- Target Name
	//gMessageFactory->addUint16(0);
	gMessageFactory->addString(missionObject->getTarget()); // "@STF:Name" format - ASCII//114

	//16 -- Mission Waypoint
	gMessageFactory->addUint32(0);
	gMessageFactory->addFloat(missionObject->getWaypoint()->getCoords().mX);  //X
	gMessageFactory->addFloat(missionObject->getWaypoint()->getCoords().mY);  //Y
	gMessageFactory->addFloat(missionObject->getWaypoint()->getCoords().mZ);  //Z
	gMessageFactory->addUint64(0); //Possible Cell ID
	gMessageFactory->addUint32(missionObject->getWaypoint()->getPlanetCRC()); //Planet CRC
	data = missionObject->getWaypoint()->getName();
	data.convert(BSTRType_Unicode16);
	gMessageFactory->addString(data); //Waypoint name "@STF:Name" format - UNICODE
	gMessageFactory->addUint64(missionObject->getWaypoint()->getId()); //Waypoint ID (MISO3 ID +1)
	gMessageFactory->addUint8(missionObject->getWaypoint()->getWPType());  //Waypoint Type
	gMessageFactory->addUint8(missionObject->getWaypoint()->getActive());  //Activated Flag +42 =156
		gMessageFactory->addUint32(0);

	message = gMessageFactory->EndMessage();

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
	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(missionObject->getId()); 
	gMessageFactory->addUint32(opMISO);
	gMessageFactory->addUint8(6);  
	
	gMessageFactory->addUint32(15); //size
	gMessageFactory->addUint16(2);  //Opperand/Variable count

	//####
	//BODY
	//####
	gMessageFactory->addUint64(0x86); 
	gMessageFactory->addUint32(0);    
	gMessageFactory->addUint8(0);    

	message = gMessageFactory->EndMessage();

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
	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(missionObject->getId()); 
	gMessageFactory->addUint32(opMISO);
	gMessageFactory->addUint8(8);  
	gMessageFactory->addUint32(2);    
	gMessageFactory->addUint16(0);  //Opperand count
	

	//####
	//BODY
	//####
	gMessageFactory->addUint32(0);    

	message = gMessageFactory->EndMessage();

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
	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(missionObject->getId()); 
	gMessageFactory->addUint32(opMISO);
	gMessageFactory->addUint8(9);  
	
	gMessageFactory->addUint32(2);  //Opperand count
	

	//####
	//BODY
	//####
	gMessageFactory->addUint16(0);    

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

return true;
}

//======================================================================================================================

bool MessageLib::sendMISO_Delta(MissionObject* missionObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))	return(false);
	if(!missionObject || !targetObject) return false;

	int update_count = 0;
	bytebuffer body;
	
	//NOTE: Do not mess with the order these are in. Being mildly AR I  
	//      arranged them in order according to vID and it stopped working!

	//NOTE
	// use messages as bytebuffers!

	//vID 1 - Name
	if(strcmp(missionObject->getNameFile().getRawData(),"mission/mission_object"))
	{
		update_count++;
		body.writeSHORT(0x01);
		body.writeASTRING(missionObject->getNameFile().getRawData());
		body.writeINT(0);
		body.writeASTRING(missionObject->getName().getRawData());
	}

	//vID 11 - Desciption
	if(missionObject->getDetail().getDataLength() > 0)
	{
		update_count++;
		body.writeSHORT(0x0B); 
		body.writeASTRING(missionObject->getDetailFile().getRawData());
		body.writeINT(0);
		body.writeASTRING(missionObject->getDetail().getRawData());
	}

	//vID 5 - Difficulty
	if(missionObject->getDifficulty() > 0)
	{
		update_count++;
		body.writeSHORT(0x05); 
		body.writeINT(missionObject->getDifficulty());
	}

    //vID 6 - mission start
	if(missionObject->getStart().PlanetCRC > 0)
	{
		update_count++;
		body.writeSHORT(0x06);
		body.writeFLOAT(missionObject->getStart().Coordinates.mX);
		body.writeFLOAT(missionObject->getStart().Coordinates.mY);
		body.writeFLOAT(missionObject->getStart().Coordinates.mZ);
		body.writeLONG(missionObject->getStart().CellID);
		body.writeINT(missionObject->getStart().PlanetCRC);
	}

	//vID 7 - Creator
	if(missionObject->getCreator().getDataLength())
	{
		update_count++;
		body.writeSHORT(0x07);  
		body.writeUSTRING(missionObject->getCreator().getRawData());
	}

	//vID 14 - Mission Type
	if(missionObject->getMissionType() > 0)
	{
		update_count++;
		body.writeSHORT(0x0E); 
		body.writeINT(missionObject->getMissionType());
	}

	//vID 8	- payment
	if(missionObject->getReward() > 0)
	{
		update_count++;
		body.writeSHORT(0x08);
		body.writeINT(missionObject->getReward());
	}

	//vID 9 - mission location
	if(missionObject->getDestination().PlanetCRC > 0)
	{
		update_count++;
		body.writeSHORT(0x09);
		body.writeFLOAT(missionObject->getDestination().Coordinates.mX);
		body.writeFLOAT(missionObject->getDestination().Coordinates.mY);
		body.writeFLOAT(missionObject->getDestination().Coordinates.mZ);
		body.writeLONG(missionObject->getDestination().CellID);
		body.writeINT(missionObject->getDestination().PlanetCRC);
	}

	//vID 10 - Target Model
	if(missionObject->getTargetModel() > 0)
	{
		update_count++;
		body.writeSHORT(0x0A);
		body.writeINT(missionObject->getTargetModel());
	}
			   
	//vID 12 - Title
	if(missionObject->getTitle().getDataLength() > 0)
	{
		update_count++;
		body.writeSHORT(0x0C); 
		body.writeASTRING(missionObject->getTitleFile().getRawData());
		body.writeINT(0);
		body.writeASTRING(missionObject->getTitle().getRawData());
	}

	//vID 13 - Refresh Count
	if(missionObject->getRefreshCount() > 0)
	{
		update_count++;
		body.writeSHORT(0x0D);
		body.writeINT(missionObject->getRefreshCount()); //The count given by the terminal
	}

	//vID 15 - Target Name
	if(missionObject->getTarget().getDataLength() > 0)
	{
		update_count++;
		body.writeSHORT(0x0F); 
		body.writeASTRING(missionObject->getTarget().getRawData());
	}

	//vID 16 - Mission Waypoint
	if(missionObject->getWaypoint()->getPlanetCRC() > 0)
	{
		update_count++;
		body.writeSHORT(0x10);
		body.writeINT(0);
		body.writeFLOAT(missionObject->getWaypoint()->getCoords().mX);  //X
		body.writeFLOAT(missionObject->getWaypoint()->getCoords().mY);  //Y
		body.writeFLOAT(missionObject->getWaypoint()->getCoords().mZ);  //Z
		body.writeLONG(0); //Possible Cell ID
		body.writeINT(missionObject->getWaypoint()->getPlanetCRC()); //Planet CRC
		body.writeUSTRING(missionObject->getWaypoint()->getName().getRawData());
		body.writeLONG(missionObject->getWaypoint()->getId());	 //waypoint id
		body.writeBYTE(missionObject->getWaypoint()->getWPType());   //waypoint type
		body.writeBYTE(missionObject->getWaypoint()->getActive());	 //activated flag
	}

	//Nothing to update
	if(body.getSize() <= 0) 
	{
		return false;
	}
     
	//####
	//HEAD
	//####
	Message* message;
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(missionObject->getId()); 
	gMessageFactory->addUint32(opMISO);
	gMessageFactory->addUint8(3);
	gMessageFactory->addUint32(body.getSize()+2);
	gMessageFactory->addUint16(update_count);
	gMessageFactory->addData(body.getBuffer(),(uint16)body.getSize());
	
	
	message = gMessageFactory->EndMessage();
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
	gMessageFactory->StartMessage();        
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000001B);           
	gMessageFactory->addUint32(opMissionGenericResponse);           
	gMessageFactory->addUint64(missionObject->getOwner()->getId());
	gMessageFactory->addUint32(0);                    
	gMessageFactory->addUint64(missionObject->getId());
	gMessageFactory->addUint8(0x01);
	gMessageFactory->addInt8(0x04);

	message = gMessageFactory->EndMessage();

	targetObject->getClient()->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

return true;
}

//======================================================================================================================

bool MessageLib::sendMissionAbort(MissionObject* missionObject,PlayerObject* targetObject)
{
	if(!missionObject || !targetObject) return false;
	else if(!targetObject->isConnected()) return false;

	Message* message;
	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opMissionAbort);           
	gMessageFactory->addUint64(missionObject->getOwner()->getPlayerObjId());
	gMessageFactory->addUint32(0);  
	gMessageFactory->addUint64(missionObject->getId());

	message = gMessageFactory->EndMessage();

	targetObject->getClient()->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

return true;
}
//======================================================================================================================

bool MessageLib::sendMissionComplete(PlayerObject* targetObject)
{

	if(!targetObject) return false;
	else if(!targetObject->isConnected()) return false;

	Message* message;
	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x00000083);           
	gMessageFactory->addUint32(opMissionComplete);           
	gMessageFactory->addUint64(targetObject->getPlayerObjId());
	gMessageFactory->addUint64(0);  
	gMessageFactory->addUint16(0);
	gMessageFactory->addUint8(0);

	message = gMessageFactory->EndMessage();

	targetObject->getClient()->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

return true;
}


//======================================================================================================================


