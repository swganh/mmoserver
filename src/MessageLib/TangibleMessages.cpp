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

	mMessageFactory->StartMessage();

	mMessageFactory->addUint32(opBaselinesMessage);   
	mMessageFactory->addUint64(tangibleObject->getId()); 
	mMessageFactory->addUint32(opTANO);
	mMessageFactory->addUint8(3);  

	mMessageFactory->addUint32(49 + (customName.getLength() << 1) + tangibleObject->getName().getLength() + tangibleObject->getCustomizationStr().getLength() + tangibleObject->getNameFile().getLength());
	mMessageFactory->addUint16(11);	
	mMessageFactory->addFloat(0);//tangibleObject->getComplexity());
	mMessageFactory->addString(tangibleObject->getNameFile());
	mMessageFactory->addUint32(0);	// unknown
	mMessageFactory->addString(tangibleObject->getName());
	mMessageFactory->addString(customName);
	uint32 uses = 0;

	mMessageFactory->addUint32(1);//volume gives the volume taken up in the inventory!!!!!!!!
	mMessageFactory->addString(tangibleObject->getCustomizationStr());
	mMessageFactory->addUint64(0);	// unknown list might be defender list
	mMessageFactory->addUint32(tangibleObject->getTypeOptions());

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

	mMessageFactory->addUint32(uses);
	mMessageFactory->addUint32(tangibleObject->getDamage());
	mMessageFactory->addUint32(tangibleObject->getMaxCondition());
	
	//1 when not moveable
	mMessageFactory->addUint8(tangibleObject->getStatic());	// !!!!
	

	message = mMessageFactory->EndMessage();

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

	mMessageFactory->StartMessage();  
	mMessageFactory->addUint32(opBaselinesMessage);   
	mMessageFactory->addUint64(tangibleObject->getId()); 
	mMessageFactory->addUint32(opTANO);
	mMessageFactory->addUint8(6);  

	mMessageFactory->addUint32(15 + tangibleObject->getUnknownStr1().getLength() + tangibleObject->getUnknownStr2().getLength());
	mMessageFactory->addUint16(3);	// unknown
	mMessageFactory->addUint32(tangibleObject->getSubZoneId());
	mMessageFactory->addString(tangibleObject->getUnknownStr1());
	mMessageFactory->addUint32(0);	// unknown
	mMessageFactory->addString(tangibleObject->getUnknownStr2());
	mMessageFactory->addUint8(0);	// unknown

	message = mMessageFactory->EndMessage();

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

	mMessageFactory->StartMessage();       
	mMessageFactory->addUint32(opBaselinesMessage);   
	mMessageFactory->addUint64(tangibleObject->getId()); 
	mMessageFactory->addUint32(opTANO);
	mMessageFactory->addUint8(8);  

	mMessageFactory->addUint32(2);
	mMessageFactory->addUint16(0);	// unknown

	message = mMessageFactory->EndMessage();

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

	mMessageFactory->StartMessage();         
	mMessageFactory->addUint32(opBaselinesMessage);   
	mMessageFactory->addUint64(tangibleObject->getId()); 
	mMessageFactory->addUint32(opTANO);
	mMessageFactory->addUint8(9);  

	mMessageFactory->addUint32(2);
	mMessageFactory->addUint16(0);	// unknown

	message = mMessageFactory->EndMessage();

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

	mMessageFactory->StartMessage();  
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(tangibleObject->getId());
	mMessageFactory->addUint32(opTANO);
	mMessageFactory->addUint8(3);

	mMessageFactory->addUint32(8);
	mMessageFactory->addUint16(1);

	mMessageFactory->addUint16(0);
	mMessageFactory->addFloat(tangibleObject->getComplexity());

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

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

	mMessageFactory->StartMessage();  
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(tangibleObject->getId());
	mMessageFactory->addUint32(opTANO);
	mMessageFactory->addUint8(3);

	mMessageFactory->addUint32(6+tangibleObject->getCustomizationStr().getLength());//length

	mMessageFactory->addUint16(1);	   //one update

	mMessageFactory->addUint16(4);	   //nr 4 = customization
	mMessageFactory->addString(tangibleObject->getCustomizationStr());

	_sendToInRange(mMessageFactory->EndMessage(),playerObject,8,true);
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

	mMessageFactory->StartMessage();  
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(tangibleObject->getId());
	mMessageFactory->addUint32(opTANO);
	mMessageFactory->addUint8(3);

	mMessageFactory->addUint32(8);		//length

	mMessageFactory->addUint16(1);	   //one update

	mMessageFactory->addUint16(6);	   //nr 6 = type option.
	mMessageFactory->addUint32(tangibleObject->getTypeOptions());

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);
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

	mMessageFactory->StartMessage();  
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(tangibleObject->getId());
	mMessageFactory->addUint32(opTANO);
	mMessageFactory->addUint8(3);

	mMessageFactory->addUint32(8);
	mMessageFactory->addUint16(1);

	mMessageFactory->addUint16(7);
	mMessageFactory->addUint32(tangibleObject->getTimer());

	newMessage = mMessageFactory->EndMessage();

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

	mMessageFactory->StartMessage();  
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(tangibleObject->getId());
	mMessageFactory->addUint32(opTANO);
	mMessageFactory->addUint8(3);

	mMessageFactory->addUint32(8);
	mMessageFactory->addUint16(1);

	mMessageFactory->addUint16(7);
	mMessageFactory->addUint32(uses);

	newMessage = mMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage,playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
