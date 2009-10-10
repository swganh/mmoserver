/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/


#include "MessageLib.h"

#include "ZoneServer/Bank.h"
#include "ZoneServer/Inventory.h"
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

#include <boost/lexical_cast.hpp>


//======================================================================================================================
//
// Creature Baselines Type 1
// contain: skills, bank and inventory credits, maximum ham bar values
//

bool MessageLib::sendBaselinesCREO_1(PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	Message*	message;
	Ham*		playerHam		= player->getHam();
	SkillList*	playerSkills	= player->getSkills();

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(player->getId()); 
	gMessageFactory->addUint32(opCREO);
	gMessageFactory->addUint8(1);    

	// compute the skill list size
	uint32				skillByteCount	= 0;
	SkillList::iterator it				= playerSkills->begin();

	while(it != playerSkills->end())
	{
		skillByteCount += ((*it)->mName.getLength() + 2);

		++it;
	}

	gMessageFactory->addUint32(62 + skillByteCount);
	gMessageFactory->addUint16(4);     

	// bank credits
	if(Bank* bank = dynamic_cast<Bank*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank)))
	{
		gMessageFactory->addUint32(bank->getCredits());
	}
	else
	{
		gMessageFactory->addUint32(0);
	}

	// inventory credits
	if(Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)))
	{
		gMessageFactory->addUint32(inventory->getCredits());
	}
	else
	{
		gMessageFactory->addUint32(0);
	}

	// ham maxs
	gMessageFactory->addUint32(9);
	gMessageFactory->addUint32(9);

	gMessageFactory->addUint32(playerHam->mHealth.getBaseHitPoints());
	gMessageFactory->addUint32(playerHam->mStrength.getBaseHitPoints());
	gMessageFactory->addUint32(playerHam->mConstitution.getBaseHitPoints());
	gMessageFactory->addUint32(playerHam->mAction.getBaseHitPoints());
	gMessageFactory->addUint32(playerHam->mQuickness.getBaseHitPoints());
	gMessageFactory->addUint32(playerHam->mStamina.getBaseHitPoints());
	gMessageFactory->addUint32(playerHam->mMind.getBaseHitPoints());
	gMessageFactory->addUint32(playerHam->mFocus.getBaseHitPoints());
	gMessageFactory->addUint32(playerHam->mWillpower.getBaseHitPoints());

	// skills
	gMessageFactory->addUint64(playerSkills->size());

	it = playerSkills->begin();

	while(it != playerSkills->end())
	{
		gMessageFactory->addString((*it)->mName);

		++it;
	}

	message = gMessageFactory->EndMessage();

	(player->getClient())->SendChannelA(message, player->getAccountId(), CR_Client, 3);

	return(true);
}

//======================================================================================================================
//
// Creature Baselines Type 3
// contain: general creature properties(name, species, customization, posture, state, faction, ham wounds,..)
//

