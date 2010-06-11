/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opBaselinesMessage);
	mMessageFactory->addUint64(player->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(1);

	// compute the skill list size
	uint32				skillByteCount	= 0;
	SkillList::iterator it				= playerSkills->begin();

	while(it != playerSkills->end())
	{
		skillByteCount += ((*it)->mName.getLength() + 2);

		++it;
	}

	mMessageFactory->addUint32(62 + skillByteCount);
	mMessageFactory->addUint16(4);

	// bank credits
	if(Bank* bank = dynamic_cast<Bank*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank)))
	{
		mMessageFactory->addUint32(bank->getCredits());
	}
	else
	{
		mMessageFactory->addUint32(0);
	}

	// inventory credits
	if(Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)))
	{
		mMessageFactory->addUint32(inventory->getCredits());
	}
	else
	{
		mMessageFactory->addUint32(0);
	}

	// ham maxs
	mMessageFactory->addUint32(9);
	mMessageFactory->addUint32(9);

	mMessageFactory->addUint32(playerHam->mHealth.getBaseHitPoints());
	mMessageFactory->addUint32(playerHam->mStrength.getBaseHitPoints());
	mMessageFactory->addUint32(playerHam->mConstitution.getBaseHitPoints());
	mMessageFactory->addUint32(playerHam->mAction.getBaseHitPoints());
	mMessageFactory->addUint32(playerHam->mQuickness.getBaseHitPoints());
	mMessageFactory->addUint32(playerHam->mStamina.getBaseHitPoints());
	mMessageFactory->addUint32(playerHam->mMind.getBaseHitPoints());
	mMessageFactory->addUint32(playerHam->mFocus.getBaseHitPoints());
	mMessageFactory->addUint32(playerHam->mWillpower.getBaseHitPoints());

	// skills
	mMessageFactory->addUint64(playerSkills->size());

	it = playerSkills->begin();

	while(it != playerSkills->end())
	{
		mMessageFactory->addString((*it)->mName);

		++it;
	}

	message = mMessageFactory->EndMessage();

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
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opBaselinesMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(3);
	mMessageFactory->addUint32(creoByteCount);
	mMessageFactory->addUint16(12);
	//0
	mMessageFactory->addUint32(16256); // unknown
	//1
	mMessageFactory->addString(creatureObject->getSpeciesGroup());
	mMessageFactory->addUint32(0);     // unknown
	mMessageFactory->addString(creatureObject->getSpeciesString());
	//2
	mMessageFactory->addString(fullName);
	//3
	mMessageFactory->addUint32(1); // unknown
	//4
	mMessageFactory->addString(creatureObject->getCustomizationStr());
	//5 unknown list
	mMessageFactory->addUint32(0); // unknown
	mMessageFactory->addUint32(0); // unknown

	//6
	mMessageFactory->addUint32(creatureObject->getTypeOptions());
	//7 incap timer
	mMessageFactory->addUint32(0); // unknown


	// For now, we use the Health bar when using a single H-bar.

	//8 condition damage (vehicle) //this is the amount of damage... used to set appearence of swoop
	mMessageFactory->addUint32(creatureHam->getPropertyValue(HamBar_Health,HamProperty_MaxHitpoints) - creatureHam->getPropertyValue(HamBar_Health,HamProperty_CurrentHitpoints));

	//9 max condition (vehicle)
	mMessageFactory->addUint32(creatureHam->getPropertyValue(HamBar_Health,HamProperty_MaxHitpoints));

	//10 locomotion ?
	mMessageFactory->addUint8(1); // unknown
	//11 posture
	mMessageFactory->addUint8(creatureObject->getPosture());
	//12
	mMessageFactory->addUint8(creatureObject->getFactionRank());
	//13 owner id
	if(creatureObject->getCreoGroup()  == CreoGroup_Vehicle)
	{
		MountObject* mount = dynamic_cast<MountObject*>(creatureObject);
		if(mount)
			mMessageFactory->addUint64(mount->owner());
		else
			mMessageFactory->addUint64(0);

		mMessageFactory->addFloat(creatureObject->getScale());
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint64(0);
	}
	else
	{
		mMessageFactory->addUint64(0);
		mMessageFactory->addFloat(creatureObject->getScale());
		mMessageFactory->addUint32(creatureHam->getBattleFatigue());
		mMessageFactory->addUint64(creatureObject->getState());
	}

	// ham wounds

	if(creatureObject->getCreoGroup() == CreoGroup_PersistentNpc)
	{
		mMessageFactory->addUint32(6);
		mMessageFactory->addUint32(creatureHam->getWoundsUpdateCounter());

		mMessageFactory->addUint32(0);
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint32(0);
	}
	else if(creatureObject->getCreoGroup() == CreoGroup_Vehicle)
	{
		//no wounds for vehicles
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint32(0);
	}
	else
	{
		mMessageFactory->addUint32(9);
		//ok make sure updatecounter gets send as 9 on baseline
		mMessageFactory->addUint32(creatureHam->getWoundsUpdateCounter());

		mMessageFactory->addUint32(creatureHam->mHealth.getWounds());
		mMessageFactory->addUint32(creatureHam->mStrength.getWounds());
		mMessageFactory->addUint32(creatureHam->mConstitution.getWounds());
		mMessageFactory->addUint32(creatureHam->mAction.getWounds());
		mMessageFactory->addUint32(creatureHam->mQuickness.getWounds());
		mMessageFactory->addUint32(creatureHam->mStamina.getWounds());
		mMessageFactory->addUint32(creatureHam->mMind.getWounds());
		mMessageFactory->addUint32(creatureHam->mFocus.getWounds());
		mMessageFactory->addUint32(creatureHam->mWillpower.getWounds());
	}

	message = mMessageFactory->EndMessage();

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
	mMessageFactory->StartMessage();

	mMessageFactory->addUint16(14);

	mMessageFactory->addFloat(1.0f); // acceleration base
	mMessageFactory->addFloat(1.0f); // acceleration mod

	// ham encumbrance
	mMessageFactory->addUint32(3);
	mMessageFactory->addUint32(3);

	mMessageFactory->addUint32(playerHam->mHealth.getEncumbrance());
	mMessageFactory->addUint32(playerHam->mAction.getEncumbrance());
	mMessageFactory->addUint32(playerHam->mMind.getEncumbrance());

	// skillmods
	mMessageFactory->addUint32(playerSkillMods->size());
	player->mSkillModUpdateCounter += playerSkillMods->size();
	mMessageFactory->addUint32(player->mSkillModUpdateCounter);

	SkillModsList::iterator it = playerSkillMods->begin();

	while(it != playerSkillMods->end())
	{
		mMessageFactory->addUint8(0);
		mMessageFactory->addString(gSkillManager->getSkillModById((*it).first));
		mMessageFactory->addUint32((*it).second);
		mMessageFactory->addUint32(0);

		++it;
	}

	// 4 Speedmultiplier base
	mMessageFactory->addFloat(1.0f);

	//5 Speedmultiplier mod
	mMessageFactory->addFloat(1.0f);

	//6 Listen to Id
	mMessageFactory->addUint64(player->getEntertainerListenToId());

	//7
	mMessageFactory->addFloat(player->getCurrentRunSpeedLimit()); //RUN speed

	//8
	mMessageFactory->addFloat(1.02f);	// slopemod angle

	//9
	mMessageFactory->addFloat(player->getCurrentTerrainNegotiation());

	//10 (a)
	mMessageFactory->addFloat(player->getCurrentTurnRate());

	//11(b)
	mMessageFactory->addFloat(player->getCurrentAcceleration()); // This is the walk speed, nothing else.

	mMessageFactory->addFloat(0.0125f);	// unknown
	mMessageFactory->addUint64(0);	// unknown

	Message* data = mMessageFactory->EndMessage();


	//Now the Message header

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opBaselinesMessage);
	mMessageFactory->addUint64(player->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(4);

	mMessageFactory->addUint32(data->getSize());
	mMessageFactory->addData(data->getData(),data->getSize());
	data->setPendingDelete(true);

	(player->getClient())->SendChannelA(mMessageFactory->EndMessage(),player->getAccountId(),CR_Client,3);

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

	ObjectList::iterator eqIt = equippedObjects->begin();

	
	mMessageFactory->StartMessage();

	mMessageFactory->addUint16(22);

	mMessageFactory->addUint32(creatureObject->getSubZoneId());

	// defenders
	mMessageFactory->addUint32(defenders->size());
	mMessageFactory->addUint32(creatureObject->mDefenderUpdateCounter);

	ObjectIDList::iterator defenderIt = defenders->begin();

	while(defenderIt != defenders->end())
	{
		//mMessageFactory->addUint64((*defenderIt)->getId());
		mMessageFactory->addUint64(*defenderIt);
		++defenderIt;
	}

	mMessageFactory->addUint16(creatureObject->getCL());
	mMessageFactory->addString(creatureObject->getCurrentAnimation());   // music/dance string here - current animation
	mMessageFactory->addString(moodStr);

	if(Object* weapon = creatureObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hold_Left))
	{
		mMessageFactory->addUint64(weapon->getId());
	}
	else
	{
		mMessageFactory->addUint64(0);
	}

	//6 Group Id
	mMessageFactory->addUint64(creatureObject->getGroupId());
	mMessageFactory->addUint64(0);   // Invite sender Id
	mMessageFactory->addUint64(0);   // Invite Counter
	mMessageFactory->addUint32(0);   // guild Id

	//9
	mMessageFactory->addUint64(creatureObject->getTargetId());

	//a
	mMessageFactory->addUint8(creatureObject->getMoodId());

	//b
	if(creatureObject->getCreoGroup() == CreoGroup_Vehicle)
	{
		mMessageFactory->addUint32(0);
	}
	else
	{
		mMessageFactory->addUint32(creatureObject->UpdatePerformanceCounter());   // unknown
	}

	//c thats not performance id to be used with dancing
	//use with music only
	mMessageFactory->addUint32(creatureObject->getPerformanceId());   // performance id

	// current ham
	if(creatureObject->getCreoGroup() == CreoGroup_PersistentNpc)
	{
		if(!creatureHam->getCurrentHitpointsUpdateCounter())
		{
			creatureHam->advanceCurrentHitpointsUpdateCounter(6);
		}

		mMessageFactory->addUint32(6);
		mMessageFactory->addUint32(creatureHam->getCurrentHitpointsUpdateCounter());

		mMessageFactory->addUint32(creatureHam->mHealth.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mStrength.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mConstitution.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mAction.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mQuickness.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mStamina.getCurrentHitPoints());

		// max ham
		if(!creatureHam->getMaxHitpointsUpdateCounter())
		{
			creatureHam->advanceMaxHitpointsUpdateCounter(6);
		}

		mMessageFactory->addUint32(6);
		mMessageFactory->addUint32(creatureHam->getMaxHitpointsUpdateCounter());

		mMessageFactory->addUint32(creatureHam->mHealth.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mStrength.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mConstitution.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mAction.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mQuickness.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mStamina.getMaxHitPoints());
	}
	else if(creatureObject->getCreoGroup() == CreoGroup_Vehicle)
	{
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint32(0);

	}
	else
	{
		if(!creatureHam->getCurrentHitpointsUpdateCounter())
		{
			creatureHam->advanceCurrentHitpointsUpdateCounter(9);
		}

		mMessageFactory->addUint32(9);
		mMessageFactory->addUint32(creatureHam->getCurrentHitpointsUpdateCounter());

		mMessageFactory->addUint32(creatureHam->mHealth.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mStrength.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mConstitution.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mAction.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mQuickness.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mStamina.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mMind.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mFocus.getCurrentHitPoints());
		mMessageFactory->addUint32(creatureHam->mWillpower.getCurrentHitPoints());

		// max ham
		if(!creatureHam->getMaxHitpointsUpdateCounter())
		{
			creatureHam->advanceMaxHitpointsUpdateCounter(9);
		}

		mMessageFactory->addUint32(9);
		mMessageFactory->addUint32(creatureHam->getMaxHitpointsUpdateCounter());

		mMessageFactory->addUint32(creatureHam->mHealth.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mStrength.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mConstitution.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mAction.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mQuickness.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mStamina.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mMind.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mFocus.getMaxHitPoints());
		mMessageFactory->addUint32(creatureHam->mWillpower.getMaxHitPoints());
	}

	// creatures tangible objects	 ->equipped list
	eqIt = equippedObjects->begin();

	mMessageFactory->addUint32(equippedObjects->size());
	creatureObject->getEquipManager()->setEquippedObjectsUpdateCounter(0);
	mMessageFactory->addUint32(creatureObject->getEquipManager()->getEquippedObjectsUpdateCounter());

	while(eqIt != equippedObjects->end())
	{
		Object* object = (*eqIt);

		if(TangibleObject* tObject = dynamic_cast<TangibleObject*>(object))
		{
			mMessageFactory->addString(tObject->getCustomizationStr());
		}
		else if(CreatureObject* pet = dynamic_cast<CreatureObject*>(object)) //support for mounts
		{
			mMessageFactory->addString(pet->getCustomizationStr());
		}
		else
		{
			mMessageFactory->addUint16(0);
		}

		mMessageFactory->addUint32(4);
		mMessageFactory->addUint64(object->getId());
		mMessageFactory->addUint32((object->getModelString()).getCrc());

		++eqIt;
	}

	mMessageFactory->addUint16(0); // unknown
	mMessageFactory->addUint8(0);  // extra byte that was needed to correct movement

   	Message* data = mMessageFactory->EndMessage();


	//Now the Message header
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opBaselinesMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);

	mMessageFactory->addUint32(data->getSize());
	mMessageFactory->addData(data->getData(),data->getSize());
	data->setPendingDelete(true);

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opUpdatePostureMessage);
	mMessageFactory->addUint8(creatureObject->getPosture());
	mMessageFactory->addUint64(creatureObject->getId());

	message = mMessageFactory->EndMessage();

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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);

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
		gLogger->log(LogManager::DEBUG,"MessageLib::sendDefenderUpdate Invalid option = %u",updateType);
		return;
	}

	mMessageFactory->addUint32(payloadSize);

	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(1);

	mMessageFactory->addUint32(1);
	mMessageFactory->addUint32(++creatureObject->mDefenderUpdateCounter);

	mMessageFactory->addUint8(updateType);

	if (updateType == 0)
	{
		mMessageFactory->addUint16(index);
	}
	else if ((updateType == 1) || (updateType == 2))
	{
		mMessageFactory->addUint16(index);
		mMessageFactory->addUint64(defenderId);
	}
	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);

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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);

	mMessageFactory->addUint32(byteCount + (defenders->size() * 8));
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(1);

	ObjectIDList::iterator defenderIt = defenders->begin();
	// Shall we not advance the updatecounter if we send a reset, where size() is 0?

	// I'm pretty sure the idea of update counters is to let the client know that somethings have changed,
	// and to know in what order, given several messages "at once".
	// creatureObject->mDefenderUpdateCounter = creatureObject->mDefenderUpdateCounter + defenders->size();
	// mMessageFactory->addUint32(++creatureObject->mDefenderUpdateCounter);

	if(!defenders->size())
	{
		// Even an update with zero defenders is a new update.
		mMessageFactory->addUint32(1);

		mMessageFactory->addUint32(++creatureObject->mDefenderUpdateCounter);
		mMessageFactory->addUint8(4);
	}
	else
	{
		mMessageFactory->addUint32(defenders->size());
		// mMessageFactory->addUint32(1);

		mMessageFactory->addUint32(++creatureObject->mDefenderUpdateCounter);
		mMessageFactory->addUint8(3);
		mMessageFactory->addUint16(defenders->size());

		while (defenderIt != defenders->end())
		{
			mMessageFactory->addUint64((*defenderIt));
			++defenderIt;
		}
	}

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);

	mMessageFactory->addUint32(15 + (equippedObjects->size() * 18)+ cSize);
	mMessageFactory->addUint16(1);   //one update
	mMessageFactory->addUint16(15);				 //id 15

	// creatures tangible objects
	eqIt = equippedObjects->begin();

	mMessageFactory->addUint32(equippedObjects->size());
	mMessageFactory->addUint32(creatureObject->getEquipManager()->advanceEquippedObjectsUpdateCounter(equippedObjects->size()));//+1
	creatureObject->getEquipManager()->advanceEquippedObjectsUpdateCounter(1);

	mMessageFactory->addUint8(3);
	mMessageFactory->addUint16(equippedObjects->size());

	while(eqIt != equippedObjects->end())
	{
		Object* object = (*eqIt);

		if(TangibleObject* tObject = dynamic_cast<TangibleObject*>(object))
		{
			mMessageFactory->addString(tObject->getCustomizationStr());
		}
		else if(CreatureObject* pet = dynamic_cast<CreatureObject*>(object))
		{
			mMessageFactory->addString(pet->getCustomizationStr());
		}
		else
		{
			mMessageFactory->addUint16(0);
		}

		mMessageFactory->addUint32(4);
		mMessageFactory->addUint64(object->getId());
		mMessageFactory->addUint32((object->getModelString()).getCrc());

		++eqIt;
	}

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);

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
		gLogger->log(LogManager::DEBUG,"MessageLib::sendEquippedItemUpdate_InRange : Item not found : %I64u",itemId);
		return false;
	}

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);

	mMessageFactory->addUint32(15 + (1 * 18)+ cSize);
	mMessageFactory->addUint16(1);   //one update
	mMessageFactory->addUint16(15);				 //id 15

	// creatures tangible objects
	eqIt = equippedObjects->begin();

	mMessageFactory->addUint32(1);	//only one item gets updated
	mMessageFactory->addUint32(creatureObject->getEquipManager()->advanceEquippedObjectsUpdateCounter(1));//+1
	creatureObject->getEquipManager()->advanceEquippedObjectsUpdateCounter(1);

	mMessageFactory->addUint8(2);  //2 for change a given entry
	mMessageFactory->addUint16(index);//index of the entry

	while(eqIt != equippedObjects->end())
	{
		Object* object = (*eqIt);
		if ( object->getId() == itemId)
		{
			if(TangibleObject* tObject = dynamic_cast<TangibleObject*>(object))
			{
				mMessageFactory->addString(tObject->getCustomizationStr());
			}
			else if(CreatureObject* pet = dynamic_cast<CreatureObject*>(object))
			{
				mMessageFactory->addString(pet->getCustomizationStr());
			}
			else
			{
				mMessageFactory->addUint16(0);
			}

			mMessageFactory->addUint32(4);
			mMessageFactory->addUint64(object->getId());
			mMessageFactory->addUint32((object->getModelString()).getCrc());
			break;
		}
		++eqIt;
	}

	Message* message = mMessageFactory->EndMessage();
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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opUpdatePvpStatusMessage);

	if (!statusMask)
	{
		mMessageFactory->addUint32(creatureObject->getPvPStatus());
	}
	else
	{
		mMessageFactory->addUint32(statusMask);
	}

	// neutral
	if(creatureObject->getFaction().getCrc() == 0x1fdc3051)
		mMessageFactory->addUint32(0);
	else
		mMessageFactory->addUint32(creatureObject->getFaction().getCrc());

	mMessageFactory->addUint64(creatureObject->getId());

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,4);

	return(true);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: mood
