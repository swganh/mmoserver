/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"
#include "LogManager/LogManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ObjectFactory.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"
#include "ZoneServer/CurrentResource.h"
#include "ZoneServer/ResourceManager.h"
#include <boost/lexical_cast.hpp>
#include "ZoneServer/ResourceType.h"

//======================================================================================================================
//
// Resource Container Baselines Type 3
// contain: name,type,amount
//

bool MessageLib::sendBaselinesRCNO_3(ResourceContainer* resourceContainer,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message*	message;
	string		resourceNameFile = "kb/kb_resources_n";
	string		resourceTypeName = ((resourceContainer->getResource())->getType())->getTypeName();

	gMessageFactory->StartMessage();       
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(resourceContainer->getId()); 
	gMessageFactory->addUint32(opRCNO);
	gMessageFactory->addUint8(3);  

	gMessageFactory->addUint32(61 + resourceNameFile.getLength() + resourceTypeName.getLength());
	gMessageFactory->addUint16(13);	
	gMessageFactory->addFloat(0);	
	gMessageFactory->addString(resourceNameFile);
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addString(resourceTypeName);

	gMessageFactory->addUint32(0);//customname
	gMessageFactory->addUint32(1);	// volume

	gMessageFactory->addUint16(0);//ascii customization

	// unknown list			   probably inherited from tano the customization crc of special attachments like scope etc
	// so most likely not related to resources
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);

	gMessageFactory->addUint8(0);//OptionsBitmask figure the bitmap out!!!!! 4 is insured
	gMessageFactory->addUint8(0);//OptionsBitmask figure the bitmap out!!!!!
	gMessageFactory->addUint8(0);//OptionsBitmask figure the bitmap out!!!!!
	gMessageFactory->addUint8(0);//OptionsBitmask figure the bitmap out!!!!!
	//4 is insured

	//timer incap (unused)
	gMessageFactory->addUint32(0);	// unknown
	//cond damage
	gMessageFactory->addUint32(0);
	//max condition
	gMessageFactory->addUint32(100);

	//unknown - bitmask?????
	gMessageFactory->addUint8(0);
	gMessageFactory->addUint32(resourceContainer->getAmount());
	gMessageFactory->addUint64((resourceContainer->getResource())->getId());
	
	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	Resource* resource = gResourceManager->getResourceById((resourceContainer->getResource())->getId());

	if(resource != NULL)
	{
		// this info gets automatically requested by the client every time we create the resource from scratch
		// it will NOT be requested when we load the resource on login / inventory creation
	    // this makes me think whether we do something wrong there ???
		//should we end the inventory baseline and THEN send the inventory content ????
		// however this will fix the greenbar problem while crafting !!!
		resource->sendAttributes(targetObject);
	}

	return(true);
}

//======================================================================================================================
//
// Resource Container Baselines Type 6
// contain: type description, max amount
//

bool MessageLib::sendBaselinesRCNO_6(ResourceContainer* resourceContainer,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message*	message;
	string		unknownStr = "";
	string		resourceTypeDescriptor = (((resourceContainer->getResource())->getType())->getDescriptor());
	string		resourceName = (resourceContainer->getResource())->getName().getAnsi();
	resourceName.convert(BSTRType_Unicode16);

	gMessageFactory->StartMessage();        
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(resourceContainer->getId()); 
	gMessageFactory->addUint32(opRCNO);
	gMessageFactory->addUint8(6);  

	gMessageFactory->addUint32(32 + (unknownStr.getLength() << 1) + resourceTypeDescriptor.getLength() + (resourceName.getLength() << 1));
	gMessageFactory->addUint16(5);	
	gMessageFactory->addFloat(1.0);	// unknown
	gMessageFactory->addString(unknownStr);
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(unknownStr);
	gMessageFactory->addUint32(100000); // max stack size ?
	gMessageFactory->addString(resourceTypeDescriptor);
	gMessageFactory->addString(resourceName);
	

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Resource Container Baselines Type 8
// contain: unknown
//

bool MessageLib::sendBaselinesRCNO_8(ResourceContainer* resourceContainer,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message*	message;

	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(resourceContainer->getId()); 
	gMessageFactory->addUint32(opRCNO);
	gMessageFactory->addUint8(8);  

	gMessageFactory->addUint32(2);
	gMessageFactory->addUint16(0);	

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Resource Container Baselines Type 9
// contain: unknown
//

bool MessageLib::sendBaselinesRCNO_9(ResourceContainer* resourceContainer,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message*	message;

	gMessageFactory->StartMessage();         
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(resourceContainer->getId()); 
	gMessageFactory->addUint32(opRCNO);
	gMessageFactory->addUint8(9);  

	gMessageFactory->addUint32(2);
	gMessageFactory->addUint16(0);	

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Resource Container Deltas Type 3
// update: amount
//

bool MessageLib::sendResourceContainerUpdateAmount(ResourceContainer* resourceContainer,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(resourceContainer->getId());          
	gMessageFactory->addUint32(opRCNO);           
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(8);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(11);
	gMessageFactory->addUint32(resourceContainer->getAmount());

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

