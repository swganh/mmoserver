/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include <anh\app\swganh_kernel.h>
#include <anh\service/service_manager.h>
#include "ZoneServer\Services\ham\ham_service.h"

#include "ZoneServer/Objects/Bank.h"
#include "ZoneServer/Objects/Inventory.h"
#include "ZoneServer/Objects/Object/ObjectFactory.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/Objects/Wearable.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"

#include "ZoneServer\Services\equipment\equipment_service.h"
#include "ZoneServer\Objects\Creature Object\equipment_item.h"

#include "anh/logger.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

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
    //SkillList*	playerSkills	= player->getSkills();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint16(4);

    // bank credits
auto bank_object = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService")->GetEquippedObject(player, "bank");
    if(Bank* bank = dynamic_cast<Bank*>(bank_object)) {
        mMessageFactory->addUint32(bank->getCredits());
    }
    else
    {
LOG (error) << "MessageLib::sendBaselinesCREO_1 :: No Bank Object for " << player->getId();
        mMessageFactory->addUint32(0);
    }

    // inventory credits
auto inventory_object = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService")->GetEquippedObject(player, "inventory");
    if(Inventory* inventory = dynamic_cast<Inventory*>(inventory_object)) {
        mMessageFactory->addUint32(inventory->getCredits());
    }
    else
    {
LOG (error) << "MessageLib::sendBaselinesCREO_1 :: No Inventory Object for " << player->getId();
        mMessageFactory->addUint32(0);
    }

    // ham maxs
swganh::messages::BaselinesMessage baseline_message;
player->SerializeMaxStats(&baseline_message);

mMessageFactory->addData(baseline_message.data.data(),baseline_message.data.size());

    // skills
	auto skilllist		= player->GetSkills(); 
	auto skillIt		= skilllist.begin();

	mMessageFactory->addUint32(skilllist.size());
	mMessageFactory->addUint32(0);

    while(skillIt != skilllist.end())
    {
		mMessageFactory->addString((*skillIt));

        skillIt++;
    }

    message = mMessageFactory->EndMessage();

    
mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(player->getId());
    mMessageFactory->addUint32(opCREO);
    mMessageFactory->addUint8(1);

mMessageFactory->addUint32(message->getSize());
mMessageFactory->addData(message->getData(), message->getSize());
message->setPendingDelete(true);

(player->getClient())->SendChannelA(mMessageFactory->EndMessage(), player->getAccountId(), CR_Client, 3);

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

    Message*	message;
    
    // make sure we got a name
std::stringstream stream;
stream << creatureObject->getFirstName() << " " << creatureObject->getLastName();
std::string s(stream.str());
std::u16string custom_name_u16(s.begin(), s.end());


    mMessageFactory->StartMessage();
    
    mMessageFactory->addUint16(12); //Object Operand count
    //0
    mMessageFactory->addUint32(16256); // float complexity
    //1
    mMessageFactory->addString(creatureObject->getSpeciesGroup());
    mMessageFactory->addUint32(0); // unknown
    mMessageFactory->addString(creatureObject->getSpeciesString());
    //2
    mMessageFactory->addString(custom_name_u16);
    //3
    mMessageFactory->addUint32(1); //volume
    //4
    mMessageFactory->addString(creatureObject->getCustomizationStr());
   
//5 customization list
    mMessageFactory->addUint32(0); //
    mMessageFactory->addUint32(0); //

    //6
    mMessageFactory->addUint32(creatureObject->getTypeOptions());
    //7 incap timer
    mMessageFactory->addUint32(0); // unknown

    //8 condition damage (vehicle) //this is the amount of damage... used to set appearence of swoop

//auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");
mMessageFactory->addUint32(creatureObject->GetStatMax(HamBar_Health) - creatureObject->GetStatCurrent(HamBar_Health));

    //9 max condition (vehicle)
    mMessageFactory->addUint32(creatureObject->GetStatMax(HamBar_Health));

    //10 locomotion ??
    mMessageFactory->addUint8(1);
    //11 posture
    mMessageFactory->addUint8(creatureObject->GetPosture());
    //12
    mMessageFactory->addUint8(creatureObject->getFactionRank());
    //13 owner id
    if(creatureObject->getCreoGroup() == CreoGroup_Vehicle)
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
mMessageFactory->addUint32(creatureObject->GetBattleFatigue());
        mMessageFactory->addUint64(creatureObject->states.getAction());
    }

    // ham wounds

swganh::messages::BaselinesMessage baseline_message;
creatureObject->SerializeStatWounds(&baseline_message);

mMessageFactory->addData(baseline_message.data.data(),baseline_message.data.size());

    message = mMessageFactory->EndMessage();

mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(creatureObject->getId());
    mMessageFactory->addUint32(opCREO);
    mMessageFactory->addUint8(3);
mMessageFactory->addUint32(message->getSize());
mMessageFactory->addData(message->getData(), message->getSize());
message->setPendingDelete(true);