bool MessageLib::sendBaselinesCREO_3(CreatureObject* creatureObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message*		message;
	Ham*			creatureHam = creatureObject->getHam();
	string			firstName = creatureObject->getFirstName().getAnsi();
	string			lastName = creatureObject->getLastName().getAnsi();
	string			fullName;
	uint32			creoByteCount;
	uint32			byteCount;

	// if its a persistent npc, we don't need all ham bars
	if(creatureObject->getCreoGroup() == CreoGroup_PersistentNpc)
		byteCount = 107;
	else
		byteCount = 119;

	// make sure we got a name
	if(firstName.getLength() > 1)
	{
		fullName << firstName.getAnsi();
	}

	if(lastName.getLength() > 1)
	{
		fullName << " ";
		fullName << lastName.getAnsi();
	}

	// needs to be send as unicode
	fullName.convert(BSTRType_Unicode16);

	creoByteCount = byteCount + creatureObject->getSpeciesGroup().getLength() + (fullName.getLength() << 1) + creatureObject->getCustomizationStr().getLength() + creatureObject->getSpeciesString().getLength();
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(creatureObject->getId()); 
	gMessageFactory->addUint32(opCREO);
	gMessageFactory->addUint8(3);    
	gMessageFactory->addUint32(creoByteCount);
	gMessageFactory->addUint16(12);
	//0
	gMessageFactory->addUint32(16256); // unknown
	//1
	gMessageFactory->addString(creatureObject->getSpeciesGroup());
	gMessageFactory->addUint32(0);     // unknown
	gMessageFactory->addString(creatureObject->getSpeciesString());
	//2
	gMessageFactory->addString(fullName);
	//3
	gMessageFactory->addUint32(1); // unknown
	//4
	gMessageFactory->addString(creatureObject->getCustomizationStr());
	//5 unknown list
	gMessageFactory->addUint32(0); // unknown
	gMessageFactory->addUint32(0); // unknown

	//6
	gMessageFactory->addUint32(creatureObject->getTypeOptions());
	//7 incap timer
	gMessageFactory->addUint32(0); // unknown


	// For now, we use the Health bar when using a single H-bar.

	//8 condition damage (vehicle) //this is the amount of damage... used to set appearence of swoop
	gMessageFactory->addUint32(creatureHam->getPropertyValue(HamBar_Health,HamProperty_MaxHitpoints) - creatureHam->getPropertyValue(HamBar_Health,HamProperty_CurrentHitpoints));

	//9 max condition (vehicle)
	gMessageFactory->addUint32(creatureHam->getPropertyValue(HamBar_Health,HamProperty_MaxHitpoints));

	//10 posture updatecounter
	gMessageFactory->addUint8(1); // unknown
	//11 posture
	gMessageFactory->addUint8(creatureObject->getPosture()); 
	//12
	gMessageFactory->addUint8(creatureObject->getFactionRank());
	//13 owner id
	if(creatureObject->getCreoGroup()  == CreoGroup_Vehicle)
	{
		gMessageFactory->addUint64(creatureObject->getOwner());
		gMessageFactory->addFloat(creatureObject->getScale());
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint64(0);
	}
	else	
	{
		gMessageFactory->addUint64(0); 
		gMessageFactory->addFloat(creatureObject->getScale());
		gMessageFactory->addUint32(creatureHam->getBattleFatigue());
		gMessageFactory->addUint64(creatureObject->getState()); 
	}	

	// ham wounds

	if(creatureObject->getCreoGroup() == CreoGroup_PersistentNpc)
	{
		gMessageFactory->addUint32(6);
		gMessageFactory->addUint32(creatureHam->getWoundsUpdateCounter());

		gMessageFactory->addUint32(0);
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint32(0);
	}
	else if(creatureObject->getCreoGroup() == CreoGroup_Vehicle)
	{
		//no wounds for vehicles
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint32(0);
	}
	else
	{
		gMessageFactory->addUint32(9);
		//ok make sure updatecounter gets send as 9 on baseline
		gMessageFactory->addUint32(creatureHam->getWoundsUpdateCounter());

		gMessageFactory->addUint32(creatureHam->mHealth.getWounds());
		gMessageFactory->addUint32(creatureHam->mStrength.getWounds());
		gMessageFactory->addUint32(creatureHam->mConstitution.getWounds());
		gMessageFactory->addUint32(creatureHam->mAction.getWounds());
		gMessageFactory->addUint32(creatureHam->mQuickness.getWounds());
		gMessageFactory->addUint32(creatureHam->mStamina.getWounds());
		gMessageFactory->addUint32(creatureHam->mMind.getWounds());
		gMessageFactory->addUint32(creatureHam->mFocus.getWounds());
		gMessageFactory->addUint32(creatureHam->mWillpower.getWounds());
	}

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Creature Baselines Type 4
// contain: movement properties, ham encumbrance, skillmods
//

bool MessageLib::sendBaselinesCREO_4(PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	Ham*			playerHam		= player->getHam();
	SkillModsList*	playerSkillMods = player->getSkillMods();

	//thats the message databody
	gMessageFactory->StartMessage();
	
	gMessageFactory->addUint16(14); 

	gMessageFactory->addFloat(1.0f); // acceleration base
	gMessageFactory->addFloat(1.0f); // acceleration mod

	// ham encumbrance
	gMessageFactory->addUint32(3);
	gMessageFactory->addUint32(3);

	gMessageFactory->addUint32(playerHam->mHealth.getEncumbrance());
	gMessageFactory->addUint32(playerHam->mAction.getEncumbrance());
	gMessageFactory->addUint32(playerHam->mMind.getEncumbrance());

	// skillmods
	gMessageFactory->addUint32(playerSkillMods->size());
	player->mSkillModUpdateCounter += playerSkillMods->size();
	gMessageFactory->addUint32(player->mSkillModUpdateCounter);  

	SkillModsList::iterator it = playerSkillMods->begin();

	while(it != playerSkillMods->end())
	{
		gMessageFactory->addUint8(0);
		gMessageFactory->addString(gSkillManager->getSkillModById((*it).first));
		gMessageFactory->addUint32((*it).second);
		gMessageFactory->addUint32(0);

		++it;
	}

	// 4
	gMessageFactory->addFloat(1.0f);

	//5
	gMessageFactory->addFloat(1.0f); 

	//6 Listen to Id
	gMessageFactory->addUint64(player->getEntertainerListenToId());

	//7
	gMessageFactory->addFloat(player->getCurrentRunSpeedLimit()); //RUN speed  

	//8
	gMessageFactory->addFloat(1.02f);	// unknown

	//9
	gMessageFactory->addFloat(player->getCurrentTerrainNegotiation());
	
	//10 (a)
	gMessageFactory->addFloat(player->getCurrentTurnRate());	

	//11(b)
	gMessageFactory->addFloat(player->getCurrentAcceleration()); // This is the walk speed, nothing else.

	gMessageFactory->addFloat(0.0125f);	// unknown
	gMessageFactory->addUint64(0);	// unknown

	Message* data = gMessageFactory->EndMessage();


	//Now the Message header

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(player->getId()); 
	gMessageFactory->addUint32(opCREO);
	gMessageFactory->addUint8(4);  

	gMessageFactory->addUint32(data->getSize()); 
	gMessageFactory->addData(data->getData(),data->getSize());
	data->setPendingDelete(true);
															  
	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(),player->getAccountId(),CR_Client,3);

	return(true);
}


//======================================================================================================================
//
// Creature Baselines Type 6
// contain: gameplay properties(cl, animation, mood, group, target, hitpoints, equipped objects, defender lists)
//

