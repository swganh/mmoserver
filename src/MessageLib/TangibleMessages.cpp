/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"

#include "ZoneServer/CraftingTool.h"
#include "ZoneServer/ManufacturingSchematic.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/Wearable.h"
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
// Tangible Baselines Type 3
// contain: general information, name, customization, type, condition
//

bool MessageLib::sendBaselinesTANO_3(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const
{
	if(!(targetObject->isConnected()))
		return(false);

	Message* message;
	string customName = tangibleObject->getCustomName().getAnsi();
	customName.convert(BSTRType_Unicode16);

	gMessageFactory->StartMessage();

	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(tangibleObject->getId()); 
	gMessageFactory->addUint32(opTANO);
	gMessageFactory->addUint8(3);  

	gMessageFactory->addUint32(49 + (customName.getLength() << 1) + tangibleObject->getName().getLength() + tangibleObject->getCustomizationStr().getLength() + tangibleObject->getNameFile().getLength());
	gMessageFactory->addUint16(11);	
	gMessageFactory->addFloat(0);//tangibleObject->getComplexity());
	gMessageFactory->addString(tangibleObject->getNameFile());
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addString(tangibleObject->getName());
	gMessageFactory->addString(customName);
	uint32 uses = 0;

	gMessageFactory->addUint32(1);//volume gives the volume taken up in the inventory!!!!!!!!
	gMessageFactory->addString(tangibleObject->getCustomizationStr());
	gMessageFactory->addUint64(0);	// unknown list might be defender list
	gMessageFactory->addUint32(tangibleObject->getTypeOptions());

	if(tangibleObject->hasAttribute("counter_uses_remaining"))
	{
		float fUses = tangibleObject->getAttribute<float>("counter_uses_remaining");
		uses = (int) fUses;
	}

	if(tangibleObject->hasAttribute("stacksize"))
	{
		uses = tangibleObject->getAttribute<int>("stacksize");
	}
	if(tangibleObject->getTimer() != 0)
		uses = tangibleObject->getTimer();

	gMessageFactory->addUint32(uses);
	gMessageFactory->addUint32(tangibleObject->getDamage());
	gMessageFactory->addUint32(tangibleObject->getMaxCondition());
	
	//1 when not moveable
	gMessageFactory->addUint8(1);	// 
	

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Tangible Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesTANO_6(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const
{
	if(!(targetObject->isConnected()))
		return(false);

	Message* message;

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(tangibleObject->getId()); 
	gMessageFactory->addUint32(opTANO);
	gMessageFactory->addUint8(6);  

	gMessageFactory->addUint32(15 + tangibleObject->getUnknownStr1().getLength() + tangibleObject->getUnknownStr2().getLength());
	gMessageFactory->addUint16(3);	// unknown
	gMessageFactory->addUint32(tangibleObject->getSubZoneId());
	gMessageFactory->addString(tangibleObject->getUnknownStr1());
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addString(tangibleObject->getUnknownStr2());
	gMessageFactory->addUint8(1);	// unknown

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Tangible Baselines Type 8
// contain: unknown
//

bool MessageLib::sendBaselinesTANO_8(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const
{
	if(!(targetObject->isConnected()))
		return(false);

	Message* message;

	gMessageFactory->StartMessage();       
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(tangibleObject->getId()); 
	gMessageFactory->addUint32(opTANO);
	gMessageFactory->addUint8(8);  

	gMessageFactory->addUint32(2);
	gMessageFactory->addUint16(0);	// unknown

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Tangible Baselines Type 9
// contain: unknown
//

bool MessageLib::sendBaselinesTANO_9(const TangibleObject* const tangibleObject,const PlayerObject* const targetObject) const
{
	if(!(targetObject->isConnected()))
		return(false);

	Message* message;

	gMessageFactory->StartMessage();         
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(tangibleObject->getId()); 
	gMessageFactory->addUint32(opTANO);
	gMessageFactory->addUint8(9);  

	gMessageFactory->addUint32(2);
	gMessageFactory->addUint16(0);	// unknown

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Tangible deltas Type 3
// update: complexity
//

bool MessageLib::sendUpdateComplexity(TangibleObject* tangibleObject,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(tangibleObject->getId());
	gMessageFactory->addUint32(opTANO);
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(8);
	gMessageFactory->addUint16(1);

	gMessageFactory->addUint16(0);
	gMessageFactory->addFloat(tangibleObject->getComplexity());

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
// Tangible deltas Type 3
// update: customization string
//

bool  MessageLib::sendUpdateCustomization_InRange(TangibleObject* tangibleObject,PlayerObject* playerObject)
{
		if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(tangibleObject->getId());
	gMessageFactory->addUint32(opTANO);
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(6+tangibleObject->getCustomizationStr().getLength());//length

	gMessageFactory->addUint16(1);	   //one update

	gMessageFactory->addUint16(4);	   //nr 4 = customization
	gMessageFactory->addString(tangibleObject->getCustomizationStr());

	_sendToInRange(gMessageFactory->EndMessage(),playerObject,8,true);
	//(playerObject->getClient())->SendChannelA(newMessage,playerObject->getAccountId(),CR_Client,5,false);

	return(true);


}

//======================================================================================================================
//
// Tangible deltas Type 3
// update: type option
//

bool MessageLib::sendUpdateTypeOption(TangibleObject* tangibleObject,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(tangibleObject->getId());
	gMessageFactory->addUint32(opTANO);
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(8);		//length

	gMessageFactory->addUint16(1);	   //one update

	gMessageFactory->addUint16(6);	   //nr 6 = type option.
	gMessageFactory->addUint32(tangibleObject->getTypeOptions());

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);
	return(true);
}

//======================================================================================================================
//
// Tangible deltas Type 3
// update: timer
//

bool MessageLib::sendUpdateTimer(TangibleObject* tangibleObject,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(tangibleObject->getId());
	gMessageFactory->addUint32(opTANO);
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(8);
	gMessageFactory->addUint16(1);

	gMessageFactory->addUint16(7);
	gMessageFactory->addUint32(tangibleObject->getTimer());

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage,playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================



bool MessageLib::sendUpdateUses(TangibleObject* tangibleObject,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	uint32 uses = 0;

	if(tangibleObject->hasAttribute("counter_uses_remaining"))
	{
		uses = tangibleObject->getAttribute<int>("counter_uses_remaining");
	}

		   
	Message* newMessage;

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(tangibleObject->getId());
	gMessageFactory->addUint32(opTANO);
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(8);
	gMessageFactory->addUint16(1);

	gMessageFactory->addUint16(7);
	gMessageFactory->addUint32(uses);

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage,playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
