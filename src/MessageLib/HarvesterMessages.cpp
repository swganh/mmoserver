		   /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"

#include "ZoneServer/Deed.h"
#include "ZoneServer/HarvesterObject.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"

#include "LogManager/LogManager.h"

#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"


//======================================================================================================================
//
// Building Baselines Type 3
// contain: name,
//

bool MessageLib::sendBaselinesHINO_3(HarvesterObject* harvester,PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();    
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(harvester->getId());
	gMessageFactory->addUint32(opHINO);
	gMessageFactory->addUint8(3);

	uint32 byteCount = 59 + harvester->getNameFile().getLength() + harvester->getName().getLength();
	gMessageFactory->addUint32(byteCount);
	gMessageFactory->addUint16(16);
	gMessageFactory->addFloat(1.0);
	gMessageFactory->addString(harvester->getNameFile());
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(harvester->getName());
	gMessageFactory->addString(harvester->getCustomName());
	
	gMessageFactory->addUint32(1);//volume (in inventory)
	gMessageFactory->addUint16(0);//customization
	gMessageFactory->addUint32(0);//list
	gMessageFactory->addUint32(0);//list
	gMessageFactory->addUint32(0);//optionsbitmask
	gMessageFactory->addUint32(0);//timer
	gMessageFactory->addUint32(0);//condition damage
	gMessageFactory->addUint32(1000);   //maxcondition
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(0);//active flag
	gMessageFactory->addFloat(0);//power reserve
	gMessageFactory->addFloat(0);//power cost
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);

	newMessage = gMessageFactory->EndMessage();

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

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(harvester->getId());
	gMessageFactory->addUint32(opHINO);
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(14);
	gMessageFactory->addUint16(2);	// unknown
	gMessageFactory->addUint16(0); // unknown
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);

	newMessage = gMessageFactory->EndMessage();

	(player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

	return(true);
}

bool MessageLib::sendBaselinesHINO_7(HarvesterObject* harvester,PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(harvester->getId());
	gMessageFactory->addUint32(opHINO);
	gMessageFactory->addUint8(7);

	gMessageFactory->addUint32(16);
	gMessageFactory->addUint8(1);	// respoolupdate
	
	//gMessageFactory->addUint32(); // respool IDs count
	//gMessageFactory->addUint32(); // respool IDs list counter
		//gMessageFactory->addUint64(); // ID

	//gMessageFactory->addUint32(); // respool Names count
	//gMessageFactory->addUint32(); // respool Names list counter
		//gMessageFactory->addString(); // ressourceName


	gMessageFactory->addUint32(66); // unknown
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);

	newMessage = gMessageFactory->EndMessage();

	(player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

	return(true);
}


//======================================================================================================================
//
// Building Baselines Type 3
// contain: name,
//

bool MessageLib::sendBaselinesINSO_3(PlayerStructure* structure,PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();    
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(structure->getId());
	gMessageFactory->addUint32(opINSO);
	gMessageFactory->addUint8(3);

	structure->getNameFile().convert(BSTRType_ANSI);
	structure->getName().convert(BSTRType_ANSI);

	printf(structure->getNameFile().getAnsi());
	printf(structure->getName().getAnsi());

	uint32 byteCount = 61 + structure->getNameFile().getLength() + structure->getName().getLength();
	
	gMessageFactory->addUint32(byteCount);
	gMessageFactory->addUint16(10);
	gMessageFactory->addFloat(1.0);
	gMessageFactory->addString(structure->getNameFile());
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(structure->getName());
	gMessageFactory->addUint32(0);
	//gMessageFactory->addString("");
	
	gMessageFactory->addUint32(1);//volume (in inventory)
	gMessageFactory->addUint16(0);//customization
	gMessageFactory->addUint32(0);//list
	gMessageFactory->addUint32(0);//list
	gMessageFactory->addUint32(0);//optionsbitmask
	gMessageFactory->addUint32(0);//timer
	gMessageFactory->addUint32(0);//condition damage
	gMessageFactory->addUint32(0);   //maxcondition
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(0);//active flag
	gMessageFactory->addFloat(0);//power reserve
	gMessageFactory->addFloat(0);//power cost
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);

	newMessage = gMessageFactory->EndMessage();

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

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(structure->getId());
	gMessageFactory->addUint32(opINSO);
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(43);
	gMessageFactory->addUint16(0);	// unknown
	gMessageFactory->addUint16(0); // unknown
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);

	newMessage = gMessageFactory->EndMessage();

	(player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

	return(true);
}