bool MessageLib::sendBaselinesCREO_6(CreatureObject* creatureObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message*		message;
	uint32			byteCount;
	uint32			cSize			= 0;
	Ham*			creatureHam		= creatureObject->getHam();
	// Test ERU 
	// If no mood is set, use neutral for avatar / npc, then they will look less angry as default.
	// This will NOT affect the chat-mood
	// string			moodStr			= gWorldManager->getMood(creatureObject->getMoodId());
	uint8 moodId = creatureObject->getMoodId();
	if (moodId == 0)
	{
		moodId = 74;
	}
	string			moodStr			= gWorldManager->getMood(moodId);

	ObjectList*		equippedObjects = creatureObject->getEquipManager()->getEquippedObjects();
	ObjectIDList*	defenders		= creatureObject->getDefenders();

	// add customization string size of equipped objects
	ObjectList::iterator eqIt = equippedObjects->begin();

	while(eqIt != equippedObjects->end())
	{
		if(TangibleObject* object = dynamic_cast<TangibleObject*>(*eqIt))
		{
			cSize += object->getCustomizationStr().getLength();
		}
		else if(CreatureObject* pet = dynamic_cast<CreatureObject*>(*eqIt))
		{
			cSize += pet->getCustomizationStr().getLength();
		}

		++eqIt;
	}

	// persistent npc don't need all hambars
	if(creatureObject->getCreoGroup() == CreoGroup_PersistentNpc)
	{
		byteCount = 148;
	}
	else if(creatureObject->getCreoGroup() == CreoGroup_Vehicle)
	{
		//vehicles don't need ham bars
		byteCount = 100;
	}
	else
	{
		byteCount = 172;
	}

	byteCount += cSize;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBaselinesMessage);   
	gMessageFactory->addUint64(creatureObject->getId()); 
	gMessageFactory->addUint32(opCREO);
	gMessageFactory->addUint8(6);  

	gMessageFactory->addUint32(byteCount + (defenders->size() * 8) + (equippedObjects->size() * 18) + moodStr.getLength() + creatureObject->getCurrentAnimation().getLength());
	gMessageFactory->addUint16(22); 

	gMessageFactory->addUint32(creatureObject->getSubZoneId());

	// defenders
	gMessageFactory->addUint32(defenders->size());	
	gMessageFactory->addUint32(creatureObject->mDefenderUpdateCounter); 

	ObjectIDList::iterator defenderIt = defenders->begin();

	while(defenderIt != defenders->end())
	{
		//gMessageFactory->addUint64((*defenderIt)->getId());
		gMessageFactory->addUint64(*defenderIt);
		++defenderIt;
	}

	gMessageFactory->addUint16(creatureObject->getCL());  
	gMessageFactory->addString(creatureObject->getCurrentAnimation());   // music/dance string here - current animation
	gMessageFactory->addString(moodStr);	

	if(Object* weapon = creatureObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Weapon))
	{
		gMessageFactory->addUint64(weapon->getId());
	}
	else
	{
		gMessageFactory->addUint64(0);
	}

	//6 Group Id
	gMessageFactory->addUint64(creatureObject->getGroupId());  
	gMessageFactory->addUint64(0);   // Invite sender Id
	gMessageFactory->addUint64(0);   // Invite Counter
	gMessageFactory->addUint32(0);   // guild Id

	//9
	gMessageFactory->addUint64(creatureObject->getTargetId());  

	//a
	gMessageFactory->addUint8(creatureObject->getMoodId());

	//b
	if(creatureObject->getCreoGroup() == CreoGroup_Vehicle)
	{
		gMessageFactory->addUint32(0);
	}
	else
	{
		gMessageFactory->addUint32(creatureObject->UpdatePerformanceCounter());   // unknown
	}

	//c thats not performance id to be used with dancing
	//use with music only
	gMessageFactory->addUint32(creatureObject->getPerformanceId());   // performance id

	// current ham
	if(creatureObject->getCreoGroup() == CreoGroup_PersistentNpc)
	{
		if(!creatureHam->getCurrentHitpointsUpdateCounter())
		{
			creatureHam->advanceCurrentHitpointsUpdateCounter(6);
		}

		gMessageFactory->addUint32(6);
		gMessageFactory->addUint32(creatureHam->getCurrentHitpointsUpdateCounter());

		gMessageFactory->addUint32(creatureHam->mHealth.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mStrength.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mConstitution.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mAction.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mQuickness.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mStamina.getCurrentHitPoints());

		// max ham
		if(!creatureHam->getMaxHitpointsUpdateCounter())
		{
			creatureHam->advanceMaxHitpointsUpdateCounter(6);
		}

		gMessageFactory->addUint32(6);
		gMessageFactory->addUint32(creatureHam->getMaxHitpointsUpdateCounter());

		gMessageFactory->addUint32(creatureHam->mHealth.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mStrength.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mConstitution.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mAction.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mQuickness.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mStamina.getMaxHitPoints());
	}
	else if(creatureObject->getCreoGroup() == CreoGroup_Vehicle)
	{
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint32(0);

	}
	else
	{
		if(!creatureHam->getCurrentHitpointsUpdateCounter())
		{
			creatureHam->advanceCurrentHitpointsUpdateCounter(9);
		}

		gMessageFactory->addUint32(9);
		gMessageFactory->addUint32(creatureHam->getCurrentHitpointsUpdateCounter());

		gMessageFactory->addUint32(creatureHam->mHealth.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mStrength.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mConstitution.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mAction.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mQuickness.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mStamina.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mMind.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mFocus.getCurrentHitPoints());
		gMessageFactory->addUint32(creatureHam->mWillpower.getCurrentHitPoints());

		// max ham
		if(!creatureHam->getMaxHitpointsUpdateCounter())
		{
			creatureHam->advanceMaxHitpointsUpdateCounter(9);
		}

		gMessageFactory->addUint32(9);
		gMessageFactory->addUint32(creatureHam->getMaxHitpointsUpdateCounter());

		gMessageFactory->addUint32(creatureHam->mHealth.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mStrength.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mConstitution.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mAction.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mQuickness.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mStamina.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mMind.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mFocus.getMaxHitPoints());
		gMessageFactory->addUint32(creatureHam->mWillpower.getMaxHitPoints());
	}

	// creatures tangible objects	 ->equipped list
	eqIt = equippedObjects->begin();

	gMessageFactory->addUint32(equippedObjects->size());
	creatureObject->getEquipManager()->resetEquippedObjectsUpdateCounter();
	gMessageFactory->addUint32(creatureObject->getEquipManager()->getEquippedObjectsUpdateCounter());

	while(eqIt != equippedObjects->end())
	{
		Object* object = (*eqIt);

		if(TangibleObject* tObject = dynamic_cast<TangibleObject*>(object))
		{
			gMessageFactory->addString(tObject->getCustomizationStr());
		}
		else if(CreatureObject* pet = dynamic_cast<CreatureObject*>(object)) //support for mounts
		{
			gMessageFactory->addString(pet->getCustomizationStr());
		}
		else
		{
			gMessageFactory->addUint16(0);
		}

		gMessageFactory->addUint32(4);
		gMessageFactory->addUint64(object->getId());
		gMessageFactory->addUint32((object->getModelString()).getCrc());

		++eqIt;
	}

	gMessageFactory->addUint16(0); // unknown
	gMessageFactory->addUint8(0);  // extra byte that was needed to correct movement
	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// Posture Message