//

void MessageLib::sendMoodUpdate(CreatureObject* srcObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);

	mMessageFactory->addUint32(5);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(10);
	mMessageFactory->addUint8(srcObject->getMoodId());

	_sendToInRange(mMessageFactory->EndMessage(),srcObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: posture
//

void MessageLib::sendPostureUpdate(CreatureObject* creatureObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(3);
	mMessageFactory->addUint32(5);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(11);
	mMessageFactory->addUint8(creatureObject->getPosture());

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5,true);
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
		mMessageFactory->StartMessage();
		mMessageFactory->addUint32(opDeltasMessage);
		mMessageFactory->addUint64(creatureObject->getId());
		mMessageFactory->addUint32(opCREO);
		mMessageFactory->addUint8(3);
		mMessageFactory->addUint32(15);
		mMessageFactory->addUint16(2);
		mMessageFactory->addUint16(11);
		mMessageFactory->addUint8(creatureObject->getPosture());
		mMessageFactory->addUint16(16);
		mMessageFactory->addUint64(creatureObject->getState());

		_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
	}
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: state
//

void MessageLib::sendStateUpdate(CreatureObject* creatureObject)
{
	// Test code for npc combat with objects that can have no states, like debris.
	if (creatureObject->getCreoGroup() != CreoGroup_AttackableObject)
	{
		mMessageFactory->StartMessage();
		mMessageFactory->addUint32(opDeltasMessage);
		mMessageFactory->addUint64(creatureObject->getId());
		mMessageFactory->addUint32(opCREO);
		mMessageFactory->addUint8(3);
		mMessageFactory->addUint32(12);
		mMessageFactory->addUint16(1);
		mMessageFactory->addUint16(16);
		mMessageFactory->addUint64(creatureObject->getState());

		_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
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

		mMessageFactory->StartMessage();
		mMessageFactory->addUint32(opDeltasMessage);
		mMessageFactory->addUint64(creatureObject->getId());
		mMessageFactory->addUint32(opCREO);
		mMessageFactory->addUint8(3);
		mMessageFactory->addUint32(8); // bytes
		mMessageFactory->addUint16(1);	// No of items
		mMessageFactory->addUint16(8);	// Index 8 condition damage (vehicle)
		uint32 damage = ham->getPropertyValue(HamBar_Health,HamProperty_MaxHitpoints);
		damage -= ham->getPropertyValue(HamBar_Health,HamProperty_CurrentHitpoints);
		// mMessageFactory->addUint32(ham->getPropertyValue(HamBar_Health,HamProperty_CurrentHitpoints));
		mMessageFactory->addUint32(damage);
		_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(1);
	mMessageFactory->addUint32(8);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(0);

	if(Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank)))
	{
		mMessageFactory->addUint32(bank->getCredits());
	}
	else
	{
		mMessageFactory->addUint32(0);
	}

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(1);
	mMessageFactory->addUint32(8);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(1);

	if(Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)))
	{
		mMessageFactory->addUint32(inventory->getCredits());
	}
	else
	{
		mMessageFactory->addUint32(0);
	}

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

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

	MovingObject* object = dynamic_cast<MovingObject*>(playerObject);

	if (playerObject->checkIfMounted()) {
		object = dynamic_cast<MovingObject*>(playerObject->getMount());
	}

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(4);
	mMessageFactory->addUint32(26);

	mMessageFactory->addUint16(4);

	mMessageFactory->addUint16(5);
	mMessageFactory->addFloat(object->getCurrentSpeedModifier());

	mMessageFactory->addUint16(7);
	mMessageFactory->addFloat(object->getCurrentRunSpeedLimit());

	mMessageFactory->addUint16(10);
	mMessageFactory->addFloat(object->getCurrentTurnRate());

	mMessageFactory->addUint16(11);
	mMessageFactory->addFloat(object->getCurrentAcceleration());

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(targetObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(1);

	mMessageFactory->addUint32(15 + skill->mName.getLength());
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(3);

	mMessageFactory->addUint32(1); // listsize
	mMessageFactory->addUint32(targetObject->getAndIncrementSkillUpdateCounter());
	mMessageFactory->addUint8(action);
	mMessageFactory->addString(skill->mName);

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(4);

	mMessageFactory->addUint32(12 + skillModByteCount);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(3);

	mMessageFactory->addUint32(smList.size());

	mMessageFactory->addUint32(playerObject->getAndIncrementSkillModUpdateCounter(smList.size()));
	mMessageFactory->addUint8(remove);

	it = smList.begin();
	while(it != smList.end())
	{
		mMessageFactory->addUint8(0);
		mMessageFactory->addString(gSkillManager->getSkillModById((*it).first));
		mMessageFactory->addUint32((*it).second);
		mMessageFactory->addUint32(0);

		++it;
	}

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);

	mMessageFactory->addUint32(19);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(13);

	// advance by 1 (overload for bigger increment)
	ham->advanceCurrentHitpointsUpdateCounter();
	mMessageFactory->addUint32(1);
	mMessageFactory->addUint32(ham->getCurrentHitpointsUpdateCounter());

	mMessageFactory->addUint8(2);
	mMessageFactory->addUint16(barIndex);
	mMessageFactory->addInt32(ham->getPropertyValue(barIndex,HamProperty_CurrentHitpoints));

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);

	mMessageFactory->addUint32(19);

	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(14);//delta nr

	mMessageFactory->addUint32(1);
	ham->advanceMaxHitpointsUpdateCounter();  // increment list up counter by 1
	mMessageFactory->addUint32(ham->getMaxHitpointsUpdateCounter());

	mMessageFactory->addUint8(2);
	mMessageFactory->addUint16(barIndex);
	mMessageFactory->addInt32(ham->getPropertyValue(barIndex,HamProperty_MaxHitpoints));

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(1);

	mMessageFactory->addUint32(19);

	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(2);//delta nr

	mMessageFactory->addUint32(1);
	ham->advanceBaseHitpointsUpdateCounter();
	mMessageFactory->addUint32(ham->getBaseHitpointsUpdateCounter());  // increment list up counter by 1

	mMessageFactory->addUint8(2);
	mMessageFactory->addUint16(barIndex);
	mMessageFactory->addInt32(ham->getPropertyValue(barIndex,HamProperty_BaseHitpoints));

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(3);

	mMessageFactory->addUint32(19);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(17);

	ham->advanceWoundsUpdateCounter();
	mMessageFactory->addUint32(1);
	mMessageFactory->addUint32(ham->getWoundsUpdateCounter());

	mMessageFactory->addUint8(2);
	mMessageFactory->addUint16(barIndex);
	mMessageFactory->addInt32(ham->getPropertyValue(barIndex,HamProperty_Wounds));

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);

	mMessageFactory->addUint32(33);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(13);

	mMessageFactory->addUint32(3);

	ham->advanceCurrentHitpointsUpdateCounter(3);

	mMessageFactory->addUint32(ham->getCurrentHitpointsUpdateCounter());

	mMessageFactory->addUint8(2);
	mMessageFactory->addUint16(HamBar_Health);
	mMessageFactory->addInt32(ham->getPropertyValue(HamBar_Health,HamProperty_CurrentHitpoints));

	mMessageFactory->addUint8(2);
	mMessageFactory->addUint16(HamBar_Action);
	mMessageFactory->addInt32(ham->getPropertyValue(HamBar_Action,HamProperty_CurrentHitpoints));

	mMessageFactory->addUint8(2);
	mMessageFactory->addUint16(HamBar_Mind);
	mMessageFactory->addInt32(ham->getPropertyValue(HamBar_Mind,HamProperty_CurrentHitpoints));

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(3);

	mMessageFactory->addUint32(8);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(15);

	mMessageFactory->addInt32(ham->getBattleFatigue());

	(pObject)->getClient()->SendChannelA(mMessageFactory->EndMessage(),pObject->getAccountId(),CR_Client,5);
}

