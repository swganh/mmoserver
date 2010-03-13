/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"

#include "ZoneServer/Deed.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
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


bool MessageLib::sendBaselinesBUIO_3(TangibleObject* structure,PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	Message* newMessage;

	mMessageFactory->StartMessage();    
	mMessageFactory->addUint32(opBaselinesMessage);  
	mMessageFactory->addUint64(structure->getId());
	mMessageFactory->addUint32(opBUIO);
	mMessageFactory->addUint8(3);

	uint32 byteCount = 49 + structure->getNameFile().getLength() + structure->getName().getLength();
	mMessageFactory->addUint32(byteCount);
	mMessageFactory->addUint16(11);
	mMessageFactory->addFloat(1.0);
	mMessageFactory->addString(structure->getNameFile());
	mMessageFactory->addUint32(0);
	mMessageFactory->addString(structure->getName());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(0xFF);
	mMessageFactory->addUint16(0);
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(256);
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(1000);
	mMessageFactory->addUint8(1);

	newMessage = mMessageFactory->EndMessage();

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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opBaselinesMessage);  
	mMessageFactory->addUint64(structure->getId());
	mMessageFactory->addUint32(opBUIO);
	mMessageFactory->addUint8(6);

	mMessageFactory->addUint32(14);
	mMessageFactory->addUint16(2);	// unknown
	mMessageFactory->addUint32(66); // unknown
	mMessageFactory->addUint32(0);	// unknown
	mMessageFactory->addUint32(0);

	newMessage = mMessageFactory->EndMessage();

	(player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

	return(true);
}
