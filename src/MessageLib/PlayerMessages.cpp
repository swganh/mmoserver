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
#include "ZoneServer/UIOpcodes.h"
#include "ZoneServer/ObjectFactory.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"
#include "ZoneServer/ObjectControllerOpcodes.h"
#include "ZoneServer/CurrentResource.h"
#include "ZoneServer/CharSheetManager.h"
#include "ZoneServer/Wearable.h"
#include "ZoneServer/Conversation.h"
#include "ZoneServer/NPCObject.h"
#include "ZoneServer/CraftingTool.h"
#include "ZoneServer/ManufacturingSchematic.h"
#include <boost/lexical_cast.hpp>
#include "Common/atMacroString.h"


//======================================================================================================================
//
// Player Baselines Type 3
// contain: title, flags(afk,..), matchmaking, borndate, playtime
//

bool MessageLib::sendBaselinesPLAY_3(PlayerObject* playerObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage(); 
	gMessageFactory->addUint32(opBaselinesMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId()); 
	gMessageFactory->addUint32(opPLAY);
	gMessageFactory->addUint8(3);  

	gMessageFactory->addUint32(95 + playerObject->getTitle().getLength());

	gMessageFactory->addUint16(0x000b); // unknown
	gMessageFactory->addUint32(0x3f800000); // complexity
	gMessageFactory->addString(BString("string_id_table"));//stf file
	gMessageFactory->addUint32(0); // unknown
	gMessageFactory->addUint16(0); // stf name

	gMessageFactory->addUint32(0); // custom name
	gMessageFactory->addUint32(0); // volume
	gMessageFactory->addUint32(0); // generic int (not used)	

	// player flags
	gMessageFactory->addUint32(4); //flaglist size
	gMessageFactory->addUint32(playerObject->getPlayerFlags()); 
	gMessageFactory->addUint32(0); 
	gMessageFactory->addUint32(0); 
	gMessageFactory->addUint32(0); 

	// profile - matchmaking
	gMessageFactory->addUint32(4); // flaglist size
	gMessageFactory->addUint32(playerObject->getPlayerMatch(0)); 
	gMessageFactory->addUint32(playerObject->getPlayerMatch(1));
	gMessageFactory->addUint32(playerObject->getPlayerMatch(2)); 
	gMessageFactory->addUint32(playerObject->getPlayerMatch(3)); 

	gMessageFactory->addString(playerObject->getTitle());

	// birthdate + totalplaytime
	gMessageFactory->addUint32(playerObject->getBornyear()); // Born year
	gMessageFactory->addUint32(0x000018d8); // Total Playtime in seconds
	gMessageFactory->addUint32(0);

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Player Baselines Type 6
// contain: subzone region_id, csr / developer tag
//

bool MessageLib::sendBaselinesPLAY_6(PlayerObject* playerObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(playerObject->getPlayerObjId()); 
	gMessageFactory->addUint32(opPLAY);
	gMessageFactory->addUint8(6);  

	gMessageFactory->addUint32(7);
	gMessageFactory->addUint16(2); 

	gMessageFactory->addUint32(playerObject->getSubZoneId()); 
	gMessageFactory->addUint8(playerObject->getCsrTag());

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Player Baselines Type 8
// contain: xp, waypoints, forcebar, quests
//

bool MessageLib::sendBaselinesPLAY_8(PlayerObject* playerObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	XPList* xpList = playerObject->getXpList();

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(playerObject->getPlayerObjId()); 
	gMessageFactory->addUint32(opPLAY);
	gMessageFactory->addUint8(8);  

	// xp list size
	uint32 xpByteCount = 0;
	XPList::iterator xpIt = xpList->begin();
	
	uint32 xpListSize = 0;
	while(xpIt != xpList->end())
	{
		//if ((*xpIt).second > 0)	// Only add xptypes that we actually have any xp from.
		//{
			xpByteCount += ((gSkillManager->getXPTypeById((*xpIt).first)).getLength() + 7); // strlen + value + delimiter
			xpListSize++;
		//}
		++xpIt;
	}

	// waypoint list size
	uint32					waypointsByteCount	= 0;
	Datapad*				datapad				= dynamic_cast<Datapad*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));
	WaypointList*			waypointList		= datapad->getWaypoints();
	WaypointList::iterator	waypointIt			= waypointList->begin();

	while(waypointIt != waypointList->end())
	{
		waypointsByteCount += (51 + ((*waypointIt)->getName().getLength() *2));
		++waypointIt;
	}

	gMessageFactory->addUint32(76 + xpByteCount + waypointsByteCount);
	gMessageFactory->addUint16(7);

	// xp list
	// gMessageFactory->addUint32(xpList->size());
	gMessageFactory->addUint32(xpListSize);
	gMessageFactory->addUint32(playerObject->mXpUpdateCounter);

	xpIt = xpList->begin();

	while(xpIt != xpList->end())
	{
		//if ((*xpIt).second > 0)	// Only add xptypes that we actually have xp from.
		//{
			gMessageFactory->addUint8(0);
			gMessageFactory->addString(gSkillManager->getXPTypeById((*xpIt).first)); 
			gMessageFactory->addInt32((*xpIt).second);
		//}
		++xpIt;
	}

	// waypoint list
	gMessageFactory->addUint32(waypointList->size());
	gMessageFactory->addUint32(datapad->mWaypointUpdateCounter);

	waypointIt = waypointList->begin();

	
	while(waypointIt != waypointList->end())
	{
		WaypointObject* waypoint = (*waypointIt);
		
		gMessageFactory->addUint8(2);
		gMessageFactory->addUint64(waypoint->getId());
		gMessageFactory->addUint32(0);
		gMessageFactory->addFloat(waypoint->getCoords().mX);
		gMessageFactory->addFloat(waypoint->getCoords().mY);
		gMessageFactory->addFloat(waypoint->getCoords().mZ);
		gMessageFactory->addUint64(0);
		gMessageFactory->addUint32(waypoint->getPlanetCRC());//planetcrc
		gMessageFactory->addString(waypoint->getName());
		gMessageFactory->addUint64(waypoint->getId());
		gMessageFactory->addUint8(waypoint->getWPType());
		if(waypoint->getActive())
			gMessageFactory->addUint8(1);
		else
			gMessageFactory->addUint8(0);

		++waypointIt;
	}

	// current force
	gMessageFactory->addUint32(playerObject->getHam()->getCurrentForce());

	// max force
	gMessageFactory->addUint32(playerObject->getHam()->getMaxForce());

	// unknown
	gMessageFactory->addUint32(5);
	gMessageFactory->addUint64(20);
	gMessageFactory->addUint8(0);
	gMessageFactory->addUint32(5);
	gMessageFactory->addUint64(14);
	gMessageFactory->addUint8(0);

	// quest list
	gMessageFactory->addUint64(0);

	gMessageFactory->addUint32(0);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint16(0);

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Player Baselines Type 9
// contain: skillcommands, crafting info, draftschematics, stomach, language, friend/ignore lists
//

bool MessageLib::sendBaselinesPLAY_9(PlayerObject* playerObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	SkillCommandList*		scList		= playerObject->getSkillCommands();
	SchematicsIdList*		schemIdList = playerObject->getSchematicsIdList();

	gMessageFactory->StartMessage();         
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(playerObject->getPlayerObjId()); 
	gMessageFactory->addUint32(opPLAY);
	gMessageFactory->addUint8(9);  

	// get skillcmds bytecount
	uint32 skillCommandsByteCount = 0;
	SkillCommandList::iterator scIt = scList->begin();

	while(scIt != scList->end())
	{
		skillCommandsByteCount += ((gSkillManager->getSkillCmdById(*scIt)).getLength() + 2);
		++scIt;
	}

	// get schematics bytecount
	uint32 schematicsByteCount = schemIdList->size() * 8;

	gMessageFactory->addUint32(98 + skillCommandsByteCount + schematicsByteCount); 
	gMessageFactory->addUint16(19); 

	// skill commands
	gMessageFactory->addUint32(scList->size()); 
	gMessageFactory->addUint32(playerObject->mSkillCmdUpdateCounter);

	scIt = scList->begin();

	while(scIt != scList->end())
	{
		gMessageFactory->addString(gSkillManager->getSkillCmdById(*scIt));
		++scIt;
	}

	// crafting experimentation
	gMessageFactory->addUint32(0);

	// crafting stage
	gMessageFactory->addUint32(0); 

	// nearest crafting station
	gMessageFactory->addUint64(0);

	// draft schematics

	
	Datapad* datapad = dynamic_cast<Datapad*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));
	
	//gLogger->logMsgF("Yalp9 Baseline Schematicslist::Listsize %u",MSG_HIGH,schemIdList->size());
	
	gMessageFactory->addUint32(schemIdList->size());
	
	datapad->mSchematicUpdateCounter += schemIdList->size();
	gMessageFactory->addUint32(datapad->mSchematicUpdateCounter);
	
	//gLogger->logMsgF("Yalp9 Baseline Schematicslist::UpdateCounter %u",MSG_HIGH,datapad->mSchematicUpdateCounter);

	SchematicsIdList::iterator schemIt = schemIdList->begin();

	while(schemIt != schemIdList->end())
	{
		gMessageFactory->addUint64(*schemIt);
		++schemIt;
	}
	
	
	// experimentation points
	gMessageFactory->addUint32(0);

	// accomplishment counter
	gMessageFactory->addUint32(0);


	// contact lists: these are always 0 here, will get updated by seperate deltas messages
	// after object initialization is complete

	// friends list
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	// ignores list
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);


	// current language
	gMessageFactory->addUint32(playerObject->getLanguage()); 
	// 1 = basic

	// stomach
	Stomach* stomach = playerObject->getStomach();

	gMessageFactory->addUint32(stomach->getFood()); 
	gMessageFactory->addUint32(stomach->getFoodMax()); 
	gMessageFactory->addUint32(stomach->getDrink()); 
	gMessageFactory->addUint32(stomach->getDrinkMax()); 

	// consumeables
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);

	// unused waypoints list
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);

	// jedi state
	gMessageFactory->addUint32(playerObject->getJediState());

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: Stomach
//