//======================================================================================================================
//
// Creature Deltas Type 3
// update: owner id
// used for mountable creatures (pets, vehicles..)


void MessageLib::sendOwnerUpdateCreo3(MountObject* mount)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(mount->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(3);

	mMessageFactory->addUint32(12);
	mMessageFactory->addUint16(2);
	mMessageFactory->addUint16(13); // CREO 3 owner id

	mMessageFactory->addInt64(mount->owner());

	_sendToInRange(mMessageFactory->EndMessage(),mount,5);
	//(pObject)->getClient()->SendChannelA(mMessageFactory->EndMessage(),pObject->getAccountId(),CR_Client,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: target
//

void MessageLib::sendTargetUpdateDeltasCreo6(CreatureObject* creatureObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);
	mMessageFactory->addUint32(12); // Size, short short long
	mMessageFactory->addUint16(1);  // Update count
	mMessageFactory->addUint16(9);  // CREO6 TargetID
	mMessageFactory->addUint64(creatureObject->getTargetId());  // new target

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(targetPlayer->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);

	mMessageFactory->addUint32(20);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(7);
	mMessageFactory->addUint64(id);
	mMessageFactory->addUint64((uint64)targetPlayer->getClientTickCount());  // fake counter, otherwise window doesnt popup two times

	(targetPlayer->getClient())->SendChannelA(mMessageFactory->EndMessage(),targetPlayer->getAccountId(),CR_Client,5);
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

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(player->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);
	mMessageFactory->addUint32(12); // Size, short short long
	mMessageFactory->addUint16(1);  // Update count
	mMessageFactory->addUint16(6);  // CREO6 GroupID
	mMessageFactory->addUint64(groupId);  // new id

	(target->getClient())->SendChannelA(mMessageFactory->EndMessage(),target->getAccountId(),CR_Client,5);
}