(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

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

    SkillModsList*	playerSkillMods = player->getSkillMods();

    //thats the message databody
    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(14);

    mMessageFactory->addFloat(1.0f); // acceleration base
    mMessageFactory->addFloat(1.0f); // acceleration mod

    // ham encumbrance
swganh::messages::BaselinesMessage baseline_message;
player->SerializeStatEncumberances(&baseline_message);
mMessageFactory->addData(baseline_message.data.data(),baseline_message.data.size());

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

    // If no mood is set, use neutral for avatar / npc, then they will look less angry as default.
    // This will NOT affect the chat-mood
    // BString moodStr = gWorldManager->getMood(creatureObject->getMoodId());
    uint8 moodId = creatureObject->getMoodId();
    if (moodId == 0)
    {
        moodId = 74;
    }

    std::string	moodStr	= gWorldManager->getMood(moodId);

    auto	defenders	= creatureObject->GetDefender();

    //ObjectList::iterator eqIt = equippedObjects.begin();


    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(22);

    mMessageFactory->addUint32(creatureObject->getSubZoneId());

    // defenders
    mMessageFactory->addUint32(defenders.size());
    mMessageFactory->addUint32(creatureObject->GetDefenderCounter());

    auto defenderIt = defenders.begin();

    while(defenderIt != defenders.end())
    {
        mMessageFactory->addUint64(*defenderIt);
        ++defenderIt;
    }

    mMessageFactory->addUint16(creatureObject->getCL());
    mMessageFactory->addString(creatureObject->getCurrentAnimation()); // music/dance string here - current animation
    mMessageFactory->addString(moodStr);

    mMessageFactory->addUint64(creatureObject->GetWeaponId());

    //6 Group Id
    mMessageFactory->addUint64(creatureObject->getGroupId());
    mMessageFactory->addUint64(0); // Invite sender Id
    mMessageFactory->addUint64(0); // Invite Counter
    mMessageFactory->addUint32(0); // guild Id

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
        mMessageFactory->addUint32(creatureObject->UpdatePerformanceCounter()); // unknown
    }

    //c thats not performance id to be used with dancing
    //use with music only
    mMessageFactory->addUint32(creatureObject->getPerformanceId()); // performance id

    // current ham
swganh::messages::BaselinesMessage baseline_message;
creatureObject->SerializeCurrentStats(&baseline_message);

mMessageFactory->addData(baseline_message.data.data(),baseline_message.data.size());

// max ham
swganh::messages::BaselinesMessage haxham_message;
creatureObject->SerializeMaxStats(&haxham_message);

mMessageFactory->addData(haxham_message.data.data(),haxham_message.data.size());

    
    // creatures tangible objects ->equipped list
swganh::messages::BaselinesMessage equipment_baseline_message;
creatureObject->SerializeEquipment(&equipment_baseline_message);	// Equipment
mMessageFactory->addData(equipment_baseline_message.data.data(),equipment_baseline_message.data.size());



    mMessageFactory->addUint16(0); // unknown set object template string
    mMessageFactory->addUint8(0); // extra byte that was needed to correct movement stationary byte

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
    mMessageFactory->addUint8(creatureObject->GetPosture());

    _sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// Creature Deltas Type 6
// update: posture and state
//

void MessageLib::sendPostureAndStateUpdate(CreatureObject* creatureObject)
{
    // Test code for npc combat with objects that can have no states, like debris.

	MessageFactory* factory = getFactory_();

    if (creatureObject->getCreoGroup() != CreoGroup_AttackableObject)
    {
        factory->StartMessage();
        factory->addUint32(opDeltasMessage);
        factory->addUint64(creatureObject->getId());
        factory->addUint32(opCREO);
        factory->addUint8(3);
        factory->addUint32(15);
        factory->addUint16(2);
        factory->addUint16(11);
        factory->addUint8(creatureObject->GetPosture());
        factory->addUint16(16);
        factory->addUint64(creatureObject->states.getAction());

        _sendToInRange(factory->EndMessage(),creatureObject,5);
		factory_queue_.push(factory);
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
        mMessageFactory->addUint64(creatureObject->states.getAction());

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
        
        mMessageFactory->StartMessage();
        mMessageFactory->addUint32(opDeltasMessage);
        mMessageFactory->addUint64(creatureObject->getId());
        mMessageFactory->addUint32(opCREO);
        mMessageFactory->addUint8(3);
        mMessageFactory->addUint32(8); // bytes
        mMessageFactory->addUint16(1);	// No of items
        mMessageFactory->addUint16(8);	// Index 8 condition damage (vehicle)
		uint32 damage = creatureObject->GetStatMax(HamBar_Health);
		damage -= creatureObject->GetStatCurrent(HamBar_Health);
		
        mMessageFactory->addUint32(damage);
        _sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
    }
}


//======================================================================================================================
//
// Creature Deltas Type 4
// update: movement properties
//

bool MessageLib::sendUpdateMovementProperties(PlayerObject* playerObject)
{
    if(!_checkPlayer(playerObject)) {
		return false;
	}

	MessageFactory* factory = getFactory_();

    MovingObject* object = dynamic_cast<MovingObject*>(playerObject);

    if (playerObject->checkIfMounted()) {
        object = dynamic_cast<MovingObject*>(playerObject->getMount());
    }

    factory->StartMessage();
    factory->addUint32(opDeltasMessage);
    factory->addUint64(playerObject->getId());
    factory->addUint32(opCREO);
    factory->addUint8(4);
    factory->addUint32(26);

    factory->addUint16(4);

    factory->addUint16(5);
    factory->addFloat(object->getCurrentSpeedModifier());

    factory->addUint16(7);
    factory->addFloat(object->getCurrentRunSpeedLimit());

    factory->addUint16(10);
    factory->addFloat(object->getCurrentTurnRate());

    factory->addUint16(11);
    factory->addFloat(object->getCurrentAcceleration());

    (playerObject->getClient())->SendChannelA(factory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	factory_queue_.push(factory);

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
    //mMessageFactory->addUint8(remove);

    it = smList.begin();
    while(it != smList.end())
    {
        mMessageFactory->addUint8(remove);
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
    mMessageFactory->addUint16(1);
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

void MessageLib::sendGroupIdUpdateDeltasCreo6(uint64 groupId,  PlayerObject*  player, const PlayerObject* const target) const
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
	if(!(playerObject->isConnected()))
        return;

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

void MessageLib::sendMoodString(CreatureObject* creatureObject,BString animation)
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
	if(!(playerObject->isConnected()))
        return;

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