void MessageLib::sendFoodUpdate(PlayerObject* playerObject)
{
	// stomach
	Stomach* stomach = playerObject->getStomach();

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(9);

	gMessageFactory->addUint32(8);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(0x0a);
	gMessageFactory->addUint32(stomach->getFood());
	
	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: Drink
//

void MessageLib::sendDrinkUpdate(PlayerObject* playerObject)
{
	// stomach
	Stomach* stomach = playerObject->getStomach();

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(9);

	gMessageFactory->addUint32(8);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(0x0c);
	gMessageFactory->addUint32(stomach->getDrink());
	
	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);
}

//======================================================================================================================
//
// Player Deltas Type 3
// update: title
//

void MessageLib::sendTitleUpdate(PlayerObject* playerObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(6 + playerObject->getTitle().getLength());
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(7);
	gMessageFactory->addString(playerObject->getTitle());

	_sendToInRange(gMessageFactory->EndMessage(),playerObject,5);
}

//======================================================================================================================
//
// Player Deltas Type 3
// update: flags
//

void MessageLib::sendUpdatePlayerFlags(PlayerObject* playerObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(24);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(5);

	gMessageFactory->addUint32(4);
	gMessageFactory->addUint32(playerObject->getPlayerFlags());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);

	_sendToInRange(gMessageFactory->EndMessage(),playerObject,5);
}