//======================================================================================================================
//
// Creature Deltas Type 4
// update: terrain negotiation
//

void MessageLib::sendTerrainNegotiation(CreatureObject* creatureObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(4);
	mMessageFactory->addUint32(8);

	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(9);
	mMessageFactory->addFloat(creatureObject->getCurrentTerrainNegotiation());

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 4
// update: listen to
//

void MessageLib::sendListenToId(PlayerObject* playerObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(4);
	mMessageFactory->addUint32(12);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(6);
	mMessageFactory->addUint64(playerObject->getEntertainerListenToId());

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);
	//_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: entertainer performance counter
//

void MessageLib::UpdateEntertainerPerfomanceCounter(CreatureObject* creatureObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);
	mMessageFactory->addUint32(8);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(11);
	mMessageFactory->addUint32(creatureObject->UpdatePerformanceCounter());

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: performance id
//

void MessageLib::sendPerformanceId(CreatureObject* creatureObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);
	mMessageFactory->addUint32(8);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(12);
	mMessageFactory->addUint32(creatureObject->getPerformanceId());

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: animation
//

void MessageLib::sendAnimationString(CreatureObject* creatureObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);
	mMessageFactory->addUint32(6+creatureObject->getCurrentAnimation().getLength());
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(3);
	mMessageFactory->addString(creatureObject->getCurrentAnimation());

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: mood
//

void MessageLib::sendMoodString(CreatureObject* creatureObject,string animation)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);
	mMessageFactory->addUint32(6 + animation.getLength());
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(4);
	mMessageFactory->addString(animation);

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//send customization string update
//======================================================================================================================