// updates a creatures posture
//

bool MessageLib::sendPostureMessage(CreatureObject* creatureObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message*		message;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opUpdatePostureMessage);
	gMessageFactory->addUint8(creatureObject->getPosture());      
	gMessageFactory->addUint64(creatureObject->getId());

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 3);

	return(true);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// updates: defenders single
//

void MessageLib::sendDefenderUpdate(CreatureObject* creatureObject,uint8 updateType,uint16 index,uint64 defenderId)
{
	// ObjectList*	defenders = creatureObject->getDefenders();

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opDeltasMessage);   
	gMessageFactory->addUint64(creatureObject->getId()); 
	gMessageFactory->addUint32(opCREO);
	gMessageFactory->addUint8(6);  

	uint32	payloadSize = 0;
	
	if (updateType == 0)
	{
		// Clear defender
		payloadSize = 15;
	}
	else if ((updateType == 1) || (updateType == 2))
	{
		// Add or change defender
		payloadSize = 23;
	}
	else if (updateType == 4)
	{
		// Clear all
		payloadSize = 13;
	}
	else // if (updateType == 3)
	{
		// Reset all
		// Not suported yet
		gLogger->logMsgF("MessageLib::sendDefenderUpdate Invalid option = %u", MSG_NORMAL,updateType);
		assert(false);
	}

	gMessageFactory->addUint32(payloadSize);

	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(1);

	gMessageFactory->addUint32(1);
	gMessageFactory->addUint32(++creatureObject->mDefenderUpdateCounter);

	gMessageFactory->addUint8(updateType);

	if (updateType == 0)
	{	
		gMessageFactory->addUint16(index);
	}
	else if ((updateType == 1) || (updateType == 2))
	{
		gMessageFactory->addUint16(index);
		gMessageFactory->addUint64(defenderId);
	}
	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);

}

//======================================================================================================================
//
// Creature Deltas Type 6
// updates: defenders full
//


void MessageLib::sendNewDefenderList(CreatureObject* creatureObject)
{
	ObjectIDList* defenders = creatureObject->getDefenders();
	uint32 byteCount = 15;

	if (defenders->empty())
	{
		// Doing a reset if 0 defenders.
		byteCount = 13;
	}

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opDeltasMessage);   
	gMessageFactory->addUint64(creatureObject->getId()); 
	gMessageFactory->addUint32(opCREO);
	gMessageFactory->addUint8(6);  

	gMessageFactory->addUint32(byteCount + (defenders->size() * 8));
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(1);

	ObjectIDList::iterator defenderIt = defenders->begin();
	// Shall we not advance the updatecounter if we send a reset, where size() is 0? 

	// I'm pretty sure the idea of update counters is to let the client know that somethings have changed, 
	// and to know in what order, given several messages "at once".
	// creatureObject->mDefenderUpdateCounter = creatureObject->mDefenderUpdateCounter + defenders->size();
	// gMessageFactory->addUint32(++creatureObject->mDefenderUpdateCounter);

	if(!defenders->size())
	{
		// Even an update with zero defenders is a new update.
		gMessageFactory->addUint32(1);
		
		gMessageFactory->addUint32(++creatureObject->mDefenderUpdateCounter);
		gMessageFactory->addUint8(4);
	}
	else
	{
		gMessageFactory->addUint32(defenders->size());
		// gMessageFactory->addUint32(1);

		gMessageFactory->addUint32(++creatureObject->mDefenderUpdateCounter);
		gMessageFactory->addUint8(3);
		gMessageFactory->addUint16(defenders->size());

		while (defenderIt != defenders->end())
		{
			gMessageFactory->addUint64((*defenderIt));
			++defenderIt;
		}
	}

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}


//======================================================================================================================
//
// Creature Deltas Type 6
// updates: list of equipped objects
//

bool MessageLib::sendEquippedListUpdate_InRange(CreatureObject* creatureObject)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(creatureObject);
	if(player)
	{
		if(!player->isConnected())
			return(false);
	}

	ObjectList*				equippedObjects				= creatureObject->getEquipManager()->getEquippedObjects();
	ObjectList::iterator	eqIt						= equippedObjects->begin();
	uint32					cSize						= 0;

	// customization is necessary for haircolor on imagedesign
	while(eqIt != equippedObjects->end())
	{
		if(TangibleObject* object = dynamic_cast<TangibleObject*>(*eqIt))
		{
			cSize += object->getCustomizationStr().getLength();
		}
		else if(CreatureObject* pet = dynamic_cast<CreatureObject*>(*eqIt))
		{
			cSize += pet->getCustomizationStr().getLength();
		}

		++eqIt;
	}

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opDeltasMessage);   
	gMessageFactory->addUint64(creatureObject->getId()); 
	gMessageFactory->addUint32(opCREO);
	gMessageFactory->addUint8(6);  

	gMessageFactory->addUint32(15 + (equippedObjects->size() * 18)+ cSize);
	gMessageFactory->addUint16(1);   //one update
	gMessageFactory->addUint16(15);				 //id 15

	// creatures tangible objects
	eqIt = equippedObjects->begin();

	gMessageFactory->addUint32(equippedObjects->size());
	gMessageFactory->addUint32(creatureObject->getEquipManager()->advanceEquippedObjectsUpdateCounter(equippedObjects->size()));//+1
	creatureObject->getEquipManager()->advanceEquippedObjectsUpdateCounter(1);

	gMessageFactory->addUint8(3);
	gMessageFactory->addUint16(equippedObjects->size());

	while(eqIt != equippedObjects->end())
	{
		Object* object = (*eqIt);

		if(TangibleObject* tObject = dynamic_cast<TangibleObject*>(object))
		{
			gMessageFactory->addString(tObject->getCustomizationStr());
		}
		else if(CreatureObject* pet = dynamic_cast<CreatureObject*>(object))
		{
			gMessageFactory->addString(pet->getCustomizationStr());
		}
		else
		{
			gMessageFactory->addUint16(0);
		}

		gMessageFactory->addUint32(4);
		gMessageFactory->addUint64(object->getId());
		gMessageFactory->addUint32((object->getModelString()).getCrc());

		++eqIt;
	}

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);

	return(true);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// updates: list of equipped objects