//======================================================================================================================
//
// Player Deltas Type 8
// update: waypoints
//

bool MessageLib::sendWaypointsUpdate(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	Datapad* datapad = dynamic_cast<Datapad*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(8);

	uint32					waypointsByteCount	= 0;
	WaypointList*			waypointList		= datapad->getWaypoints();
	WaypointList::iterator	waypointIt			= waypointList->begin();

	while(waypointIt != waypointList->end())
	{
		waypointsByteCount += (51 + ((*waypointIt)->getName().getLength() << 1));
		++waypointIt;
	}

	gMessageFactory->addUint32(12 + waypointsByteCount);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(1);

	// waypoint list
	gMessageFactory->addUint32(waypointList->size());

	datapad->mWaypointUpdateCounter += waypointList->size();
	gMessageFactory->addUint32(datapad->mWaypointUpdateCounter);

	waypointIt = waypointList->begin();

	while(waypointIt != waypointList->end())
	{
		WaypointObject* waypoint = (*waypointIt);

		gMessageFactory->addUint8(0);
		gMessageFactory->addUint64(waypoint->getId());
		gMessageFactory->addUint32(1); //perhaps volume ???
		gMessageFactory->addFloat(waypoint->getCoords().mX);
		gMessageFactory->addFloat(waypoint->getCoords().mY);
		gMessageFactory->addFloat(waypoint->getCoords().mZ);
		gMessageFactory->addUint64(0);
		gMessageFactory->addUint32(waypoint->getModelString().getCrc());
		gMessageFactory->addString(waypoint->getName());
		gMessageFactory->addUint64(waypoint->getId());
		gMessageFactory->addUint8(waypoint->getWPType());
		gMessageFactory->addUint8((uint8)waypoint->getActive());

		++waypointIt;
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 8
// update: waypoints
//

bool MessageLib::sendUpdateWaypoint(WaypointObject* waypoint,ObjectUpdate updateType,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	uint8 type = 0xFF;
	if(updateType == ObjectUpdateAdd)
		type = 0;

	if(updateType == ObjectUpdateDelete)
		type = 1;

	if(updateType == ObjectUpdateChange)
		type = 2;

	if(type == 0xFF)
		return false;

	Datapad* datapad = dynamic_cast<Datapad*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(8);


	gMessageFactory->addUint32(63 + (waypoint->getName().getLength() << 1));
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(1);

	// elements
	gMessageFactory->addUint32(1);
	gMessageFactory->addUint32(++datapad->mWaypointUpdateCounter);

	gMessageFactory->addUint8(type);
	gMessageFactory->addUint64(waypoint->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addFloat(waypoint->getCoords().mX);
	gMessageFactory->addFloat(waypoint->getCoords().mY);
	gMessageFactory->addFloat(waypoint->getCoords().mZ);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint32(waypoint->getModelString().getCrc());
	gMessageFactory->addString(waypoint->getName());
	gMessageFactory->addUint64(waypoint->getId());
	gMessageFactory->addUint8(waypoint->getWPType());
	gMessageFactory->addUint8((uint8)waypoint->getActive());

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: skill commands
//

bool MessageLib::sendSkillCmdDeltasPLAY_9(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	// get skillcmds bytecount
	SkillCommandList*		scList					= playerObject->getSkillCommands();
	uint32					skillCommandsByteCount	= 0;
	SkillCommandList::iterator scIt					= scList->begin();

	while(scIt != scList->end())
	{
		skillCommandsByteCount += ((gSkillManager->getSkillCmdById(*scIt)).getLength() + 2);
		++scIt;
	}

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(9);

	gMessageFactory->addUint32(15 + skillCommandsByteCount);
	gMessageFactory->addUint16(1);

	gMessageFactory->addUint16(0);
	gMessageFactory->addUint32(scList->size() + 1); 

	// playerObject->mSkillCmdUpdateCounter += 1;
	playerObject->mSkillCmdUpdateCounter += scList->size() + 1;
	gMessageFactory->addUint32(playerObject->mSkillCmdUpdateCounter);

	gMessageFactory->addUint8(3);	   //3 for rewrite
	gMessageFactory->addUint16(scList->size());

	scIt = scList->begin();

	while(scIt != scList->end())
	{
		gMessageFactory->addString(gSkillManager->getSkillCmdById(*scIt));
		++scIt;
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: schematics
//

bool MessageLib::sendSchematicDeltasPLAY_9(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	SchematicsIdList*			sList		= playerObject->getSchematicsIdList();
	SchematicsIdList::iterator	sIt			= sList->begin();
	Datapad*					datapad		= dynamic_cast<Datapad*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Datapad));

	gMessageFactory->StartMessage();               
	
	gMessageFactory->addUint16(1); 

	gMessageFactory->addUint16(4);

	gMessageFactory->addUint32(sList->size()+1);

	//gLogger->logMsgF("Yalp9 Baseline Schematicslist::Listsize %u",MSG_HIGH,sList->size());

	datapad->mSchematicUpdateCounter += sList->size()+1;
	//gLogger->logMsgF("Yalp9 Baseline Schematicslist::UpdateCounter %u",MSG_HIGH,datapad->mSchematicUpdateCounter);
	
	gMessageFactory->addUint32(datapad->mSchematicUpdateCounter);
	
	gMessageFactory->addUint8(3);	 //this looks a little strange to me
	// basically I think we are resetting the list (3 = reset all 0 = new list size)
	// and then adding all the entries from scratch
	// however it works nicely :)

	gMessageFactory->addUint16(0);
	uint16 count = 1;

	sIt = sList->begin();

	while(sIt != sList->end())
	{
		gMessageFactory->addUint8(1);
		gMessageFactory->addUint16(count);
		gMessageFactory->addUint64((*sIt));
		count += 1;
		++sIt;
	}

	Message* data = gMessageFactory->EndMessage(); 



	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(9);

	gMessageFactory->addUint32(data->getSize());
	gMessageFactory->addData(data->getData(),data->getSize());

	data->setPendingDelete(true);

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}



//======================================================================================================================
//
// Player Deltas Type 8
// update: xp
//

bool MessageLib::sendXpUpdate(uint32 xpType,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	// We must be able to set the value of 0, when we use up all xp.

	string		xpTypeName = gSkillManager->getXPTypeById(xpType);

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(8);

	gMessageFactory->addUint32(19 + xpTypeName.getLength());

	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(0);

	gMessageFactory->addUint32(1);
	gMessageFactory->addUint32(++playerObject->mXpUpdateCounter);

	gMessageFactory->addUint8(2);
	gMessageFactory->addString(xpTypeName);
	gMessageFactory->addInt32(playerObject->getXpAmount(xpType));

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 8
// update: xp types
//

bool MessageLib::sendUpdateXpTypes(SkillXpTypesList newXpTypes,uint8 remove,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	uint32		xpByteCount			= 0;
	SkillXpTypesList::iterator it	= newXpTypes.begin();

	while(it != newXpTypes.end())
	{
		xpByteCount += 7 + gSkillManager->getXPTypeById(*it).getLength();
		++it;
	}

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(8);

	gMessageFactory->addUint32(12 + xpByteCount);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(0);

	gMessageFactory->addUint32(newXpTypes.size());

	playerObject->mXpUpdateCounter += newXpTypes.size();
	gMessageFactory->addUint32(playerObject->mXpUpdateCounter);

	it = newXpTypes.begin();

	while(it != newXpTypes.end())
	{
		gMessageFactory->addUint8(remove);
		gMessageFactory->addString(gSkillManager->getXPTypeById(*it));	
		gMessageFactory->addInt32(0);

		++it;
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: friend list
//

bool MessageLib::sendFriendListPlay9(PlayerObject* playerObject)
{
	// gLogger->logMsg("MessageLib::sendFriendListPlay9...");
	if(!(playerObject->isConnected()))
		return(false);
	// gLogger->logMsg("... connected");
	ContactMap*				friendList	= playerObject->getFriendsList();
	ContactMap::iterator	nameIt		= friendList->begin();

	uint32 friendListByteCount = 0;

	while(nameIt != friendList->end())
	{
		friendListByteCount += ((*nameIt).second.getLength() + 2);
		++nameIt;
	}

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(9);

	gMessageFactory->addUint32(15 + friendListByteCount);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(7);

	
	playerObject->advanceFriendsListUpdateCounter(friendList->size() + 1);

	gMessageFactory->addUint32(friendList->size() + 1);
	gMessageFactory->addUint32(playerObject->getFriendsListUpdateCounter());

	gMessageFactory->addUint8(3);
	gMessageFactory->addUint16(friendList->size());

	nameIt = friendList->begin();

	while(nameIt != friendList->end())
	{
		gMessageFactory->addString((*nameIt).second);
		++nameIt;
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: ignore list
//

bool MessageLib::sendIgnoreListPlay9(PlayerObject* playerObject)
{
	// gLogger->logMsg("MessageLib::sendIgnoreListPlay9...");
	if(!(playerObject->isConnected()))
		return(false);
	// gLogger->logMsg("... connected");

	ContactMap*				ignoreList	= playerObject->getIgnoreList();
	ContactMap::iterator	nameIt		= ignoreList->begin();

	uint32 ignoreListByteCount = 0;

	while(nameIt != ignoreList->end())
	{
		ignoreListByteCount += (((*nameIt).second).getLength() + 2);
		++nameIt;
	}

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(9);

	gMessageFactory->addUint32(15 + ignoreListByteCount);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(8);

	playerObject->advanceIgnoresListUpdateCounter(ignoreList->size() + 1);

	gMessageFactory->addUint32(ignoreList->size() + 1);
	gMessageFactory->addUint32(playerObject->getIgnoresListUpdateCounter());

	gMessageFactory->addUint8(3);
	gMessageFactory->addUint16(ignoreList->size());

	nameIt = ignoreList->begin();

	while(nameIt != ignoreList->end())
	{
		// gLogger->logMsgF("Ignore: %s",MSG_NORMAL, (*nameIt).second.getAnsi());
		gMessageFactory->addString((*nameIt).second);
		++nameIt;
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 3
// update: matchmaking
//

bool MessageLib::sendMatchPlay3(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(24);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(6);

	gMessageFactory->addUint32(4); // unknown
	gMessageFactory->addUint32(playerObject->getPlayerMatch(0)); 
	gMessageFactory->addUint32(playerObject->getPlayerMatch(1));
	gMessageFactory->addUint32(playerObject->getPlayerMatch(2)); 
	gMessageFactory->addUint32(playerObject->getPlayerMatch(3)); 

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: crafting stage
//

bool MessageLib::sendUpdateCraftingStage(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());           
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(9);

	gMessageFactory->addUint32(8);                 
	gMessageFactory->addUint16(1);	
	gMessageFactory->addUint16(2);
	gMessageFactory->addUint32(playerObject->getCraftingStage());

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: experimentation flag
//

bool MessageLib::sendUpdateExperimentationFlag(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());           
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(9);

	gMessageFactory->addUint32(8);                 
	gMessageFactory->addUint16(1);	
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint32(playerObject->getExperimentationFlag());

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: crafting stage
//

bool MessageLib::sendUpdateExperimentationPoints(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());           
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(9);

	gMessageFactory->addUint32(8);                 
	gMessageFactory->addUint16(1);	
	gMessageFactory->addUint16(5);
	gMessageFactory->addUint32(playerObject->getExperimentationPoints());

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: nearest station
//

bool MessageLib::sendUpdateNearestCraftingStation(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());           
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(9);

	gMessageFactory->addUint32(12);                 
	gMessageFactory->addUint16(1);	
	gMessageFactory->addUint16(3);
	gMessageFactory->addUint64(playerObject->getNearestCraftingStation());

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 9
// update: language
//

void MessageLib::sendLanguagePlay9(PlayerObject* playerObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());          
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(9);

	gMessageFactory->addUint32(8);
	gMessageFactory->addUint16(1);//one var updated
	gMessageFactory->addUint16(9);//var nr 9 updated

	gMessageFactory->addUint32(playerObject->getLanguage());

	_sendToInRange(gMessageFactory->EndMessage(),playerObject,5);
}

//======================================================================================================================
//
// Player Deltas Type 8
// update: current force
//

bool MessageLib::sendUpdateCurrentForce(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());           
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(8);

	gMessageFactory->addUint32(8);                 
	gMessageFactory->addUint16(1);	
	gMessageFactory->addUint16(2);
	gMessageFactory->addUint32(playerObject->getHam()->getCurrentForce());

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Player Deltas Type 8
// update: max force
//

bool MessageLib::sendUpdateMaxForce(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getPlayerObjId());           
	gMessageFactory->addUint32(opPLAY);           
	gMessageFactory->addUint8(8);

	gMessageFactory->addUint32(8);                 
	gMessageFactory->addUint16(1);	
	gMessageFactory->addUint16(3);
	gMessageFactory->addUint32(playerObject->getHam()->getMaxForce());

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================


