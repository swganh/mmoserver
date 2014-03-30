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

#include "Zoneserver/Objects/Player Object/PlayerObjectFactory.h"
#include "ZoneServer\Objects\Creature Object\CreatureObject.h"
#include "anh/logger.h"

#include "Zoneserver/GameSystemManagers/Buff Manager/BuffManager.h"
#include "ZoneServer/GameSystemManagers/Mission Manager/MissionBag.h"
#include "ZoneServer/GameSystemManagers/State Manager/StateManager.h"

#include "Zoneserver/Objects/Bank.h"
#include "Zoneserver/Objects/Datapad.h"
#include "Zoneserver/Objects/DatapadFactory.h"
#include "Zoneserver/Objects/Inventory.h"
#include "Zoneserver/Objects/InventoryFactory.h"
#include "Zoneserver/Objects/Item.h"

#include "ZoneServer/Objects/Object/ObjectFactoryCallback.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer\Objects\Object\ObjectManager.h"

#include "ZoneServer/Objects/Tangible Object/TangibleFactory.h"
#include "ZoneServer/Tutorial.h"
#include "ZoneServer/Objects/Weapon.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include <cppconn/resultset.h>
#include <sstream>
#include <string> 

#include "Utils/utils.h"

#include "anh\event_dispatcher\event_dispatcher.h"
#include "ZoneServer\Services\ham\ham_service.h"
#include "ZoneServer\Services\equipment\equipment_service.h"
#include "anh/app/swganh_kernel.h"
#include "anh\service\service_manager.h"

using namespace swganh::event_dispatcher;

//=============================================================================

bool					PlayerObjectFactory::mInsFlag    = false;
PlayerObjectFactory*	PlayerObjectFactory::mSingleton  = NULL;

//======================================================================================================================