//

bool MessageLib::sendEquippedItemUpdate_InRange(CreatureObject* creatureObject, uint64 itemId)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(creatureObject);
	if((!player)||(!player->isConnected()))
	{
		return(false);
	}

	ObjectList*				equippedObjects				= creatureObject->getEquipManager()->getEquippedObjects();
	ObjectList::iterator	eqIt						= equippedObjects->begin();
	uint32					cSize						= 0;

	// customization is necessary for haircolor on imagedesign
	//we only want to change the object with the given ID
	uint16	index	= 0;
	uint16	i		= 0;
	bool	found	= false;

	while(eqIt != equippedObjects->end())
	{		
		if(TangibleObject* object = dynamic_cast<TangibleObject*>(*eqIt))
		{
			if(object->getId() == itemId)
			{
				cSize += object->getCustomizationStr().getLength();
				index = i;
				found = true;
				break;
			}

		}
		else if(CreatureObject* pet = dynamic_cast<CreatureObject*>(*eqIt))
		{
			if(pet->getId() == itemId)
			{
				cSize += pet->getCustomizationStr().getLength();
				index = i;
				found = true;
				break;
			}
		}
		i++;				
		++eqIt;
	}

	if(!found)
	{
		gLogger->logMsgF("MessageLib::sendEquippedItemUpdate_InRange : Item not found : %I64u", MSG_NORMAL,itemId);
		return false;
	}

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opDeltasMessage);   
	gMessageFactory->addUint64(creatureObject->getId()); 
	gMessageFactory->addUint32(opCREO);
	gMessageFactory->addUint8(6);  

	gMessageFactory->addUint32(15 + (1 * 18)+ cSize);
	gMessageFactory->addUint16(1);   //one update
	gMessageFactory->addUint16(15);				 //id 15

	// creatures tangible objects
	eqIt = equippedObjects->begin();

	gMessageFactory->addUint32(1);	//only one item gets updated
	gMessageFactory->addUint32(creatureObject->getEquipManager()->advanceEquippedObjectsUpdateCounter(1));//+1
	creatureObject->getEquipManager()->advanceEquippedObjectsUpdateCounter(1);

	gMessageFactory->addUint8(2);  //2 for change a given entry
	gMessageFactory->addUint16(index);//index of the entry

	while(eqIt != equippedObjects->end())
	{
		Object* object = (*eqIt);
		if ( object->getId() == itemId)
		{
			if(TangibleObject* tObject = dynamic_cast<TangibleObject*>(object))
			{
				gMessageFactory->addString(tObject->getCustomizationStr());
			}
			else if(CreatureObject* pet = dynamic_cast<CreatureObject*>(object))
			{
				gMessageFactory->addString(pet->getCustomizationStr());
			}
			else
			{
				gMessageFactory->addUint16(0);
			}

			gMessageFactory->addUint32(4);
			gMessageFactory->addUint64(object->getId());
			gMessageFactory->addUint32((object->getModelString()).getCrc());
			break;
		}
		++eqIt;
	}

	Message* message = gMessageFactory->EndMessage();
	//gLogger->hexDump(message->getData(),message->getSize());
	//message->ResetIndex();
	_sendToInRange(message,creatureObject,5);


	return(true);
}


//======================================================================================================================
//
// Update pvp status
// sets if attackable or not
//

