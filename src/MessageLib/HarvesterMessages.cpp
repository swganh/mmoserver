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
#include "ZoneServer/ObjectControllerOpcodes.h"

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
	gMessageFactory->addUint32(harvester->getDamage());//condition damage
	gMessageFactory->addUint32(harvester->getMaxCondition());   //maxcondition
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(harvester->getActive());//active flag
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

//======================================================================================================================
//
// Building Baselines Type 7
// contain: unknown
//

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

	gMessageFactory->addUint64(harvester->getCurrentResource());//current Res Id harvesting

	gMessageFactory->addUint8(harvester->getActive());//on off status flag
	gMessageFactory->addUint32(uint32(harvester->getSpecExtraction()));//hopper capacity
	
	gMessageFactory->addFloat(harvester->getSpecExtraction());//spec rate
	gMessageFactory->addFloat(harvester->getCurrentExtractionRate());//current rate

	gMessageFactory->addFloat(harvester->getCurrentHopperSize());//current hopper size
	gMessageFactory->addUint32((uint32)harvester->getHopperSize());//max Hoppersize

	gMessageFactory->addUint8(1);//	  hopper update flag


	HResourceList* rList = harvester->getResourceList();
	harvester->setRListUpdateCounter(rList->size());

	gMessageFactory->addUint32(rList->size());//listsize hopper contents
	gMessageFactory->addUint32(harvester->getRListUpdateCounter());//updatecounter hopper contents

	//harvester->setRListUpdateCounter(harvester->getRListUpdateCounter()+rList->size());
	
	HResourceList::iterator it = rList->begin();
	while (it != rList->end())
	{
		gMessageFactory->addUint64((*it).first);//
		gMessageFactory->addFloat((*it).second);//
		it++;
	}
	
	uint8 condition = ((harvester->getMaxCondition()-harvester->getDamage())/(harvester->getMaxCondition()/100));
	gMessageFactory->addUint8(condition);//	  condition

	gMessageFactory->addUint64(0);//
	
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
		{
			ratio	= (cR->getDistribution((int)posX + 8192,(int)posZ + 8192)*100);
			if(ratio > 100)
			{
				ratio = 100;
			}
		}

		gMessageFactory->addUint8((uint8)ratio);

		++resourceIt;
	}

	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);
}

//======================================================================================================================
//send current Resource Update
//======================================================================================================================

void MessageLib::sendCurrentResourceUpdate(HarvesterObject* harvester, PlayerObject* player)
{										  
	gMessageFactory->StartMessage();
	
	gMessageFactory->addUint16(1);	//1 updated var
	gMessageFactory->addUint16(5);	//var Nr 5
	gMessageFactory->addUint64(harvester->getCurrentResource());
	Message* fragment = gMessageFactory->EndMessage();

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(harvester->getId());
	gMessageFactory->addUint32(opHINO);
	gMessageFactory->addUint8(7);
	gMessageFactory->addUint32(fragment->getSize());
	gMessageFactory->addData(fragment->getData(),fragment->getSize());

	fragment->setPendingDelete(true);

	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);
}

//======================================================================================================================
//send current ExtractionRate update
//======================================================================================================================

void MessageLib::sendCurrentExtractionRate(HarvesterObject* harvester, PlayerObject* player)
{										  
	gMessageFactory->StartMessage();
	
	gMessageFactory->addUint16(1);	//1 updated var
	gMessageFactory->addUint16(9);	//var Nr 9
	gMessageFactory->addFloat(harvester->getCurrentExtractionRate());
	Message* fragment = gMessageFactory->EndMessage();

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(harvester->getId());
	gMessageFactory->addUint32(opHINO);
	gMessageFactory->addUint8(7);
	gMessageFactory->addUint32(fragment->getSize());
	gMessageFactory->addData(fragment->getData(),fragment->getSize());

	fragment->setPendingDelete(true);

	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);
}

//======================================================================================================================
//turn the harvester on
//======================================================================================================================

void MessageLib::sendHarvesterActive(HarvesterObject* harvester)
{										  
	gMessageFactory->StartMessage();
	
	gMessageFactory->addUint16(1);	//1 updated var
	gMessageFactory->addUint16(6);	//var Nr 9
	gMessageFactory->addUint8(harvester->getActive());
	Message* fragment = gMessageFactory->EndMessage();

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(harvester->getId());
	gMessageFactory->addUint32(opHINO);
	gMessageFactory->addUint8(7);
	gMessageFactory->addUint32(fragment->getSize());
	gMessageFactory->addData(fragment->getData(),fragment->getSize());

	fragment->setPendingDelete(true);

	_sendToInRange(gMessageFactory->EndMessage(),harvester,5);
	//(player->getClient())->SendChannelA(gMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);
	
}