PlayerObjectFactory*	PlayerObjectFactory::Init(swganh::app::SwganhKernel*	kernel)
{
    if(!mInsFlag)
    {
        mSingleton	= new PlayerObjectFactory(kernel);
        mInsFlag	= true;

        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

PlayerObjectFactory::PlayerObjectFactory(swganh::app::SwganhKernel*	kernel) : FactoryBase(kernel)
{
    mInventoryFactory	= InventoryFactory::Init(kernel);
    mDatapadFactory		= DatapadFactory::Init(kernel);

    _setupDatabindings();
}

//=============================================================================

PlayerObjectFactory::~PlayerObjectFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void PlayerObjectFactory::handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case POFQuery_MainPlayerData:
    {

        PlayerObject* playerObject = _createPlayer(result);
        if(!playerObject)
        {
            DLOG(error) << "Failed to load player account [" << asyncContainer->mClient->getAccountId() << "]";
            break;
        }

        playerObject->setClient(asyncContainer->mClient);

		QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,POFQuery_Ham,asyncContainer->mClient);
        asContainer->mObject = playerObject;
		std::stringstream sql;
		sql << "SELECT 	character_attributes.health_max,character_attributes.strength_max,"//134
            "character_attributes.constitution_max,character_attributes.action_max,character_attributes.quickness_max,character_attributes.stamina_max,"  //138
            "character_attributes.mind_max,character_attributes.focus_max,character_attributes.willpower_max,character_attributes.health_current,"//142
            "character_attributes.strength_current,character_attributes.constitution_current,character_attributes.action_current,"	 //145
            "character_attributes.quickness_current,character_attributes.stamina_current,character_attributes.mind_current,character_attributes.focus_current,"	//149
            "character_attributes.willpower_current,character_attributes.health_wounds,character_attributes.strength_wounds,"//152
            "character_attributes.constitution_wounds,character_attributes.action_wounds,character_attributes.quickness_wounds," //155
            "character_attributes.stamina_wounds,character_attributes.mind_wounds,character_attributes.focus_wounds,character_attributes.willpower_wounds,"//159
            "character_attributes.health_encum,character_attributes.action_encum,character_attributes.mind_encum,character_attributes.battlefatigue"
			" FROM " << mDatabase->galaxy() << ".character_attributes WHERE character_attributes.character_id = " << playerObject->GetCreature()->getId() << ";";			

		mDatabase->executeSqlAsync(this, asContainer, sql.str());
        
	}
    break;

	case POFQuery_Ham:
	{
	/*	
    HamBar_Health		=	0,    HamBar_Strength	=	1,    HamBar_Constitution	=	2,
    HamBar_Action		=	3,    HamBar_Quickness	=	4,    HamBar_Stamina		=	5,
    HamBar_Mind			=	6,    HamBar_Focus		=	7,    HamBar_Willpower	=	8	*/

		PlayerObject* playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);
		CreatureObject* creature = playerObject->GetCreature();

		std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        while (result_set->next()) {
		
			//recalculate stat max later just initialize it so we have proper place prepared
			creature ->InitStatMax(result_set->getUInt(1)); //HamBar_Health
			creature ->InitStatMax(result_set->getUInt(2)); //strength
			creature ->InitStatMax(result_set->getUInt(3));	//constitution

			creature ->InitStatMax(result_set->getUInt(4)); //HamBar_Health
			creature ->InitStatMax(result_set->getUInt(5)); //strength
			creature ->InitStatMax(result_set->getUInt(6));	//constitution

			creature->InitStatMax(result_set->getUInt(7)); //HamBar_Health
			creature->InitStatMax(result_set->getUInt(8)); //strength
			creature->InitStatMax(result_set->getUInt(9));	//constitution

			creature->InitStatBase(result_set->getUInt(1)); //HamBar_Health
			creature->InitStatBase(result_set->getUInt(2)); //strength
			creature->InitStatBase(result_set->getUInt(3));	//constitution

			creature->InitStatBase(result_set->getUInt(4));//HamBar_Action
			creature->InitStatBase(result_set->getUInt(5));//HamBar_Quickness
			creature->InitStatBase(result_set->getUInt(6));//HamBar_Stamina

			creature->InitStatBase(result_set->getUInt(7));//HamBar_Mind
			creature->InitStatBase(result_set->getUInt(8));//HamBar_Focus
			creature->InitStatBase(result_set->getUInt(9));//HamBar_Willpower

			creature->InitStatCurrent(result_set->getUInt(10)); //HamBar_Health
			creature->InitStatCurrent(result_set->getUInt(11)); //strength
			creature->InitStatCurrent(result_set->getUInt(12)); //constitution

			creature->InitStatCurrent(result_set->getUInt(13)); //HamBar_Action
			creature->InitStatCurrent(result_set->getUInt(14)); //
			creature->InitStatCurrent(result_set->getUInt(15)); //

			creature->InitStatCurrent(result_set->getUInt(16)); //HamBar_Mind
			creature->InitStatCurrent(result_set->getUInt(17)); //
			creature->InitStatCurrent(result_set->getUInt(18)); //

			creature->InitStatWound(result_set->getUInt(19)); //HamBar_Health
			creature->InitStatWound(result_set->getUInt(20)); //
			creature->InitStatWound(result_set->getUInt(21)); //

			creature->InitStatWound(result_set->getUInt(22)); //HamBar_Action
			creature->InitStatWound(result_set->getUInt(23)); //
			creature->InitStatWound(result_set->getUInt(24)); //

			creature->InitStatWound(result_set->getUInt(25)); //HamBar_Mind
			creature->InitStatWound(result_set->getUInt(26)); //
			creature->InitStatWound(result_set->getUInt(27));

			creature->InitStatEncumberance(result_set->getUInt(28));
			creature->InitStatEncumberance(0);
			creature->InitStatEncumberance(0);
			creature->InitStatEncumberance(result_set->getUInt(29));
			creature->InitStatEncumberance(0);
			creature->InitStatEncumberance(0);
			creature->InitStatEncumberance(result_set->getUInt(30));
			creature->InitStatEncumberance(0);
			creature->InitStatEncumberance(0);


			creature->InitBattleFatigue(result_set->getUInt(31));
        }
		
		//now call the ham service to initialize max stats
		//auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");
			
		QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,POFQuery_Skills,asyncContainer->mClient);
        asContainer->mObject = playerObject;
		std::stringstream sql;
		sql << "SELECT skill_id FROM "
			<< mDatabase->galaxy() << ".character_skills WHERE character_id = "
			<< creature->getId() << ";";

		mDatabase->executeSqlAsync(this, asContainer, sql.str());
	}
	break;

    case POFQuery_Skills:
    {
        PlayerObject* playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);

		std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        while (result_set->next()) {
          
			playerObject->GetCreature()->InitializeSkill(gSkillManager->getSkillById(result_set->getUInt(1))->mName.getAnsi());
        }

        playerObject->GetCreature()->prepareSkillMods();
        playerObject->prepareSkillCommands();
        playerObject->prepareSchematicIds();

        playerObject->mSkillCmdUpdateCounter = playerObject->getSkillCommands()->size();
        playerObject->GetCreature()->mSkillModUpdateCounter = playerObject->GetCreature()->getSkillMods()->size();

        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,POFQuery_Badges,asyncContainer->mClient);
        asContainer->mObject = playerObject;
		
		std::stringstream sql;
		sql << "SELECT badge_id FROM "
			<< mDatabase->galaxy() << ".character_badges WHERE character_id = "
			<< playerObject->GetCreature()->getId() << ";";
        
		mDatabase->executeSqlAsync(this,asContainer,sql.str());

    }
    break;

    case POFQuery_Badges:
    {
        PlayerObject* playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);
        uint32 badgeId;

        swganh::database::DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(swganh::database::DFT_uint32,0,4);

        uint64 count = result->getRowCount();

        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&badgeId);
            playerObject->mBadgeList.push_back(badgeId);
        }

        mDatabase->destroyDataBinding(binding);

        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,POFQuery_Factions,asyncContainer->mClient);
        asContainer->mObject = playerObject;
		
		std::stringstream sql;

		sql << "SELECT faction_id,value FROM " << mDatabase->galaxy() << ".character_faction WHERE character_id=" << playerObject->GetCreature()->getId() << " ORDER BY faction_id";
        mDatabase->executeSqlAsync(this,asContainer,sql.str());

    }
    break;

    case POFQuery_Factions:
    {
        PlayerObject*	playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);
        XpContainer		factionCont;

        swganh::database::DataBinding* binding = mDatabase->createDataBinding(2);
        binding->addField(swganh::database::DFT_uint32,offsetof(XpContainer,mId),4,0);
        binding->addField(swganh::database::DFT_int32,offsetof(XpContainer,mValue),4,1);

        uint64 count = result->getRowCount();

        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&factionCont);
            playerObject->GetCreature()->mFactionList.push_back(std::make_pair(factionCont.mId,factionCont.mValue));
        }

        mDatabase->destroyDataBinding(binding);

        // query friendslist
        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,POFQuery_Friends,asyncContainer->mClient);
        asContainer->mObject = playerObject;

        std::stringstream sql;

		sql << "SELECT characters.firstname FROM " << mDatabase->galaxy() << ".chat_friendlist INNER JOIN " << mDatabase->galaxy() << ".characters ON (chat_friendlist.friend_id = characters.id) "
			<< "WHERE (chat_friendlist.character_id = " << playerObject->GetCreature()->getId() << ")";
                                   
		mDatabase->executeSqlAsync(this,asContainer, sql.str());

    }
    break;

    case POFQuery_Friends:
    {
        PlayerObject* playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);
        BString name;

        swganh::database::DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(swganh::database::DFT_bstring,0,64);

        uint64 count = result->getRowCount();

        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&name);
            name.toLower();
            playerObject->mFriendsList.insert(std::make_pair(name.getCrc(),name.getAnsi()));
        }

        mDatabase->destroyDataBinding(binding);

        // check online friends

        // query ignorelist
        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,POFQuery_Ignores,asyncContainer->mClient);
        asContainer->mObject = playerObject;

		std::stringstream sql;

		sql << "SELECT characters.firstname FROM " << mDatabase->galaxy() << ".chat_ignorelist INNER JOIN " << mDatabase->galaxy() << ".characters ON (chat_ignorelist.ignore_id = characters.id) "
			<< "WHERE (chat_ignorelist.character_id = " << playerObject->GetCreature()->getId() << ")";

        mDatabase->executeSqlAsync(this,asContainer, sql.str());

    }
    break;

    case POFQuery_Ignores:
    {
        PlayerObject* playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);
        BString name;

        swganh::database::DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(swganh::database::DFT_bstring,0,64);

        uint64 count = result->getRowCount();

        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&name);
            name.toLower();
            playerObject->mIgnoreList.insert(std::make_pair(name.getCrc(),name.getAnsi()));
        }

        mDatabase->destroyDataBinding(binding);

        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,POFQuery_XP,asyncContainer->mClient);
        asContainer->mObject = playerObject;

        mDatabase->executeSqlAsync(this,asContainer,"SELECT xp_id,value FROM %s.character_xp WHERE character_id=%"PRIu64"",mDatabase->galaxy(),playerObject->GetCreature()->getId());


        QueryContainerBase* outcastContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,POFQuery_DenyService,asyncContainer->mClient);
        outcastContainer->mObject = playerObject;

        mDatabase->executeSqlAsync(this,outcastContainer,"SELECT outcast_id FROM %s.entertainer_deny_service WHERE entertainer_id=%"PRIu64"",mDatabase->galaxy(),playerObject->GetCreature()->getId());


        QueryContainerBase* cloneDestIdContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,POFQuery_PreDefCloningFacility,asyncContainer->mClient);
        cloneDestIdContainer->mObject = playerObject;

        mDatabase->executeSqlAsync(this,cloneDestIdContainer,"SELECT spawn_facility_id, x, y, z, planet_id FROM %s.character_clone WHERE character_id=%"PRIu64"",mDatabase->galaxy(),playerObject->GetCreature()->getId());


        QueryContainerBase* LotsContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,POFQuery_Lots,asyncContainer->mClient);
        LotsContainer->mObject = playerObject;

        mDatabase->executeSqlAsync(this,LotsContainer,"SELECT %s.sf_getLotCount(%"PRIu64")",mDatabase->galaxy(),playerObject->GetCreature()->getId());

    }
    break;

    case POFQuery_DenyService:
    {
        PlayerObject* playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);
        uint64 id;

        swganh::database::DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(swganh::database::DFT_uint64,0,8);

        uint64 count = result->getRowCount();

        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&id);
            playerObject->mDenyAudienceList.push_back(id);
        }

        mDatabase->destroyDataBinding(binding);


        // query Holoemotes
        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,POFQuery_HoloEmotes,asyncContainer->mClient);
        asContainer->mObject = playerObject;

        mDatabase->executeSqlAsync(this,asContainer,"SELECT  emote_id, charges FROM %s.character_holoemotes WHERE character_id = %"PRIu64"",mDatabase->galaxy(),playerObject->GetCreature()->getId());

    }
    break;

    case POFQuery_HoloEmotes:
    {
        PlayerObject*	playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);


        swganh::database::DataBinding* binding = mDatabase->createDataBinding(2);
        binding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,mHoloEmote),4,0);
        binding->addField(swganh::database::DFT_int32,offsetof(PlayerObject,mHoloCharge),4,1);

        uint64 count = result->getRowCount();

        if(count ==1)
        {
            result->getNextRow(binding,playerObject);
        }

        mDatabase->destroyDataBinding(binding);
    }
    break;

    case POFQuery_XP:
    {
        PlayerObject*	playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);
        XpContainer		xpCont;

        swganh::database::DataBinding* binding = mDatabase->createDataBinding(2);
        binding->addField(swganh::database::DFT_uint32,offsetof(XpContainer,mId),4,0);
        binding->addField(swganh::database::DFT_int32,offsetof(XpContainer,mValue),4,1);

        uint64 count = result->getRowCount();

        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&xpCont);
            playerObject->mXpList.push_back(std::make_pair(xpCont.mId,xpCont.mValue));
        }
        // Initiate all XP caps and optionally any missing skills.
        // Skills that require Jedi or JTL will not be included if player do not have the prerequisites.
        gSkillManager->initExperience(playerObject);

        playerObject->mXpUpdateCounter = static_cast<uint32>(count);

        mDatabase->destroyDataBinding(binding);


        // store us for later lookup - loadcounter is 2 for inventory and datapad
        InLoadingContainer* ilc = new(mILCPool.ordered_malloc()) InLoadingContainer(playerObject,asyncContainer->mOfCallback,asyncContainer->mClient,2);

        //flag these two as necessary to load
        ilc->mInventory = false;
        ilc->mDPad = false;

		mObjectLoadMap.insert(std::make_pair(playerObject->GetCreature()->getId(),ilc));

        // request inventory
		mInventoryFactory->requestObject(this,playerObject->GetCreature()->getId() + INVENTORY_OFFSET,TanGroup_Inventory,TanType_CharInventory,asyncContainer->mClient);

    }
    break;

    case POFQuery_EquippedItems:
    {
        PlayerObject* playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);

		InLoadingContainer*	mIlc = _getObject(playerObject->GetCreature()->getId());

        uint64 id;
        swganh::database::DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(swganh::database::DFT_uint64,0,8);

        uint64 count = result->getRowCount();
		LOG(info) << "POFQuery_EquippedItems before : " << mIlc->mLoadCounter;
        mIlc->mLoadCounter += static_cast<uint32>(count);
		LOG(info) << "POFQuery_EquippedItems after : " << mIlc->mLoadCounter;
        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&id);
            gTangibleFactory->requestObject(this,id,TanGroup_Item,0,asyncContainer->mClient);

        }
        mDatabase->destroyDataBinding(binding);

        // get the datapad here to avoid a race condition
        // request datapad
		mDatapadFactory->requestObject(this,playerObject->GetCreature()->getId() + DATAPAD_OFFSET,TanGroup_Datapad,TanType_CharacterDatapad,asyncContainer->mClient);
    }
    break;

    // Get the id of the pre defined cloning facility, if any.
    case POFQuery_PreDefCloningFacility:
    {
        PlayerObject* playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);

        swganh::database::DataBinding* binding = mDatabase->createDataBinding(5);
        binding->addField(swganh::database::DFT_uint64,offsetof(PlayerObject,mPreDesignatedCloningFacilityId),8,0);
        binding->addField(swganh::database::DFT_float,offsetof(PlayerObject,mBindCoords.x),4,1);
        binding->addField(swganh::database::DFT_float,offsetof(PlayerObject,mBindCoords.y),4,2);
        binding->addField(swganh::database::DFT_float,offsetof(PlayerObject,mBindCoords.z),4,3);
        binding->addField(swganh::database::DFT_uint8,offsetof(PlayerObject,mBindPlanet),1,4);

        uint64 count = result->getRowCount();

        if (count == 1)
        {
            result->getNextRow(binding,playerObject);
        }
        else
        {
            playerObject->mPreDesignatedCloningFacilityId = 0;
        }

        mDatabase->destroyDataBinding(binding);
    }
    break;

    case POFQuery_Lots:
    {
        PlayerObject* playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);

        uint32 lotCount;
        swganh::database::DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(swganh::database::DFT_uint32,0,4);

        uint64 count = result->getRowCount();
        if(!count)
        {
            LOG(warning) << "sf_getLotCount did not return a value";
            //now we have a problem ...
            mDatabase->destroyDataBinding(binding);
            break;
        }

        result->getNextRow(binding,&lotCount);
        uint32 maxLots = gWorldConfig->getConfiguration<uint32>("Player_Max_Lots",(uint32)10);

        maxLots -= static_cast<uint8>(lotCount);
        playerObject->setLots((uint8)maxLots);

        mDatabase->destroyDataBinding(binding);
    }
    break;

    default:
    {
        break;
    }
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void PlayerObjectFactory::RegisterEventHandlers()
{
	auto dispatcher = gWorldManager->getKernel()->GetEventDispatcher();

dispatcher->Subscribe("PlayerObjectFactory::SaveCharacterAttributes", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
		auto value_event = std::static_pointer_cast<PlayerObjectEvent>(incoming_event);
		LOG(info) << "event_dispatcher_->Subscribe : subscription to : PlayerObjectFactory::SaveCharacterAttributes : " << value_event->Get()->getId();
        
		storeCharacterAttributes_(value_event->Get());
    });