bool MessageLib::sendUpdatePvpStatus(CreatureObject* creatureObject,PlayerObject* targetObject,uint32 statusMask)
{
	if(!(targetObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opUpdatePvpStatusMessage);

	if (!statusMask)
	{
		gMessageFactory->addUint32(creatureObject->getPvPStatus());
		// gLogger->logMsgF("MessageLib::sendUpdatePvpStatus: creatureObject = %llu, State = %x", MSG_NORMAL, creatureObject->getId(), (uint32)creatureObject->getPvPStatus());
	}
	else
	{
		gMessageFactory->addUint32(statusMask);
	}

	// neutral
	if(creatureObject->getFaction().getCrc() == 0x1fdc3051)
		gMessageFactory->addUint32(0);
	else
		gMessageFactory->addUint32(creatureObject->getFaction().getCrc());

	gMessageFactory->addUint64(creatureObject->getId());

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,4);

	return(true);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: mood
//

void MessageLib::sendMoodUpdate(CreatureObject* srcObject)
{
	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(srcObject->getId());           
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(5);                 
	gMessageFactory->addUint16(1);	
	gMessageFactory->addUint16(10);
	gMessageFactory->addUint8(srcObject->getMoodId());

	_sendToInRange(gMessageFactory->EndMessage(),srcObject,5);	
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: posture
//

void MessageLib::sendPostureUpdate(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId());           
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(3);
	gMessageFactory->addUint32(5);                 
	gMessageFactory->addUint16(1);	
	gMessageFactory->addUint16(11);					 
	gMessageFactory->addUint8(creatureObject->getPosture());	

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5,true);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: posture and state
//

void MessageLib::sendPostureAndStateUpdate(CreatureObject* creatureObject)
{
	// Test code for npc combat with objects that can have no states, like debris.
	if (creatureObject->getCreoGroup() != CreoGroup_AttackableObject)
	{
		gMessageFactory->StartMessage();   
		gMessageFactory->addUint32(opDeltasMessage);  
		gMessageFactory->addUint64(creatureObject->getId());           
		gMessageFactory->addUint32(opCREO);           
		gMessageFactory->addUint8(3);
		gMessageFactory->addUint32(15);                 
		gMessageFactory->addUint16(2);	
		gMessageFactory->addUint16(11);					 
		gMessageFactory->addUint8(creatureObject->getPosture());	
		gMessageFactory->addUint16(16);
		gMessageFactory->addUint64(creatureObject->getState());

		_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
	}
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: state
//

void MessageLib::sendStateUpdate(CreatureObject* creatureObject)
{
	// gLogger->logMsgF("MessageLib::sendStateUpdate: creatureObject = %llu, State = %llx", MSG_NORMAL, creatureObject->getId(), creatureObject->getState());

	// Test code for npc combat with objects that can have no states, like debris.
	if (creatureObject->getCreoGroup() != CreoGroup_AttackableObject)
	{
		gMessageFactory->StartMessage();   
		gMessageFactory->addUint32(opDeltasMessage);  
		gMessageFactory->addUint64(creatureObject->getId());           
		gMessageFactory->addUint32(opCREO);           
		gMessageFactory->addUint8(3);
		gMessageFactory->addUint32(12); 
		gMessageFactory->addUint16(1);	
		gMessageFactory->addUint16(16);
		gMessageFactory->addUint64(creatureObject->getState());

		_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
	}
}

//======================================================================================================================
//
// Creature Deltas Type 3
// update: single ham health, used by statics like debris and vehicles.
//

void MessageLib::sendSingleBarUpdate(CreatureObject* creatureObject)
{
	// Test code for npc combat with objects that can have no states, like debris.
	if (creatureObject->getCreoGroup() == CreoGroup_AttackableObject)
	{
		Ham* ham = creatureObject->getHam();

		if (ham == NULL)
		{
			return;
		}

		gMessageFactory->StartMessage();   
		gMessageFactory->addUint32(opDeltasMessage);  
		gMessageFactory->addUint64(creatureObject->getId());           
		gMessageFactory->addUint32(opCREO);           
		gMessageFactory->addUint8(3);
		gMessageFactory->addUint32(8); // bytes
		gMessageFactory->addUint16(1);	// No of items
		gMessageFactory->addUint16(8);	// Index 8 condition damage (vehicle)
		uint32 damage = ham->getPropertyValue(HamBar_Health,HamProperty_MaxHitpoints);
		damage -= ham->getPropertyValue(HamBar_Health,HamProperty_CurrentHitpoints);
		// gMessageFactory->addUint32(ham->getPropertyValue(HamBar_Health,HamProperty_CurrentHitpoints));
		gMessageFactory->addUint32(damage);
		_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
	}
}


//======================================================================================================================
//
// Creature Deltas Type 1
// update: bank credits
//

bool MessageLib::sendBankCreditsUpdate(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();        
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getId());           
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(1);
	gMessageFactory->addUint32(8);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(0);	

	if(Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank)))
	{
		gMessageFactory->addUint32(bank->getCredits());					 
	}
	else
	{
		gMessageFactory->addUint32(0);
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Creature Deltas Type 1
// update: inventory credits
//

bool MessageLib::sendInventoryCreditsUpdate(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();        
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getId());           
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(1);
	gMessageFactory->addUint32(8);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(1);	

	if(Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)))
	{
		gMessageFactory->addUint32(inventory->getCredits());					 
	}
	else
	{
		gMessageFactory->addUint32(0);
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Creature Deltas Type 4
// update: movement properties
//

bool MessageLib::sendUpdateMovementProperties(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();             
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getId());           
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(4);
	gMessageFactory->addUint32(20);

	gMessageFactory->addUint16(3);

	gMessageFactory->addUint16(7);
	gMessageFactory->addFloat(playerObject->getCurrentRunSpeedLimit());

	gMessageFactory->addUint16(10);
	gMessageFactory->addFloat(playerObject->getCurrentTurnRate());

	gMessageFactory->addUint16(11);
	gMessageFactory->addFloat(playerObject->getCurrentAcceleration());

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Creature Deltas Type 1
// update: skills
//

bool MessageLib::sendSkillDeltasCreo1(Skill* skill,uint8 action,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(targetObject->getId());          
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(1);

	gMessageFactory->addUint32(15 + skill->mName.getLength());
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(3);

	gMessageFactory->addUint32(1); // listsize
	gMessageFactory->addUint32(targetObject->getAndIncrementSkillUpdateCounter()); 
	gMessageFactory->addUint8(action);
	gMessageFactory->addString(skill->mName);

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Creature Deltas Type 4
// update: skillmods
//
//TODO: This function appears to crash client, and what is its relation to MessageLib::sendSkillModUpdateCreo4
bool MessageLib::sendSkillModDeltasCREO_4(SkillModsList smList,uint8 remove,CreatureObject* creatureObject,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	SkillModsList::iterator it					= smList.begin();
	uint32					skillModByteCount	= 0;

	while(it != smList.end())
	{
		skillModByteCount += (gSkillManager->getSkillModById((*it).first).getLength() + 11); 
		++it;
	}

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId());          
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(4);

	gMessageFactory->addUint32(12 + skillModByteCount);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(3);

	gMessageFactory->addUint32(smList.size());  

	gMessageFactory->addUint32(playerObject->getAndIncrementSkillModUpdateCounter(smList.size()));  
	gMessageFactory->addUint8(remove);

	it = smList.begin();
	while(it != smList.end())
	{
		gMessageFactory->addUint8(0);
		gMessageFactory->addString(gSkillManager->getSkillModById((*it).first));
		gMessageFactory->addUint32((*it).second);
		gMessageFactory->addUint32(0);

		++it;
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: curent hitpoints 1 bar
//


void MessageLib::sendCurrentHitpointDeltasCreo6_Single(CreatureObject* creatureObject,uint8 barIndex)
{
	Ham*	ham = creatureObject->getHam();

	if(ham == NULL)
		return;

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId());          
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(19);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(13);

	// advance by 1 (overload for bigger increment)
	ham->advanceCurrentHitpointsUpdateCounter();  
	gMessageFactory->addUint32(1);  
	gMessageFactory->addUint32(ham->getCurrentHitpointsUpdateCounter());  
	//	gLogger->logMsgF("updatecounter : %u",MSG_NORMAL,ham->getCurrentHitpointsUpdateCounter());


	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(barIndex);
	gMessageFactory->addInt32(ham->getPropertyValue(barIndex,HamProperty_CurrentHitpoints));
	//gLogger->logMsg("sendcurrenthitpoints delta single");
	//gLogger->logMsgF("current hitpoints : %u",MSG_NORMAL,ham->getPropertyValue(barIndex,HamProperty_CurrentHitpoints));

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: max hitpoints 1 bar
//

void MessageLib::sendMaxHitpointDeltasCreo6_Single(CreatureObject* creatureObject,uint8 barIndex)
{
	Ham* ham = creatureObject->getHam();

	if(ham == NULL)
		return;

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId());          
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(19);

	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(14);//delta nr

	gMessageFactory->addUint32(1);
	ham->advanceMaxHitpointsUpdateCounter();  // increment list up counter by 1
	gMessageFactory->addUint32(ham->getMaxHitpointsUpdateCounter());  

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(barIndex);
	gMessageFactory->addInt32(ham->getPropertyValue(barIndex,HamProperty_MaxHitpoints));

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 1
// update: base hitpoints 1 bar
//

void MessageLib::sendBaseHitpointDeltasCreo1_Single(CreatureObject* creatureObject,uint8 barIndex)
{
	Ham* ham = creatureObject->getHam();

	if(ham == NULL)
		return;

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId());          
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(1);

	gMessageFactory->addUint32(19);

	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(2);//delta nr

	gMessageFactory->addUint32(1);
	ham->advanceBaseHitpointsUpdateCounter();  
	gMessageFactory->addUint32(ham->getBaseHitpointsUpdateCounter());  // increment list up counter by 1

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(barIndex);
	gMessageFactory->addInt32(ham->getPropertyValue(barIndex,HamProperty_BaseHitpoints));

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 3
// update: wounds 1 bar
//

void MessageLib::sendWoundUpdateCreo3(CreatureObject* creatureObject,uint8 barIndex)
{
	Ham* ham = creatureObject->getHam();

	if(ham == NULL)
		return;

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId());          
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(19);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(17);

	ham->advanceWoundsUpdateCounter();
	gMessageFactory->addUint32(1);  
	gMessageFactory->addUint32(ham->getWoundsUpdateCounter());  

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(barIndex);
	gMessageFactory->addInt32(ham->getPropertyValue(barIndex,HamProperty_Wounds));

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: hitpoints all bars, only send when all values have changed !
//

void MessageLib::sendCurrentHitpointDeltasCreo6_Full(CreatureObject* creatureObject)
{
	Ham* ham = creatureObject->getHam();

	if(ham == NULL)
		return;

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId());          
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(33);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(13);

	gMessageFactory->addUint32(3);  

	ham->advanceCurrentHitpointsUpdateCounter(3);

	gMessageFactory->addUint32(ham->getCurrentHitpointsUpdateCounter()); 

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(HamBar_Health);
	gMessageFactory->addInt32(ham->getPropertyValue(HamBar_Health,HamProperty_CurrentHitpoints));

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(HamBar_Action);
	gMessageFactory->addInt32(ham->getPropertyValue(HamBar_Action,HamProperty_CurrentHitpoints));

	gMessageFactory->addUint8(2);
	gMessageFactory->addUint16(HamBar_Mind);
	gMessageFactory->addInt32(ham->getPropertyValue(HamBar_Mind,HamProperty_CurrentHitpoints));

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 3
// update: battlefatigue
// Only players are aware of this, so PlayerObject*

void MessageLib::sendBFUpdateCreo3(CreatureObject* playerObject)
{
	Ham*			ham = playerObject->getHam();
	PlayerObject*	pObject = dynamic_cast<PlayerObject*>(playerObject);

	if(!ham || !pObject || !(pObject->isConnected()))
		return;

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getId());          
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(8);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(15);

	gMessageFactory->addInt32(ham->getBattleFatigue());  

	(pObject)->getClient()->SendChannelA(gMessageFactory->EndMessage(),pObject->getAccountId(),CR_Client,5);
}

//======================================================================================================================
//
// Creature Deltas Type 3
// update: owner id 
// used for mountable creatures (pets, vehicles..)


void MessageLib::sendOwnerUpdateCreo3(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId());          
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(12);
	gMessageFactory->addUint16(2);
	gMessageFactory->addUint16(13); // CREO 3 owner id

	gMessageFactory->addInt64(creatureObject->getOwner());  

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
	//(pObject)->getClient()->SendChannelA(gMessageFactory->EndMessage(),pObject->getAccountId(),CR_Client,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: target
//

void MessageLib::sendTargetUpdateDeltasCreo6(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage); 
	gMessageFactory->addUint64(creatureObject->getId());         
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(6);
	gMessageFactory->addUint32(12); // Size, short short long
	gMessageFactory->addUint16(1);  // Update count
	gMessageFactory->addUint16(9);  // CREO6 TargetID
	gMessageFactory->addUint64(creatureObject->getTargetId());  // new target

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
} 

//======================================================================================================================
//
// Creature Deltas Type 6
// update: group invite ?
//

void MessageLib::sendInviteSenderUpdateDeltasCreo6(uint64 id, PlayerObject* targetPlayer)
{
	if(!(targetPlayer->isConnected()))
		return;

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(targetPlayer->getId());          
	gMessageFactory->addUint32(opCREO);
	gMessageFactory->addUint8(6);

	gMessageFactory->addUint32(20);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(7);
	gMessageFactory->addUint64(id);
	gMessageFactory->addUint64((uint64)targetPlayer->getClientTickCount());  // fake counter, otherwise window doesnt popup two times

	(targetPlayer->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetPlayer->getAccountId(),CR_Client,5);
} 

//======================================================================================================================
//
// Creature Deltas Type 6
// update: group
//

void MessageLib::sendGroupIdUpdateDeltasCreo6(uint64 groupId, const PlayerObject* const player, const PlayerObject* const target) const
{
	if(!(target->isConnected()))
		return;

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage); 
	gMessageFactory->addUint64(player->getId());         
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(6);
	gMessageFactory->addUint32(12); // Size, short short long
	gMessageFactory->addUint16(1);  // Update count
	gMessageFactory->addUint16(6);  // CREO6 GroupID
	gMessageFactory->addUint64(groupId);  // new id

	(target->getClient())->SendChannelA(gMessageFactory->EndMessage(),target->getAccountId(),CR_Client,5);
} 

