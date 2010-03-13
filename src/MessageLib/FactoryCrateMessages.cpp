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

	mMessageFactory->StartMessage();

	
	mMessageFactory->addUint16(11);	//op count
	mMessageFactory->addFloat(1.0);//tangibleObject->getComplexity());
	mMessageFactory->addString(NameFile.getAnsi());
	mMessageFactory->addUint32(0);	// unknown
	mMessageFactory->addString(Name.getAnsi());
	mMessageFactory->addString(customName.getUnicode16());
	uint32 uses = 0;

	mMessageFactory->addUint32(1);//volume gives the volume taken up in the inventory!!!!!!!!
	//mMessageFactory->addString(crate->getCustomizationStr());
	mMessageFactory->addUint16(0);//crate customization
	mMessageFactory->addUint64(0);	// unknown list might be defender list
	mMessageFactory->addUint32(0);//crate->getTypeOptions());bitmask - insured etc

	if(crate->hasAttribute("factory_count"))
	{
		uses = crate->getAttribute<int>("factory_count");
	}

	mMessageFactory->addUint32(uses);//
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(0);
	
	//1 when not moveable
	mMessageFactory->addUint8(0);	// 
	mMessageFactory->addUint64(0);	// 
	

	Message* data = mMessageFactory->EndMessage();


	mMessageFactory->StartMessage();

	mMessageFactory->addUint32(opBaselinesMessage);   
	mMessageFactory->addUint64(crate->getId()); 
	mMessageFactory->addUint32(opFCYT);
	mMessageFactory->addUint8(3);  

	mMessageFactory->addUint32(data->getSize());
	mMessageFactory->addData(data->getData(), data->getSize());
	
	data->setPendingDelete(true);

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

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

	mMessageFactory->StartMessage();  

	mMessageFactory->addUint16(3);	// unknown
	mMessageFactory->addUint32(0);	// unknown
	//mMessageFactory->addString();//crate->getDetailFile());
	mMessageFactory->addUint32(0);	// unknown
	//mMessageFactory->addString(0);//crate->getNameFile());
	
	mMessageFactory->addUint32(0);	// unknown
	mMessageFactory->addUint32(0);	// unknown
	mMessageFactory->addUint32(0);	// unknown
	mMessageFactory->addUint32(0);	// unknown

	mMessageFactory->addUint32(0);	// unknown
	mMessageFactory->addUint32(0);	// unknown

	
	mMessageFactory->addUint8(0);	// unknown

	data = mMessageFactory->EndMessage();

	mMessageFactory->StartMessage();  
	mMessageFactory->addUint32(opBaselinesMessage);   
	mMessageFactory->addUint64(crate->getId()); 
	mMessageFactory->addUint32(opFCYT);
	mMessageFactory->addUint8(6);  

	mMessageFactory->addUint32(data->getSize());
	mMessageFactory->addData(data->getData(), data->getSize());
	
	data->setPendingDelete(true);


	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

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

	mMessageFactory->StartMessage();  

	mMessageFactory->addUint16(3);	// unknown
	mMessageFactory->addUint32(0);	// unknown
	mMessageFactory->addUint32(0);	// unknown
	
	mMessageFactory->addUint32(0);	// unknown
	mMessageFactory->addUint32(0);	// unknown
	
	data = mMessageFactory->EndMessage();

	mMessageFactory->StartMessage();  
	mMessageFactory->addUint32(opBaselinesMessage);   
	mMessageFactory->addUint64(crate->getId()); 
	mMessageFactory->addUint32(opFCYT);
	mMessageFactory->addUint8(8);  

	mMessageFactory->addUint32(data->getSize());
	mMessageFactory->addData(data->getData(), data->getSize());
	
	data->setPendingDelete(true);


	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

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

	mMessageFactory->StartMessage();  

	mMessageFactory->addUint16(3);	// unknown
	mMessageFactory->addUint32(0);	// unknown
	mMessageFactory->addUint32(0);	// unknown
	
	mMessageFactory->addUint32(0);	// unknown
	mMessageFactory->addUint32(0);	// unknown
	
	data = mMessageFactory->EndMessage();

	mMessageFactory->StartMessage();  
	mMessageFactory->addUint32(opBaselinesMessage);   
	mMessageFactory->addUint64(crate->getId()); 
	mMessageFactory->addUint32(opFCYT);
	mMessageFactory->addUint8(9);  

	mMessageFactory->addUint32(data->getSize());
	mMessageFactory->addData(data->getData(), data->getSize());
	
	data->setPendingDelete(true);


	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}


bool MessageLib::sendUpdateCrateContent(FactoryCrate* crate,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);


	uint32 amount = 0;
	if(crate->hasAttribute("factory_count"))
	{
		amount = crate->getAttribute<int>("factory_count");
	}

		   
	Message* newMessage;

	mMessageFactory->StartMessage();  
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(crate->getId());
	mMessageFactory->addUint32(opFCYT);
	mMessageFactory->addUint8(3);

	mMessageFactory->addUint32(8);
	mMessageFactory->addUint16(1);

	mMessageFactory->addUint16(7);
	mMessageFactory->addUint32(amount);

	newMessage = mMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage,playerObject->getAccountId(),CR_Client,5);

	return(true);
}