dispatcher->Subscribe("PlayerObjectFactory::SaveCharacterPosition", [this] (std::shared_ptr<EventInterface> incoming_event)
    {
		auto value_event = std::static_pointer_cast<PlayerObjectEvent>(incoming_event);
		LOG(info) << "event_dispatcher_->Subscribe : subscription to : PlayerObjectFactory::SaveCharacterAttributes : " << value_event->Get()->getId();
        
		storeCharacterPosition(value_event->Get());
    });

}


void PlayerObjectFactory::storeCharacterAttributes_(PlayerObject* player_object) {
    
	if(!player_object) {
        DLOG(warning) << "Trying to save character position with an invalid PlayerObject";
        return;
    }

	auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");

    
    std::stringstream query_stream;

    query_stream << "UPDATE "<< mDatabase->galaxy()<<".character_attributes SET "
				 /*<< "health_current=" << (ham->mHealth.getCurrentHitPoints() - ham->mHealth.getModifier()) << ", "
                 << "action_current=" << (ham->mAction.getCurrentHitPoints() - ham->mAction.getModifier()) << ", "
                 << "mind_current=" << (ham->mMind.getCurrentHitPoints() - ham->mMind.getModifier()) << ", "
                 << "health_wounds=" << ham->mHealth.getWounds() << ", "
                 << "strength_wounds=" << ham->mStrength.getWounds() << ", "
                 << "constitution_wounds=" << ham->mConstitution.getWounds() << ", "
                 << "action_wounds=" << ham->mAction.getWounds() << ", "
                 << "quickness_wounds=" << ham->mQuickness.getWounds() << ", "
                 << "stamina_wounds=" << ham->mStamina.getWounds() << ", "
                 << "mind_wounds=" << ham->mMind.getWounds() << ", "
                 << "focus_wounds=" << ham->mFocus.getWounds() << ", "
                 << "willpower_wounds=" << ham->mWillpower.getWounds() << ", "
                 << "battlefatigue=" << ham->getBattleFatigue() << ", "*/
                 << "posture=" << (uint16) player_object->GetCreature()->GetPosture() << ", "
                 << "moodId=" << static_cast<uint16_t>(player_object->GetCreature()->getMoodId()) << ", "
                 << "title='" << mDatabase->escapeString(player_object->getTitle().getAnsi()) << "', "
                 << "character_flags=" << player_object->getPlayerFlags() << ", "
                 << "states=" << player_object->GetCreature()->GetStateBitmask() << ", "
                 << "language=" << player_object->getLanguage() << ", "
                 << "new_player_exemptions=" <<  static_cast<uint16_t>(player_object->getNewPlayerExemptions()) << " "
                 << "WHERE character_id=" << player_object->getId();

    mDatabase->executeAsyncSql(query_stream.str());
}