//======================================================================================================================
//
// Creature Deltas Type 4
// update: terrain negotiation 
//

void MessageLib::sendTerrainNegotiation(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId()); 
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(4);
	gMessageFactory->addUint32(8); 

	gMessageFactory->addUint16(1);           
	gMessageFactory->addUint16(9);           
	gMessageFactory->addFloat(creatureObject->getCurrentTerrainNegotiation());          

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 4
// update: listen to
//

void MessageLib::sendListenToId(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId()); 
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(4);
	gMessageFactory->addUint32(12);           
	gMessageFactory->addUint16(1);           
	gMessageFactory->addUint16(6);          
	gMessageFactory->addUint64(creatureObject->getEntertainerListenToId());           

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: entertainer performance counter
//

void MessageLib::UpdateEntertainerPerfomanceCounter(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId()); 
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(6);
	gMessageFactory->addUint32(8);           
	gMessageFactory->addUint16(1);           
	gMessageFactory->addUint16(11);          
	gMessageFactory->addUint32(creatureObject->UpdatePerformanceCounter());           

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: performance id
//

void MessageLib::sendPerformanceId(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId()); 
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(6);
	gMessageFactory->addUint32(8);           
	gMessageFactory->addUint16(1);           
	gMessageFactory->addUint16(12);          
	gMessageFactory->addUint32(creatureObject->getPerformanceId());           

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: animation
//

