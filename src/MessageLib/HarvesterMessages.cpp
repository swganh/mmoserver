		   /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"

#include "ZoneServer/Deed.h"
#include "ZoneServer/ResourceCategory.h"
#include "ZoneServer/ResourceManager.h"
#include "ZoneServer/ResourceType.h"

#include "ZoneServer/HarvesterObject.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/ObjectFactory.h"
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

bool MessageLib::sendBaselinesHINO_3(HarvesterObject* harvester,PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();    
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(harvester->getId());
	gMessageFactory->addUint32(opHINO);
	gMessageFactory->addUint8(3);

	uint32 byteCount = 59 + harvester->getNameFile().getLength() + harvester->getName().getLength()+(harvester->getCustomName().getLength()*2);
	gMessageFactory->addUint32(byteCount);
	gMessageFactory->addUint16(16);
	gMessageFactory->addFloat(1.0);
	gMessageFactory->addString(harvester->getNameFile());
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(harvester->getName());
	
	string name;
	name = harvester->getCustomName();
	name.convert(BSTRType_Unicode16);

	gMessageFactory->addString(name.getUnicode16());
	
	gMessageFactory->addUint32(1);//volume (in inventory)
	gMessageFactory->addUint16(0);//customization
	gMessageFactory->addUint32(0);//list
	gMessageFactory->addUint32(0);//list
	gMessageFactory->addUint32(0);//optionsbitmask
	gMessageFactory->addUint32(0);//timer
	gMessageFactory->addUint32(0);//condition damage
	gMessageFactory->addUint32(1000);   //maxcondition
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(0);//active flag
	gMessageFactory->addFloat(0);//power reserve
	gMessageFactory->addFloat(0);//power cost
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);

	newMessage = gMessageFactory->EndMessage();

	(player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Building Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesHINO_6(HarvesterObject* harvester,PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(harvester->getId());
	gMessageFactory->addUint32(opHINO);
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(14);
	gMessageFactory->addUint16(2);	// unknown
	gMessageFactory->addUint16(0); // unknown
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);

	newMessage = gMessageFactory->EndMessage();

	(player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

	return(true);
}

bool MessageLib::sendBaselinesHINO_7(HarvesterObject* harvester,PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	ResourceCategory*	mainCat = gResourceManager->getResourceCategoryById(harvester->getResourceCategory());
	ResourceList		resourceList;

	mainCat->getResources(resourceList,true);

	Message* newMessage;
	
	gMessageFactory->StartMessage();
	gMessageFactory->addUint8(15);
	gMessageFactory->addUint8(0);
	gMessageFactory->addUint8(1);	// 

	
	//=====================================
	//start with the resource IDS
	harvester->setUpdateCounter(resourceList.size());

	gMessageFactory->addUint32(resourceList.size());
	gMessageFactory->addUint32(harvester->getUpdateCounter());

	ResourceList::iterator resourceIt = resourceList.begin();

	while(resourceIt != resourceList.end())
	{
		Resource* tmpResource = (*resourceIt);

		gMessageFactory->addUint64(tmpResource->getId());

		++resourceIt;
	}


	//=====================================
	//resource IDS a second time ... ???
	harvester->setUpdateCounter(resourceList.size());

	gMessageFactory->addUint32(resourceList.size());
	gMessageFactory->addUint32(harvester->getUpdateCounter());

	resourceIt = resourceList.begin();

	while(resourceIt != resourceList.end())
	{
		Resource* tmpResource = (*resourceIt);

		gMessageFactory->addUint64(tmpResource->getId());

		++resourceIt;
	}



	//=====================================
	//resource names
	gMessageFactory->addUint32(resourceList.size());
	gMessageFactory->addUint32(harvester->getUpdateCounter());
	
	resourceIt = resourceList.begin();

	while(resourceIt != resourceList.end())
	{
		Resource* tmpResource = (*resourceIt);
		gMessageFactory->addString(tmpResource->getName().getAnsi());
		
		++resourceIt;
	}

	

	//=====================================
	//resource types
	gMessageFactory->addUint32(resourceList.size());
	gMessageFactory->addUint32(harvester->getUpdateCounter());

	resourceIt = resourceList.begin();

	while(resourceIt != resourceList.end())
	{
		Resource* tmpResource = (*resourceIt);	
		gMessageFactory->addString((tmpResource->getType())->getDescriptor().getAnsi());

		++resourceIt;
	}

	gMessageFactory->addUint64(0);//current Res Id harvesting

	gMessageFactory->addUint8(0);//on off status flag
	gMessageFactory->addFloat(0);//hopper capacity
	
	gMessageFactory->addFloat(0);//spec rate
	gMessageFactory->addFloat(0);//current rate

	gMessageFactory->addFloat(0);//hopper amount
	gMessageFactory->addFloat(0);//hopper size

	gMessageFactory->addUint8(0);//
	
	gMessageFactory->addFloat(0);//
	gMessageFactory->addFloat(0);//
		
	gMessageFactory->addFloat(0);//
	gMessageFactory->addFloat(0);//
	gMessageFactory->addFloat(0);//
	gMessageFactory->addFloat(0);//
	gMessageFactory->addFloat(0);//
	gMessageFactory->addFloat(0);//
	gMessageFactory->addFloat(0);//
	gMessageFactory->addFloat(0);//

	gMessageFactory->addUint64(0);//current Res Id harvesting
	gMessageFactory->addUint64(0);//current Res Id harvesting
	gMessageFactory->addUint64(0);//current Res Id harvesting
	gMessageFactory->addUint64(0);//current Res Id harvesting
	
	newMessage = gMessageFactory->EndMessage();


	//now add the data to the Baselines header
	Message* completeMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(harvester->getId());
	gMessageFactory->addUint32(opHINO);
	gMessageFactory->addUint8(7);

	gMessageFactory->addUint32(newMessage->getSize());//ByteCount
	gMessageFactory->addData(newMessage->getData(),newMessage->getSize());
	completeMessage = gMessageFactory->EndMessage();


	//Important! -> never leave a message undeleted
	newMessage->setPendingDelete(true);

	(player->getClient())->SendChannelA(completeMessage, player->getAccountId(), CR_Client, 5);
	harvester->setUpdateCounter(harvester->getUpdateCounter()+1);

	return(true);
}


//======================================================================================================================
//
// Building Baselines Type 3
// contain: name,
//

bool MessageLib::sendBaselinesINSO_3(PlayerStructure* structure,PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();    
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(structure->getId());
	gMessageFactory->addUint32(opINSO);
	gMessageFactory->addUint8(3);

	structure->getNameFile().convert(BSTRType_ANSI);
	structure->getName().convert(BSTRType_ANSI);

	printf("%s", structure->getNameFile().getAnsi());
	printf("%s", structure->getName().getAnsi());

	uint32 byteCount = 61 + structure->getNameFile().getLength() + structure->getName().getLength();
	
	gMessageFactory->addUint32(byteCount);
	gMessageFactory->addUint16(10);
	gMessageFactory->addFloat(1.0);
	gMessageFactory->addString(structure->getNameFile());
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(structure->getName());
	gMessageFactory->addUint32(0);
	//gMessageFactory->addString("");
	
	gMessageFactory->addUint32(1);//volume (in inventory)
	gMessageFactory->addUint16(0);//customization
	gMessageFactory->addUint32(0);//list
	gMessageFactory->addUint32(0);//list
	gMessageFactory->addUint32(0);//optionsbitmask
	gMessageFactory->addUint32(0);//timer
	gMessageFactory->addUint32(0);//condition damage
	gMessageFactory->addUint32(0);   //maxcondition
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(0);//active flag
	gMessageFactory->addFloat(0);//power reserve
	gMessageFactory->addFloat(0);//power cost
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);

	newMessage = gMessageFactory->EndMessage();

	(player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Installation Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesINSO_6(PlayerStructure* structure,PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(structure->getId());
	gMessageFactory->addUint32(opINSO);
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(43);
	gMessageFactory->addUint16(0);	// unknown
	gMessageFactory->addUint16(0); // unknown
	gMessageFactory->addUint32(0);	// unknown
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint64(0);

	newMessage = gMessageFactory->EndMessage();

	(player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

	return(true);
}


//======================================================================================================================
//send Harvester Name update
//======================================================================================================================

void MessageLib::sendNewHarvesterName(PlayerStructure* harvester)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(harvester->getId());
	gMessageFactory->addUint32(opHINO);
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(8 + (harvester->getCustomName().getLength()*2));
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(2);
	//Unicode
	string name;
	name = harvester->getCustomName();
	name.convert(BSTRType_Unicode16);

	gMessageFactory->addString(name.getUnicode16());

	_sendToInRange(gMessageFactory->EndMessage(),harvester,5);
}

//======================================================================================================================
//send Harvester Name update
//======================================================================================================================

void MessageLib::sendOperateHarvester(PlayerStructure* harvester,PlayerObject* player)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opOperateHarvester);
	gMessageFactory->addUint64(harvester->getId());
	

	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(), player->getAccountId(), CR_Client, 5);
	//_sendToInRange(gMessageFactory->EndMessage(),harvester,5);
}