void PlayerObjectFactory::storeCharacterPosition(PlayerObject* player_object, uint32 zone) {
    
	if(!player_object) {
        DLOG(warning) << "PlayerObjectFactory::storeCharacterPosition no player";
        return;
    }

	if(player_object->isBeingDestroyed())	{
		DLOG(warning) << "PlayerObjectFactory::storeCharacterPosition player already in destruction";
        return;
	}
	
	uint32 zoneId;
	if(zone == 0xffffffff)
		zoneId = gWorldManager->getZoneId();
	else
		zoneId = zone;


    std::stringstream query_stream;

    query_stream << "UPDATE "<< mDatabase->galaxy() << ".characters SET parent_id=" << player_object->getParentId() << ", "
                 << "oX=" << player_object->mDirection.x << ", "
                 << "oY=" << player_object->mDirection.y << ", "
                 << "oZ=" << player_object->mDirection.z << ", "
                 << "oW=" << player_object->mDirection.w << ", "
                 << "x=" << player_object->mPosition.x << ", "
                 << "y=" << player_object->mPosition.y << ", "
                 << "z=" << player_object->mPosition.z << ", "
				 << "planet_id=" << zoneId << ", "
                 << "WHERE id=" << player_object->getId();

    mDatabase->executeAsyncSql(query_stream.str());
}


void PlayerObjectFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    QueryContainerBase* asyncContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,POFQuery_MainPlayerData,client);


    int8 sql[8152];
    sprintf(sql,"SELECT characters.id,characters.parent_Id,characters.account_id,characters.oX,characters.oY,characters.oZ,characters.oW,"//7
            "characters.x,characters.y,characters.z,character_appearance.base_model_string,"//11
            "characters.firstname,characters.lastname,character_appearance.hair,character_appearance.hair1,character_appearance.hair2,race.name,"//17
            "character_appearance.`00FF`,character_appearance.`01FF`,character_appearance.`02FF`,character_appearance.`03FF`,character_appearance.`04FF`,"	  //22
            "character_appearance.`05FF`,character_appearance.`06FF`,character_appearance.`07FF`,character_appearance.`08FF`,character_appearance.`09FF`,"	  //27
            "character_appearance.`0AFF`,character_appearance.`0BFF`,character_appearance.`0CFF`,character_appearance.`0DFF`,character_appearance.`0EFF`,"	  //32
            "character_appearance.`0FFF`,character_appearance.`10FF`,character_appearance.`11FF`,character_appearance.`12FF`,character_appearance.`13FF`,"	  //37
            "character_appearance.`14FF`,character_appearance.`15FF`,character_appearance.`16FF`,character_appearance.`17FF`,character_appearance.`18FF`,"	  //42
            "character_appearance.`19FF`,character_appearance.`1AFF`,character_appearance.`1BFF`,character_appearance.`1CFF`,character_appearance.`1DFF`,"	  //47
            "character_appearance.`1EFF`,character_appearance.`1FFF`,character_appearance.`20FF`,character_appearance.`21FF`,character_appearance.`22FF`,"	  //52
            "character_appearance.`23FF`,character_appearance.`24FF`,character_appearance.`25FF`,character_appearance.`26FF`,character_appearance.`27FF`,"	  //57
            "character_appearance.`28FF`,character_appearance.`29FF`,character_appearance.`2AFF`,character_appearance.`2BFF`,character_appearance.`2CFF`,"	  //62
            "character_appearance.`2DFF`,character_appearance.`2EFF`,character_appearance.`2FFF`,character_appearance.`30FF`,character_appearance.`31FF`,"	  //67
            "character_appearance.`32FF`,character_appearance.`33FF`,character_appearance.`34FF`,character_appearance.`35FF`,character_appearance.`36FF`,"	  //72
            "character_appearance.`37FF`,character_appearance.`38FF`,character_appearance.`39FF`,character_appearance.`3AFF`,character_appearance.`3BFF`,"	  //77
            "character_appearance.`3CFF`,character_appearance.`3DFF`,character_appearance.`3EFF`,character_appearance.`3FFF`,character_appearance.`40FF`,"	  //82
            "character_appearance.`41FF`,character_appearance.`42FF`,character_appearance.`43FF`,character_appearance.`44FF`,character_appearance.`45FF`,"	  //87
            "character_appearance.`46FF`,character_appearance.`47FF`,character_appearance.`48FF`,character_appearance.`49FF`,character_appearance.`4AFF`,"	  //92
            "character_appearance.`4BFF`,character_appearance.`4CFF`,character_appearance.`4DFF`,character_appearance.`4EFF`,character_appearance.`4FFF`,"	  //97
            "character_appearance.`50FF`,character_appearance.`51FF`,character_appearance.`52FF`,character_appearance.`53FF`,character_appearance.`54FF`,"	  //102
            "character_appearance.`55FF`,character_appearance.`56FF`,character_appearance.`57FF`,character_appearance.`58FF`,character_appearance.`59FF`,"	  //107
            "character_appearance.`5AFF`,character_appearance.`5BFF`,character_appearance.`5CFF`,character_appearance.`5DFF`,character_appearance.`5EFF`,"	  //112
            "character_appearance.`5FFF`,character_appearance.`60FF`,character_appearance.`61FF`,character_appearance.`62FF`,character_appearance.`63FF`,"	  //117
            "character_appearance.`64FF`,character_appearance.`65FF`,character_appearance.`66FF`,character_appearance.`67FF`,character_appearance.`68FF`,"	  //122
            "character_appearance.`69FF`,character_appearance.`6AFF`,character_appearance.`6BFF`,character_appearance.`6CFF`,character_appearance.`6DFF`,"	  //127
            "character_appearance.`6EFF`,character_appearance.`6FFF`,character_appearance.`70FF`,character_appearance.`ABFF`,character_appearance.`AB2FF`,"//132
         
			/*"character_attributes.health_max,character_attributes.strength_max,"//134
            "character_attributes.constitution_max,character_attributes.action_max,character_attributes.quickness_max,character_attributes.stamina_max,"  //138
            "character_attributes.mind_max,character_attributes.focus_max,character_attributes.willpower_max,character_attributes.health_current,"//142
            "character_attributes.strength_current,character_attributes.constitution_current,character_attributes.action_current,"	 //145
            "character_attributes.quickness_current,character_attributes.stamina_current,character_attributes.mind_current,character_attributes.focus_current,"	//149
            "character_attributes.willpower_current,character_attributes.health_wounds,character_attributes.strength_wounds,"//152
            "character_attributes.constitution_wounds,character_attributes.action_wounds,character_attributes.quickness_wounds," //155
            "character_attributes.stamina_wounds,character_attributes.mind_wounds,character_attributes.focus_wounds,character_attributes.willpower_wounds,"//159
            "character_attributes.health_encum,character_attributes.action_encum,character_attributes.mind_encum,character_attributes.battlefatigue,",
			*/
			"character_attributes.language,"	//164
            "banks.credits,faction.name,"//166
            "character_attributes.posture,character_attributes.moodId,characters.jedistate,character_attributes.title,character_appearance.scale,"   //171
            "character_movement.baseSpeed,character_movement.baseAcceleration,character_movement.baseTurnrate,character_movement.baseTerrainNegotiation,"//175 grml off by one it should be 176
            "character_attributes.character_flags,character_biography.biography,character_attributes.states,characters.race_id,"
            "banks.planet_id,account.account_csr,character_attributes.group_id,characters.bornyear,"
            "character_matchmaking.match_1, character_matchmaking.match_2, character_matchmaking.match_3, character_matchmaking.match_4,"
            "character_attributes.force_current,character_attributes.force_max,character_attributes.new_player_exemptions"
            " FROM %s.characters"
            " INNER JOIN %s.account ON(characters.account_id = account.account_id)"
            " INNER JOIN %s.banks ON (%"PRIu64" = banks.id)"
            " INNER JOIN %s.character_appearance ON (characters.id = character_appearance.character_id)"
            " INNER JOIN %s.race ON (characters.race_id = race.id)"
            " INNER JOIN %s.character_attributes ON (characters.id = character_attributes.character_id)"
            " INNER JOIN %s.character_movement ON (characters.id = character_movement.character_id)"
            " INNER JOIN %s.faction ON (character_attributes.faction_id = faction.id)"
            " INNER JOIN %s.character_biography ON (characters.id = character_biography.character_id)"
            " INNER JOIN %s.character_matchmaking ON (characters.id = character_matchmaking.character_id)"
            " WHERE (characters.id = %"PRIu64");",
            mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),id + BANK_OFFSET,mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),
            mDatabase->galaxy(), mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(), id);

    mDatabase->executeSqlAsync(this,asyncContainer,sql);
}

