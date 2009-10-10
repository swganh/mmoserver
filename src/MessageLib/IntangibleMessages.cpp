/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"

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
// Intangible Baselines Type 3
// contain: name,volume,custom name
//

bool MessageLib::sendBaselinesITNO_3(IntangibleObject* intangibleObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message* message;
	string customName = intangibleObject->getCustomName().getAnsi();
	customName.convert(BSTRType_Unicode16);

	gMessageFactory->StartMessage();

	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(intangibleObject->getId()); 
	gMessageFactory->addUint32(opITNO);
	gMessageFactory->addUint8(3);  

	gMessageFactory->addUint32(26 + (customName.getLength() << 1) + intangibleObject->getName().getLength() + intangibleObject->getNameFile().getLength());
	gMessageFactory->addUint16(5);	//opperand count
	gMessageFactory->addFloat(intangibleObject->getComplexity());	
	
	gMessageFactory->addString(intangibleObject->getNameFile().getAnsi());
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addString(intangibleObject->getName().getAnsi());
	gMessageFactory->addString(customName);
	gMessageFactory->addUint32(intangibleObject->getVolume());
	gMessageFactory->addUint32(0); //GenericInt(CellID,SchematicQuantity)	

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

return true;
}

//======================================================================================================================
//
// Intangible Baselines Type 6
// contain: detail
//

bool MessageLib::sendBaselinesITNO_6(IntangibleObject* intangibleObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message* message;

	gMessageFactory->StartMessage();

	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(intangibleObject->getId()); 
	gMessageFactory->addUint32(opITNO);
	gMessageFactory->addUint8(6);  

	gMessageFactory->addUint32(14 + intangibleObject->getDetail().getLength() + intangibleObject->getDetailFile().getLength());
	gMessageFactory->addUint16(2);	//opperand count
	gMessageFactory->addUint32(74);	// unknown
	gMessageFactory->addString(intangibleObject->getDetailFile().getAnsi());
	gMessageFactory->addUint32(0); //spacer
	gMessageFactory->addString(intangibleObject->getDetail().getAnsi());


	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

return true;
}

//======================================================================================================================
//
// Intangible Baselines Type 8
// contain: nothing!
//

bool MessageLib::sendBaselinesITNO_8(IntangibleObject* intangibleObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message* message;

	gMessageFactory->StartMessage();

	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(intangibleObject->getId()); 
	gMessageFactory->addUint32(opITNO);
	gMessageFactory->addUint8(8);  

	gMessageFactory->addUint32(2); //Size
	gMessageFactory->addUint16(0);	//opperand count

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

return true;
}


//======================================================================================================================
//
// Intangible Baselines Type 9
// contain: nothing!
//

bool MessageLib::sendBaselinesITNO_9(IntangibleObject* intangibleObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message* message;

	gMessageFactory->StartMessage();

	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(intangibleObject->getId()); 
	gMessageFactory->addUint32(opITNO);
	gMessageFactory->addUint8(9); 
	gMessageFactory->addUint32(2); //Size
	gMessageFactory->addUint16(0);	//opperand count
	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

return true;
}
