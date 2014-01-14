/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"
#include "ZoneServer/CraftingTool.h"
#include "ZoneServer/ManufacturingSchematic.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/StaticObject.h"
#include "ZoneServer/Wearable.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"

#include "LogManager/LogManager.h"

#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"

//======================================================================================================================
//
// Static Baselines type 3
// contain: 
//

bool MessageLib::sendBaselinesSTAO_3(TangibleObject* staticObject, PlayerObject* targetObject) 
{
	if(!(targetObject->isConnected()))
		return(false);

	Message* message;
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opBaselinesMessage);   
	mMessageFactory->addUint64(staticObject->getId()); 
	mMessageFactory->addUint32(opSTAO);
	mMessageFactory->addUint8(3);  
	mMessageFactory->addUint32(41);
	mMessageFactory->addUint32(4); 
	mMessageFactory->addUint16(0);
	mMessageFactory->addString("obj_n");
	mMessageFactory->addUint32(0); 
	mMessageFactory->addString("unknown_object");
	mMessageFactory->addUint32(0); 
	mMessageFactory->addUint16(0xFF);
	message = mMessageFactory->EndMessage();
	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Static Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesSTAO_6(TangibleObject* staticObject, PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message* message;

	mMessageFactory->StartMessage();  
	mMessageFactory->addUint32(opBaselinesMessage);
	mMessageFactory->addUint64(staticObject->getId());
	mMessageFactory->addUint32(opSTAO);
	mMessageFactory->addUint8(6);
	mMessageFactory->addUint32(29);
	mMessageFactory->addUint16(2);
	mMessageFactory->addUint32(0x00000072);
	mMessageFactory->addString("string_id_table");
	mMessageFactory->addUint32(0x00);	
	mMessageFactory->addUint16(0x00);	
	message = mMessageFactory->EndMessage();
	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