//======================================================================================================================
//Notifies the chatserver to
//the chatserver will
//======================================================================================================================

void MessageLib::sendHarvesterChatHopperUpdate(HarvesterObject* harvester, PlayerObject* player, uint8 update)
{										  
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmHarvesterUpdate);
	gMessageFactory->addUint64(harvester->getId());
	gMessageFactory->addUint8(update);
	gMessageFactory->addUint32(harvester->getRListUpdateCounter());

	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(), player->getAccountId(),CR_Chat,4);
}

//=======================================================================================================================
//
// sends the relevant delta to the client to update hopper contents
//

void MessageLib::SendHarvesterHopperUpdate(HarvesterObject* harvester, PlayerObject* player)
{			
																		 
	gMessageFactory->StartMessage();
	
	gMessageFactory->addUint16(3);	//2 updated vars
	gMessageFactory->addUint16(12);	//var Nr 12 = hopper update flag
	gMessageFactory->addUint8(1);

	gMessageFactory->addUint16(13);	//var Nr 12 = hopper resourcelist
	
	HResourceList*	hRList = harvester->getResourceList();
	harvester->setRListUpdateCounter(harvester->getRListUpdateCounter() + hRList->size());

	gLogger->logMsgF("adding update Counter  ID %u",MSG_HIGH,harvester->getRListUpdateCounter());

	gMessageFactory->addUint32(hRList->size());
	gMessageFactory->addUint32(harvester->getRListUpdateCounter());

	//gLogger->logMsgF("SendHarvesterHopperUpdate:: listsize %u updatecounter %u",MSG_NORMAL,mHopperList->size(),asynContainer->updateCounter);

	gMessageFactory->addUint8(3);
	gMessageFactory->addUint16(hRList->size());

	HResourceList::iterator it = hRList->begin();

	while(it != hRList->end())
	{
		gMessageFactory->addUint64((*it).first);		
		//gLogger->logMsgF("adding res ID %I64u",MSG_HIGH,(*it).first);
		gMessageFactory->addFloat((*it).second);		
		//gMessageFactory->addFloat((float)harvester->getRListUpdateCounter());	
		it++;
	}

	gMessageFactory->addUint16(10);	//var Nr 12 = hopper update flag
	gMessageFactory->addFloat(harvester->getCurrentHopperSize());
	
	Message* fragment = gMessageFactory->EndMessage();

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opDeltasMessage);
	gMessageFactory->addUint64(harvester->getId());
	gMessageFactory->addUint32(opHINO);
	gMessageFactory->addUint8(7);
	gMessageFactory->addUint32(fragment->getSize());
	gMessageFactory->addData(fragment->getData(),fragment->getSize());

	fragment->setPendingDelete(true);
	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);

}


//======================================================================================================================
//
// sends the Hopper List for a structure
//

bool MessageLib::sendHopperList(PlayerStructure* structure, PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opSendPermissionList);  
	gMessageFactory->addUint32(structure->getStrucureHopperList().size() );

	string name;
	BStringVector vector = 	structure->getStrucureHopperList();
	BStringVector::iterator it = vector.begin();
	while(it != vector.end())
	{
		name = (*it);
		name.convert(BSTRType_Unicode16);
		gMessageFactory->addString(name);

		it++;
	}

	gMessageFactory->addUint32(0); // ???
	//gMessageFactory->addUint16(0);	// unknown
	name = "HOPPER";
	name.convert(BSTRType_Unicode16);
	gMessageFactory->addString(name);
	gMessageFactory->addUint32(0); // ???
	
	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 5);

	structure->resetStructureHopperList();

	return(true);
}


//======================================================================================================================
// 
//======================================================================================================================

void MessageLib::sendResourceEmptyHopperResponse(PlayerStructure* structure,PlayerObject* player, uint32 amount, uint8 b1, uint8 b2)
{
	HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(structure);
	if(!harvester)
	{
		return;
	}

	gMessageFactory->StartMessage();        
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opResourceEmptyHopperResponse);           
	gMessageFactory->addUint64(player->getId());
	gMessageFactory->addUint32(amount);           
	gMessageFactory->addUint32(opResourceEmptyHopper);
	gMessageFactory->addUint8(b1);           
	gMessageFactory->addUint8(b2);           
	
	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(), player->getAccountId(),CR_Client,4);
}