void MessageLib::sendAnimationString(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId()); 
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(6);
	gMessageFactory->addUint32(6+creatureObject->getCurrentAnimation().getLength());           
	gMessageFactory->addUint16(1);           
	gMessageFactory->addUint16(3);          
	gMessageFactory->addString(creatureObject->getCurrentAnimation());           

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: mood
//

void MessageLib::sendMoodString(CreatureObject* creatureObject,string animation)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId()); 
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(6);
	gMessageFactory->addUint32(6 + animation.getLength());           
	gMessageFactory->addUint16(1);           
	gMessageFactory->addUint16(4);          
	gMessageFactory->addString(animation);           

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//send customization string update
//======================================================================================================================

void MessageLib::sendCustomizationUpdateCreo3(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId());          
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(6 + creatureObject->getCustomizationStr().getLength());
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(4);
	gMessageFactory->addString(creatureObject->getCustomizationStr());

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 3
// update: scale
//

void MessageLib::sendScaleUpdateCreo3(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(creatureObject->getId());          
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(3);

	gMessageFactory->addUint32(8);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint16(14);
	gMessageFactory->addFloat(creatureObject->getScale());

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: weapon id
//

void MessageLib::sendWeaponIdUpdate(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage); 
	gMessageFactory->addUint64(creatureObject->getId());         
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(6);
	gMessageFactory->addUint32(12); 
	gMessageFactory->addUint16(1);  
	gMessageFactory->addUint16(5);  

	if(Object* weapon = creatureObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Weapon))
	{
		gMessageFactory->addUint64(weapon->getId());
	}
	else
	{
		gMessageFactory->addUint64(0);
	}

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
} 

//======================================================================================================================
//
// Creature Deltas Type 3
// update: incapacitation timer
//

void MessageLib::sendIncapTimerUpdate(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opDeltasMessage); 
	gMessageFactory->addUint64(creatureObject->getId());         
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(3);
	gMessageFactory->addUint32(8); 
	gMessageFactory->addUint16(1);  
	gMessageFactory->addUint16(7);  

	gMessageFactory->addUint32((uint32)(creatureObject->getCurrentIncapTime() / 1000));

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
} 

//======================================================================================================================


//======================================================================================================================
//
// Creature Deltas Type 4
// update: skillmods
//
//TODO: This function doesn't seem to do anything, and how is it related to MessageLib::sendSkillModDeltasCREO_4
bool MessageLib::sendSkillModUpdateCreo4(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	// compute skillmod list size
	uint32			skillModByteCount	= 0;
	SkillModsList*	playerSkillMods		= playerObject->getSkillMods();

	SkillModsList::iterator it	= playerSkillMods->begin();


	//start the data part
	Message* data;
	
	gMessageFactory->StartMessage();             

	gMessageFactory->addUint16(1);						//nr of updates in the delta
	gMessageFactory->addUint16(3);						//position nr

	// skillmods

	gMessageFactory->addUint32(playerSkillMods->size());  
	gMessageFactory->addUint32(playerObject->getAndIncrementSkillModUpdateCounter(playerSkillMods->size()));  
	//playerObject->getAndIncrementSkillModUpdateCounter(1)

	//gMessageFactory->addUint16(playerSkillMods->size());

	while(it != playerSkillMods->end())
	{
		gMessageFactory->addUint8(0);
		gMessageFactory->addString(gSkillManager->getSkillModById((*it).first));
		gMessageFactory->addUint32((*it).second);
		gMessageFactory->addUint32(0);
		++it;
	}

	data = gMessageFactory->EndMessage();


	//now the actual message

	gMessageFactory->StartMessage();             
	gMessageFactory->addUint32(opDeltasMessage);  
	gMessageFactory->addUint64(playerObject->getId());           
	gMessageFactory->addUint32(opCREO);           
	gMessageFactory->addUint8(4);

	gMessageFactory->addUint32(data->getSize());	//bytecount
	gMessageFactory->addData(data->getData(),data->getSize());
	
	data->setPendingDelete(true);

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);
	return(true);

}