//=============================================================================

PlayerObject* PlayerObjectFactory::_createPlayer(swganh::database::DatabaseResult* result)
{
    if (!result->getRowCount()) {
        return nullptr;
    }

	PlayerObject*		player= new PlayerObject();
	CreatureObject*		creature = new CreatureObject();
	
	creature->setType(ObjType_Player); 
	creature->setCreoGroup(CreoGroup_Player);
	creature->object_type_ = SWG_CREATURE;

	player->setType(ObjType_Player); 
	player->object_type_ = SWG_PLAYER;

	player->body_ = creature;
	
	
	
	std::shared_ptr<TangibleObject> playerHair = std::make_shared<TangibleObject>();
	
    MissionBag*		playerMissionBag;
	
    // get our results
    result->getNextRow(ghost_binding,(void*)player);
    result->resetRowIndex();
	result->getNextRow(creature_binding,(void*)creature);
    result->resetRowIndex();
    result->getNextRow(mHairBinding,(void*)playerHair.get());
    
	std::shared_ptr<Bank> playerBank = std::make_shared<Bank>(player);

	result->resetRowIndex();
    result->getNextRow(mBankBinding,(void*)playerBank.get());

	std::string name = creature->getFirstName() + " " + creature->getLastName();

	creature->setCustomName(std::u16string(name.begin(), name.end()));
	creature->mKnownPlayers.insert(player);

	BString mModel = creature->GetTemplate().c_str();

    //male or female ?
    BStringVector				dataElements;
    mModel.split(dataElements,'_');
    if(dataElements.size() > 1) {
        player->setGender(dataElements[1].getCrc() == BString("female.iff").getCrc());
    } else { //couldn't find data, default to male. Is this acceptable? Crash bug patch: http://paste.swganh.org/viewp.php?id=20100627013612-b69ab274646815fb2a9befa4553c93f7
        LOG(warning) << "Player [" << creature->getId() << "] Could not determine requested gender, defaulting to male";
        player->setGender(false);
    }

    // player object
    int8 tmpModel[128];
    sprintf(tmpModel,"object/creature/player/shared_%s",&mModel.getAnsi()[23]);
	creature->SetTemplate(tmpModel);

    creature->buildCustomization(creature->mCustomization);

	auto permissions_objects_ = gObjectManager->GetPermissionsMap();
	creature->SetPermissions(permissions_objects_.find(swganh::object::CREATURE_PERMISSION)->second.get());//CREATURE_PERMISSION
	player->SetPermissions(permissions_objects_.find(swganh::object::CREATURE_CONTAINER_PERMISSION)->second.get());//CREATURE_PERMISSION

    creature->setFactionRank(0);
    // playerObject->setPvPStatus(16);
    creature->setPvPStatus(CreaturePvPStatus_Player);
    creature->setSpeciesGroup("species");
    creature->setCL(0);
	creature->mTypeOptions = 0x80;
    creature->SetGhost(player);
	player->setId(creature->mId + PLAYER_OFFSET);
	player->mBiography.convert(BSTRType_Unicode16);
	player->setNameFile("string_id_table");
	player->SetTemplate("object/player/shared_player.iff");//619BAE21,object/player/shared_player.iff,data_other_00_o.txt
	player->SetContainer(creature);
	player->setParentId(creature->getId());

	gWorldManager->addObject(creature);
	gWorldManager->addObject(player);

	gObjectManager->LoadSlotsForObject(creature);
	gObjectManager->LoadSlotsForObject(player);

	creature->InitializeObject(player);

	//now add to world - then add children

    // hair
	mModel = playerHair->GetTemplate().c_str();
    if(mModel.getLength())
    {
        int8 tmpHair[128];
        sprintf(tmpHair,"object/tangible/hair/%s/shared_%s",creature->mSpecies.getAnsi(),&mModel.getAnsi()[22 + creature->mSpecies.getLength()]);
        playerHair->setId(creature->getId() + HAIR_OFFSET);
        playerHair->setParentId(creature->getId());
        playerHair->SetTemplate(tmpHair);
        playerHair->setTangibleGroup(TanGroup_Hair);
        playerHair->setTangibleType(TanType_Hair);
        playerHair->setName("hair");
        playerHair->setNameFile("hair_name");
		playerHair->SetPermissions(permissions_objects_.find(swganh::object::CREATURE_CONTAINER_PERMISSION)->second.get());//CREATURE_CONTAINER_PERMISSION
		playerHair->SetContainer(creature);

        playerHair->buildTanoCustomization(3);

		gObjectManager->LoadSlotsForObject(playerHair.get());

        creature->InitializeObject(playerHair.get());
		gWorldManager->addObject(playerHair,true);
    }


    // mission bag
    playerMissionBag = new MissionBag(creature->getId() + MISSION_OFFSET,player,"object/tangible/mission_bag/shared_mission_bag.iff","item_n","mission_bag");
	playerMissionBag->SetPermissions(permissions_objects_.find(swganh::object::CREATURE_CONTAINER_PERMISSION)->second.get());//6
	gObjectManager->LoadSlotsForObject(playerMissionBag);
	gWorldManager->addObject(playerMissionBag,true);
	creature->InitializeObject(playerMissionBag);
	playerMissionBag->SetContainer(creature);

    // bank
	
	playerBank->SetPermissions(permissions_objects_.find(swganh::object::CREATURE_CONTAINER_PERMISSION)->second.get());//CREATURE_CONTAINER_PERMISSION
	playerBank->setId(creature->getId() + BANK_OFFSET);
    playerBank->setParentId(creature->mId);
    playerBank->SetTemplate("object/tangible/bank/shared_character_bank.iff");
    playerBank->setName("bank");
    playerBank->setNameFile("item_n");
    playerBank->setTangibleGroup(TanGroup_PlayerInternal);
    playerBank->setTangibleType(TanType_Bank);

    gObjectManager->LoadSlotsForObject(playerBank.get());
	gWorldManager->addObject(playerBank,true);
	creature->InitializeObject(playerBank.get());
	playerBank->SetContainer(creature);
	

    // default player weapon
	Weapon*			playerWeapon	= new Weapon();
    playerWeapon->SetPermissions(permissions_objects_.find(swganh::object::CREATURE_CONTAINER_PERMISSION)->second.get());//CREATURE_CONTAINER_PERMISSION
	playerWeapon->setId(creature->getId() + WEAPON_OFFSET);
    playerWeapon->setParentId(creature->getId());
    playerWeapon->SetTemplate("object/weapon/melee/unarmed/shared_unarmed_default_player.iff");
    playerWeapon->setGroup(WeaponGroup_Unarmed);
    playerWeapon->addInternalAttribute("weapon_group","1");
	playerWeapon->SetContainer(creature);
	
	gObjectManager->LoadSlotsForObject(playerWeapon);
	gWorldManager->addObject(playerWeapon,true);
	creature->InitializeObject(playerWeapon);

    // just making sure
    player->togglePlayerFlagOff(PlayerFlag_LinkDead);

    if(creature->GetPosture() == CreaturePosture_SkillAnimating
            || creature->GetPosture() == CreaturePosture_Incapacitated
            || creature->GetPosture() == CreaturePosture_Dead
			|| creature->GetPosture() == CreaturePosture_DrivingVehicle)
    {
        creature->SetPosture(CreaturePosture_Upright);
    }

    //Concerning states we must realize that some of the states persist when we use a shuttle to transfer to different planets
	//on a fresh login however these states should be (??? or not ???) reset

    // Todo : which states remain valid after a zone to zone transition ??? in order to transfer zone e need to be out of combat - so ... none ?
    creature->toggleStateOff((CreatureState)(
            CreatureState_Cover |
            CreatureState_Combat |
            CreatureState_Aiming |
            CreatureState_Berserk |
            CreatureState_FeignDeath |
            CreatureState_CombatAttitudeEvasive |		// these should be altered on login ???
            CreatureState_CombatAttitudeNormal |		// these should be altered on login ???
            CreatureState_CombatAttitudeAggressive |	// these should be altered on login ???
            CreatureState_Swimming |
            CreatureState_Crafting |
            CreatureState_RidingMount |
            CreatureState_MountedCreature |
            CreatureState_Peace ));
	
    
    player->mStomach->checkForRegen();

    // setup controller validators
    creature->mObjectController.initEnqueueValidators();
    creature->mObjectController.initProcessValidators();

    // update movement properties
    creature->updateMovementProperties();

    // update race / gender mask
    creature->updateRaceGenderMask(player->getGender());

    gBuffManager->LoadBuffs(player, gWorldManager->GetCurrentGlobalTick());

    // Start tutorial, if any.
    player->startTutorial();

    return player;
}

