/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"
#include "ZoneServer/Deed.h"
#include "LogManager/LogManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ObjectFactory.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"
#include "ZoneServer/PlayerObject.h"

//======================================================================================================================
//
// Building Baselines Type 3
// contain: name,
//


bool MessageLib::sendBaselinesBUIO_3(TangibleObject* structure,PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();    
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(structure->getId());
	gMessageFactory->addUint32(opBUIO);
	gMessageFactory->addUint8(3);

	uint32 byteCount = 49 + structure->getNameFile().getLength() + structure->getName().getLength();
	gMessageFactory->addUint32(byteCount);
	gMessageFactory->addUint16(11);
	gMessageFactory->addFloat(1.0);
	gMessageFactory->addString(structure->getNameFile());
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(structure->getName());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0xFF);
	gMessageFactory->addUint16(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(256);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(1000);
	gMessageFactory->addUint8(1);

	newMessage = gMessageFactory->EndMessage();

	(player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Building Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesBUIO_6(TangibleObject* structure,PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(structure->getId());
	gMessageFactory->addUint32(opBUIO);
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(14);
	gMessageFactory->addUint16(2);	// unknown
	gMessageFactory->addUint32(66); // unknown
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);

	newMessage = gMessageFactory->EndMessage();

	(player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

	return(true);
}