void MessageLib::sendCustomizationUpdateCreo3(CreatureObject* creatureObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(3);

	mMessageFactory->addUint32(6 + creatureObject->getCustomizationStr().getLength());
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(4);
	mMessageFactory->addString(creatureObject->getCustomizationStr());

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 3
// update: scale
//

void MessageLib::sendScaleUpdateCreo3(CreatureObject* creatureObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(3);

	mMessageFactory->addUint32(8);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(14);
	mMessageFactory->addFloat(creatureObject->getScale());

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: weapon id
//

void MessageLib::sendWeaponIdUpdate(CreatureObject* creatureObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);
	mMessageFactory->addUint32(12);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(5);

	if(Object* weapon = creatureObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hold_Left))
	{
		mMessageFactory->addUint64(weapon->getId());
	}
	else
	{
		mMessageFactory->addUint64(0);
	}

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 3
// update: incapacitation timer
//

void MessageLib::sendIncapTimerUpdate(CreatureObject* creatureObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(3);
	mMessageFactory->addUint32(8);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(7);

	mMessageFactory->addUint32((uint32)(creatureObject->getCurrentIncapTime() / 1000));

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
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
	//uint32			skillModByteCount	= 0;
	SkillModsList*	playerSkillMods		= playerObject->getSkillMods();

	SkillModsList::iterator it	= playerSkillMods->begin();


	//start the data part
	Message* data;

	mMessageFactory->StartMessage();

	mMessageFactory->addUint16(1);						//nr of updates in the delta
	mMessageFactory->addUint16(3);						//position nr

	// skillmods

	mMessageFactory->addUint32(playerSkillMods->size());
	mMessageFactory->addUint32(playerObject->getAndIncrementSkillModUpdateCounter(playerSkillMods->size()));
	//playerObject->getAndIncrementSkillModUpdateCounter(1)

	//mMessageFactory->addUint16(playerSkillMods->size());

	while(it != playerSkillMods->end())
	{
		mMessageFactory->addUint8(0);
		mMessageFactory->addString(gSkillManager->getSkillModById((*it).first));
		mMessageFactory->addUint32((*it).second);
		mMessageFactory->addUint32(0);
		++it;
	}

	data = mMessageFactory->EndMessage();


	//now the actual message

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(4);

	mMessageFactory->addUint32(data->getSize());	//bytecount
	mMessageFactory->addData(data->getData(),data->getSize());

	data->setPendingDelete(true);

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);
	return(true);

}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: Staitionary Flag
//

void MessageLib::sendStationaryFlagUpdate(PlayerObject* playerObject)
{
	mMessageFactory->StartMessage();
	
	mMessageFactory->addUint32(opDeltasMessage);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(opCREO);
	mMessageFactory->addUint8(6);
	mMessageFactory->addUint32(5);
	mMessageFactory->addUint16(1);
	mMessageFactory->addUint16(17);
	
	if(playerObject->isStationary())
		mMessageFactory->addUint8(1);
	else
		mMessageFactory->addUint8(0);

	playerObject->getClient()->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);
}
