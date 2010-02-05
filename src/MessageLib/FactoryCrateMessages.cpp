/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"


#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/FactoryCrate.h"
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

bool MessageLib::sendBaselinesTYCF_3(FactoryCrate* crate,PlayerObject* targetObject) 
{
	if(!(targetObject->isConnected()))
		return(false);

	string customName;
	string NameFile;
	string Name;

	TangibleObject* tO = crate->getLinkedObject();
	if(!tO)
	{
		assert(false);
		customName = crate->getCustomName();
		NameFile = crate->getNameFile();
		Name = crate->getName();
	}
	else
	{
		customName = tO->getCustomName();
		NameFile = tO->getNameFile();
		Name = tO->getName();
	}
	

	customName.convert(BSTRType_Unicode16);

	gMessageFactory->StartMessage();

	
	gMessageFactory->addUint16(11);	//op count
	gMessageFactory->addFloat(1.0);//tangibleObject->getComplexity());
	gMessageFactory->addString(NameFile.getAnsi());
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addString(Name.getAnsi());
	gMessageFactory->addString(customName.getUnicode16());
	uint32 uses = 0;

	gMessageFactory->addUint32(1);//volume gives the volume taken up in the inventory!!!!!!!!
	//gMessageFactory->addString(crate->getCustomizationStr());
	gMessageFactory->addUint16(0);//crate customization
	gMessageFactory->addUint64(0);	// unknown list might be defender list
	gMessageFactory->addUint32(0);//crate->getTypeOptions());bitmask - insured etc

	if(crate->hasAttribute("factory_count"))
	{
		uses = crate->getAttribute<int>("factory_count");
	}

	gMessageFactory->addUint32(uses);//
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	
	//1 when not moveable
	gMessageFactory->addUint8(0);	// 
	gMessageFactory->addUint64(0);	// 
	

	Message* data = gMessageFactory->EndMessage();


	gMessageFactory->StartMessage();

	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(crate->getId()); 
	gMessageFactory->addUint32(opFCYT);
	gMessageFactory->addUint8(3);  

	gMessageFactory->addUint32(data->getSize());
	gMessageFactory->addData(data->getData(), data->getSize());
	
	data->setPendingDelete(true);

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Crate Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesTYCF_6( FactoryCrate*  crate,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message* data;

	gMessageFactory->StartMessage();  

	gMessageFactory->addUint16(3);	// unknown
	gMessageFactory->addUint32(0);	// unknown
	//gMessageFactory->addString();//crate->getDetailFile());
	gMessageFactory->addUint32(0);	// unknown
	//gMessageFactory->addString(0);//crate->getNameFile());
	
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);	// unknown

	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);	// unknown

	
	gMessageFactory->addUint8(0);	// unknown

	data = gMessageFactory->EndMessage();

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(crate->getId()); 
	gMessageFactory->addUint32(opFCYT);
	gMessageFactory->addUint8(6);  

	gMessageFactory->addUint32(data->getSize());
	gMessageFactory->addData(data->getData(), data->getSize());
	
	data->setPendingDelete(true);


	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Tangible Baselines Type 8
// contain: unknown
//

bool MessageLib::sendBaselinesTYCF_8(FactoryCrate* crate,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message* data;

	gMessageFactory->StartMessage();  

	gMessageFactory->addUint16(3);	// unknown
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);	// unknown
	
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);	// unknown
	
	data = gMessageFactory->EndMessage();

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(crate->getId()); 
	gMessageFactory->addUint32(opFCYT);
	gMessageFactory->addUint8(8);  

	gMessageFactory->addUint32(data->getSize());
	gMessageFactory->addData(data->getData(), data->getSize());
	
	data->setPendingDelete(true);


	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}



//======================================================================================================================
//
// Tangible Baselines Type 9
// contain: unknown
//

bool MessageLib::sendBaselinesTYCF_9(FactoryCrate* crate,PlayerObject* targetObject)

{
	if(!(targetObject->isConnected()))
		return(false);

	Message* data;

	gMessageFactory->StartMessage();  

	gMessageFactory->addUint16(3);	// unknown
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);	// unknown
	
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);	// unknown
	
	data = gMessageFactory->EndMessage();

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(crate->getId()); 
	gMessageFactory->addUint32(opFCYT);
	gMessageFactory->addUint8(9);  

	gMessageFactory->addUint32(data->getSize());
	gMessageFactory->addData(data->getData(), data->getSize());
	
	data->setPendingDelete(true);


	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}


bool MessageLib::sendUpdateCrateContent(FactoryCrate* crate,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	uint32 uses = 0;

	if(crate->hasAttribute("factory_count"))
	{
		uses = crate->getAttribute<int>("factory_count");
	}

		   
	Message* newMessage;

	gMessageFactory->StartMessage();  
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(crate->getId());
	gMessageFactory->addUint32(opFCYT);
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(8);
	gMessageFactory->addUint16(1);

	gMessageFactory->addUint16(7);
	gMessageFactory->addUint32(uses);

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage,playerObject->getAccountId(),CR_Client,5);

	return(true);
}