//=============================================================================

void PlayerObjectFactory::_setupDatabindings()
{

	ghost_binding = mDatabase->createDataBinding(15);
	ghost_binding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,mAccountId),4,2);
	ghost_binding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,mJediState),4,137);
	ghost_binding->addField(swganh::database::DFT_bstring,offsetof(PlayerObject,mTitle),255,138);
	ghost_binding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,mPlayerFlags),4,144);
    ghost_binding->addField(swganh::database::DFT_bstring,offsetof(PlayerObject,mBiography),4096,145);
	ghost_binding->addField(swganh::database::DFT_uint8,offsetof(PlayerObject,mLanguage),1,132);
    ghost_binding->addField(swganh::database::DFT_uint8,offsetof(PlayerObject,mCsrTag),1,149);
    
    ghost_binding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,mBornyear),4,151);
    ghost_binding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,mPlayerMatch[0]),4,152);
    ghost_binding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,mPlayerMatch[1]),4,153);
    ghost_binding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,mPlayerMatch[2]),4,154);
    ghost_binding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,mPlayerMatch[3]),4,155);
    ghost_binding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,current_force_power_),4,156);
    ghost_binding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,max_force_power_),4,157);
    ghost_binding->addField(swganh::database::DFT_uint8,offsetof(PlayerObject,mNewPlayerExemptions),1,158);//39
	

    //player binding
    creature_binding = mDatabase->createDataBinding(185);
    creature_binding->addField(swganh::database::DFT_uint64,offsetof(CreatureObject,mId),8,0);
    creature_binding->addField(swganh::database::DFT_uint64,offsetof(CreatureObject,mParentId),8,1);
    //mPlayerBinding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,mAccountId),4,2);
    creature_binding->addField(swganh::database::DFT_float,offsetof(CreatureObject,mDirection.x),4,3);
    creature_binding->addField(swganh::database::DFT_float,offsetof(CreatureObject,mDirection.y),4,4);
    creature_binding->addField(swganh::database::DFT_float,offsetof(CreatureObject,mDirection.z),4,5);
    creature_binding->addField(swganh::database::DFT_float,offsetof(CreatureObject,mDirection.w),4,6);
    creature_binding->addField(swganh::database::DFT_float,offsetof(CreatureObject,mPosition.x),4,7);
    creature_binding->addField(swganh::database::DFT_float,offsetof(CreatureObject,mPosition.y),4,8);
    creature_binding->addField(swganh::database::DFT_float,offsetof(CreatureObject,mPosition.z),4,9);
	creature_binding->addField(swganh::database::DFT_stdstring,offsetof(CreatureObject,template_string_),128,10);
	creature_binding->addField(swganh::database::DFT_uint64,offsetof(CreatureObject,mGroupId),8,150);
	creature_binding->addField(swganh::database::DFT_stdstring,offsetof(CreatureObject,first_name),64,11);
    creature_binding->addField(swganh::database::DFT_stdstring,offsetof(CreatureObject,last_name),64,12);
    creature_binding->addField(swganh::database::DFT_bstring,offsetof(CreatureObject,mSpecies),16,16);
    creature_binding->addField(swganh::database::DFT_bstring,offsetof(CreatureObject,mFaction),16,134);
    creature_binding->addField(swganh::database::DFT_uint16,offsetof(CreatureObject,states.posture_),1,135);
    creature_binding->addField(swganh::database::DFT_uint8,offsetof(CreatureObject,mMoodId),1,136);
    //mPlayerBinding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,mJediState),4,137);
    //mPlayerBinding->addField(swganh::database::DFT_bstring,offsetof(PlayerObject,mTitle),255,138);
    creature_binding->addField(swganh::database::DFT_float,offsetof(CreatureObject,mScale),4,139);

    creature_binding->addField(swganh::database::DFT_float,offsetof(CreatureObject,mBaseRunSpeedLimit),4,140);
    creature_binding->addField(swganh::database::DFT_float,offsetof(CreatureObject,mBaseAcceleration),4,141);
    creature_binding->addField(swganh::database::DFT_float,offsetof(CreatureObject,mBaseTurnRate),4,142);
    creature_binding->addField(swganh::database::DFT_float,offsetof(CreatureObject,mBaseTerrainNegotiation),4,143);//24

    //mPlayerBinding->addField(swganh::database::DFT_uint32,offsetof(PlayerObject,mPlayerFlags),4,144);
    //mPlayerBinding->addField(swganh::database::DFT_bstring,offsetof(PlayerObject,mBiography),4096,145);
    creature_binding->addField(swganh::database::DFT_uint64,offsetof(CreatureObject,states.action),8,146);
    creature_binding->addField(swganh::database::DFT_uint8,offsetof(CreatureObject,mRaceId),1,147);
    
	

    for(uint16 i = 0; i < 0x71; i++)
        creature_binding->addField(swganh::database::DFT_uint16,offsetof(CreatureObject,mCustomization)+(i*2),2,i + 17);//+113 = 183

    creature_binding->addField(swganh::database::DFT_uint16,offsetof(CreatureObject,mCustomization[171]),2,130);
    creature_binding->addField(swganh::database::DFT_uint16,offsetof(CreatureObject,mCustomization[172]),2,131);				   //185

    //hair binding
    mHairBinding = mDatabase->createDataBinding(3);
	mHairBinding->addField(swganh::database::DFT_stdstring,offsetof(TangibleObject,template_string_),128,13);
    mHairBinding->addField(swganh::database::DFT_uint16,offsetof(TangibleObject,mCustomization[1]),2,14);
    mHairBinding->addField(swganh::database::DFT_uint16,offsetof(TangibleObject,mCustomization[2]),2,15);

    //bank binding
    mBankBinding = mDatabase->createDataBinding(2);
    mBankBinding->addField(swganh::database::DFT_uint32,offsetof(Bank,credits_),4,133);
    mBankBinding->addField(swganh::database::DFT_uint8,offsetof(Bank,planet_),1,148);
}

