/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"

#include "ZoneServer/BuildingObject.h"
#include "ZoneServer/CellObject.h"
#include "ZoneServer/HouseObject.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"

#include "LogManager/LogManager.h"
#include "ZoneServer/PlayerStructure.h"

#include "Common/DispatchClient.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "Common/Message.h"



//======================================================================================================================
//
// Building Baselines Type 3
// contain: name,
//

bool MessageLib::sendBaselinesBUIO_3(BuildingObject* buildingObject,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();    
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(buildingObject->getId());
	gMessageFactory->addUint32(opBUIO);
	gMessageFactory->addUint8(3);

	uint32 byteCount = 49 + buildingObject->getNameFile().getLength() + buildingObject->getName().getLength();
	gMessageFactory->addUint32(byteCount);
	gMessageFactory->addUint16(11);
	gMessageFactory->addFloat(1.0);
	gMessageFactory->addString(buildingObject->getNameFile());
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(buildingObject->getName());
	gMessageFactory->addUint32(0);//custom name
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint16(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(1000);
	gMessageFactory->addUint8(1);

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

bool MessageLib::sendBaselinesBUIO_3(HouseObject* buildingObject,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* fragment;

	gMessageFactory->StartMessage();    
	gMessageFactory->addUint16(11);
	gMessageFactory->addFloat(1.0);
	gMessageFactory->addString(buildingObject->getNameFile());
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(buildingObject->getName());
	gMessageFactory->addUint32(0);//custom name
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint16(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(1);

	fragment = gMessageFactory->EndMessage();


	Message* newMessage;

	gMessageFactory->StartMessage();    
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(buildingObject->getId());
	gMessageFactory->addUint32(opBUIO);
	gMessageFactory->addUint8(3);
	gMessageFactory->addUint32(fragment->getSize());
	gMessageFactory->addData(fragment->getData(),fragment->getSize());

	newMessage = gMessageFactory->EndMessage();
	fragment->setPendingDelete(true);

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	return(true);
}


//======================================================================================================================
//
// Building Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesBUIO_6(BuildingObject* buildingObject,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(buildingObject->getId());
	gMessageFactory->addUint32(opBUIO);
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(14);
	gMessageFactory->addUint16(2);	// unknown
	gMessageFactory->addUint32(66); // unknown
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	return(true);
}


bool MessageLib::sendBaselinesBUIO_6(HouseObject* buildingObject,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(buildingObject->getId());
	gMessageFactory->addUint32(opBUIO);
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(14);
	gMessageFactory->addUint16(2);	// unknown
	gMessageFactory->addUint32(0); // unknown
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	return(true);
}


//======================================================================================================================
//
// Cell Baselines Type 3
// contain: cell nr,
//

bool MessageLib::sendBaselinesSCLT_3(CellObject* cellObject,uint64 cellNr,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(cellObject->getId());
	gMessageFactory->addUint32(opSCLT);
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(26);
	gMessageFactory->addUint16(5);	// unknown
	gMessageFactory->addUint32(0); // unknown
	gMessageFactory->addUint16(0);
	gMessageFactory->addUint32(0); // unknown
	gMessageFactory->addUint16(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(static_cast<uint32>(cellNr));

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Cell Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesSCLT_6(CellObject* cellObject,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(cellObject->getId());
	gMessageFactory->addUint32(opSCLT);
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(14);
	gMessageFactory->addUint16(2);	// unknown
	gMessageFactory->addUint32(149); // unknown
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	return(true);
}


//======================================================================================================================
//
// advises the client to go into the structure placement mode
//
//

bool MessageLib::sendEnterStructurePlacement(Object* deed, string objectString, PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opStructurePlacementMode);  
	gMessageFactory->addUint64(deed->getId());
	//object/building/player/shared_player_house_tatooine_large_style_01.iff
	//char *dir = "object/installation/generators/shared_power_generator_fusion_style_1.iff";
	gMessageFactory->addString(objectString);

	newMessage = gMessageFactory->EndMessage();
	
	//gLogger->logMsgF("placement mode : %s",MSG_HIGH,objectString.getAnsi());

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	return(true);
}


//======================================================================================================================
//
// sends the Admin List for a structure
//

bool MessageLib::sendAdminList(PlayerStructure* structure, PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opSendPermissionList);  
	gMessageFactory->addUint32(structure->getStrucureAdminList().size() );

	string name;
	BStringVector vector = 	structure->getStrucureAdminList();
	BStringVector::iterator it = vector.begin();
	while(it != vector.end())
	{
		name = (*it);
		name.convert(BSTRType_Unicode16);
		gMessageFactory->addString(name);

		it++;
	}

	gMessageFactory->addUint32(0); // ???
	//gMessageFactory->addUint16(0);	// unknown
	name = "ADMIN";
	name.convert(BSTRType_Unicode16);
	gMessageFactory->addString(name);
	gMessageFactory->addUint32(0); // ???
	
	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	structure->resetStructureAdminList();

	return(true);
}

//======================================================================================================================
//
// sends the Admin List for a structure
//

bool MessageLib::sendEntryList(PlayerStructure* structure, PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opSendPermissionList);  
	gMessageFactory->addUint32(structure->getStrucureEntryList().size() );

	string name;
	BStringVector vector = 	structure->getStrucureEntryList();
	BStringVector::iterator it = vector.begin();
	while(it != vector.end())
	{
		name = (*it);
		name.convert(BSTRType_Unicode16);
		gMessageFactory->addString(name);

		it++;
	}

	gMessageFactory->addUint32(0); // ???
	//gMessageFactory->addUint16(0);	// unknown
	name = "ENTRY";
	name.convert(BSTRType_Unicode16);
	gMessageFactory->addString(name);
	gMessageFactory->addUint32(0); // ???
	
	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	structure->resetStructureAdminList();

	return(true);
}


//======================================================================================================================
//
// sends the Admin List for a structure
//

bool MessageLib::sendBanList(PlayerStructure* structure, PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opSendPermissionList);  
	gMessageFactory->addUint32(structure->getStrucureBanList().size() );

	string name;
	BStringVector vector = 	structure->getStrucureBanList();
	BStringVector::iterator it = vector.begin();
	while(it != vector.end())
	{
		name = (*it);
		name.convert(BSTRType_Unicode16);
		gMessageFactory->addString(name);

		it++;
	}

	gMessageFactory->addUint32(0); // ???
	//gMessageFactory->addUint16(0);	// unknown
	name = "BAN";
	name.convert(BSTRType_Unicode16);
	gMessageFactory->addString(name);
	gMessageFactory->addUint32(0); // ???
	
	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	structure->resetStructureAdminList();

	return(true);
}

