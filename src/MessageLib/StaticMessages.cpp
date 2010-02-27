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
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(staticObject->getId()); 
	gMessageFactory->addUint32(opSTAO);
	gMessageFactory->addUint8(3);  
	gMessageFactory->addUint32(41);
	gMessageFactory->addUint32(4); 
	gMessageFactory->addUint16(0);
	gMessageFactory->addString("obj_n");
	gMessageFactory->addUint32(0); 
	gMessageFactory->addString("unknown_object");
	gMessageFactory->addUint32(0); 
	gMessageFactory->addUint16(0xFF);
	message = gMessageFactory->EndMessage();
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

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opBaselinesMessage);
	gMessageFactory->addUint64(staticObject->getId());
	gMessageFactory->addUint32(opSTAO);
	gMessageFactory->addUint8(6);
	gMessageFactory->addUint32(29);
	gMessageFactory->addUint16(2);
	gMessageFactory->addUint32(0x00000072);
	gMessageFactory->addString("string_id_table");
	gMessageFactory->addUint32(0x00);	
	gMessageFactory->addUint16(0x00);	
	message = gMessageFactory->EndMessage();
	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