//=============================================================================

void PlayerObjectFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(ghost_binding);
	mDatabase->destroyDataBinding(creature_binding);
    mDatabase->destroyDataBinding(mHairBinding);
    mDatabase->destroyDataBinding(mBankBinding);
}

//=============================================================================

void PlayerObjectFactory::handleObjectReady(Object* object,DispatchClient* client)
{
    InLoadingContainer*				ilc;
    ilc= _getObject(object->getParentId());
    if(!ilc)
    {
        assert(false && "[PlayerObjectFactory::handleObjectReady] no InLoadingContainer");
        return;
    }
    
	ilc->mLoadCounter--;

    PlayerObject*		player = dynamic_cast<PlayerObject*>(ilc->mObject);
    if(!player)
    {
        assert(false && "[PlayerObjectFactory::handleObjectReady] no playerObject");
        return;
    }

	CreatureObject*		creature = dynamic_cast<CreatureObject*>(player->GetCreature());

    if(Inventory* inventory = dynamic_cast<Inventory*>(object))
    {
        ilc->mInventory = true;
        
		auto permissions_objects_ = gObjectManager->GetPermissionsMap();
		object->SetPermissions(permissions_objects_.find(swganh::object::CREATURE_CONTAINER_PERMISSION)->second.get());//CREATURE_PERMISSION

		creature->InitializeObject(inventory);
		gWorldManager->addObject(inventory,true);

		inventory->setParent(creature);
		inventory->setTypeOptions(256);
		inventory->SetContainer(creature);
		
        player->setInventory(inventory);

        QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(0,POFQuery_EquippedItems,client);
        asContainer->mObject = player;

        mDatabase->executeSqlAsync(this,asContainer,"SELECT id  FROM %s.items WHERE parent_id=%"PRIu64"",mDatabase->galaxy(),creature->getId());
    }
    else if(Datapad* datapad = dynamic_cast<Datapad*>(object))
    {
        ilc->mDPad = true;
        
        creature->InitializeObject(datapad);
		gWorldManager->addObject(datapad,true);

        player->setDataPad(datapad);

		datapad->SetContainer(creature);
        datapad->setOwner(creature);
    }
    else if(TangibleObject* item =  dynamic_cast<TangibleObject*>(object))
    {
		item->setParentId(creature->getId());
        gWorldManager->addObject(item,true);
		item->SetContainer(creature);

        creature->InitializeObject(item);
		
		auto equipment_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");

		//auto inventory = dynamic_cast<Inventory*>(equipment_service->GetEquippedObject(this, "inventory"));
        
    }

	LOG(info) <<"PlayerObjectFactory::handleObjectReady : loadcounter : " << ilc->mLoadCounter;
	LOG(info) <<"object : " << object->GetTemplate();

    if((!ilc->mLoadCounter) && ((!ilc->mInventory) || (!ilc->mDPad)))    {
        LOG(warning) << "mIlc LoadCounter is messed up - we have a race condition";
    }

    if((!ilc->mLoadCounter) && (ilc->mInventory) && (ilc->mDPad))    {
		LOG(info) <<"PlayerObjectFactory::handleObjectReady : done";

        if(!(_removeFromObjectLoadMap(creature->getId())))
            LOG(warning) << "Failed removing object from loadmap";
        
        Datapad* dpad = player->getDataPad();
        if(!dpad)        {
            assert(dpad && "PlayerObjectFactory::No Datapad!!!!!");
            return;

        }

        ilc->mOfCallback->handleObjectReady(creature,ilc->mClient);

        mILCPool.free(ilc);
    }
}

//=============================================================================

void PlayerObjectFactory::releaseAllPoolsMemory()
{
    mInventoryFactory->releaseQueryContainerPoolMemory();
    mInventoryFactory->releaseILCPoolMemory();
    mDatapadFactory->releaseQueryContainerPoolMemory();
    mDatapadFactory->releaseILCPoolMemory();

    releaseQueryContainerPoolMemory();
    releaseILCPoolMemory();
}

//=============================================================================