//======================================================================================================================
//send Harvester ResourceData
//======================================================================================================================

void MessageLib::sendHarvesterResourceData(PlayerStructure* structure,PlayerObject* player)
{
	HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);
	if(!harvester)
	{
		return;
	}

	ResourceCategory*	mainCat = gResourceManager->getResourceCategoryById(harvester->getResourceCategory());
	ResourceList		resourceList;

	float posX, posZ, ratio;

	posX	= harvester->mPosition.mX;
	posZ	= harvester->mPosition.mZ;
	
	mainCat->getResources(resourceList,true);

	gMessageFactory->StartMessage();        
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opHarvesterResourceData);           
	gMessageFactory->addUint64(player->getId());
	gMessageFactory->addUint32(0);           
	gMessageFactory->addUint64(harvester->getId());

	gMessageFactory->addUint32(resourceList.size());

	ResourceList::iterator resourceIt = resourceList.begin();

	while(resourceIt != resourceList.end())
	{
		Resource* tmpResource = (*resourceIt);

		gMessageFactory->addUint64(tmpResource->getId());
		gMessageFactory->addString(tmpResource->getName());
		gMessageFactory->addString((tmpResource->getType())->getDescriptor());
	
		CurrentResource* cR = reinterpret_cast<CurrentResource*>(tmpResource);
		//resource = reinterpret_cast<CurrentResource*>(gResourceManager->getResourceByNameCRC(resourceName.getCrc()));
		if(!cR)
		{
			ratio = 0;
		}
		else
			ratio	= cR->getDistribution((int)posX + 8192,(int)posZ + 8192);

		gMessageFactory->addUint8((uint8)ratio);

		++resourceIt;
	}

	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);
}