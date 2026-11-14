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
#include "Camp.h"
#include "CampRegion.h"
#include "CampTerminal.h"
#include "Item_Enums.h"
#include "nonPersistantObjectFactory.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "ScoutManager.h"
#include "Scout.h"
#include "StructureManager.h"
#include "TangibleObject.h"
#include "UIManager.h"
#include "WorldManager.h"
#include "WorldManager.h"
#include "AttackableCreature.h"
#include "Inventory.h"
#include "GroupManager.h"
#include "CombatManager.h"
#include "Resource.h"
#include "ResourceContainer.h"
#include "ResourceManager.h"
#include "ResourceType.h"
#include "DatabaseManager/Database.h"
#include "SkillManager.h"
#include "SkillEnums.h"
#include <algorithm>
#include <cctype>
#include <sstream>

#include "MessageLib/MessageLib.h"
#include "Utils/rand.h"

ScoutManager*	ScoutManager::mSingleton = NULL;


ScoutManager::ScoutManager()
{
}

ScoutManager::~ScoutManager(void)
{
}

//================================================================================
//CAMPS!
//================================================================================
bool ScoutManager::createCamp(uint32 typeId,uint64 parentId, const glm::vec3& position, const BString& customName, PlayerObject* player)
{
    //get blueprint out of the db
    if (gStructureManager->checkNoBuildRegion(player))
        return false;

    StructureDeedLink*	deedData = 	gStructureManager->getDeedData(typeId);
    if(!deedData)
        return false;

    if(!player->checkSkill(deedData->skill_Requirement))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("camp", "sys_nsf_skill"), player);
        return false;
    }

    if(!gStructureManager->checkCampRadius(player))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("camp", "error_camp_too_close"), player);
        return false;
    }

    if(!gStructureManager->checkCityRadius(player))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("camp", "error_nobuild"), player);
        return false;
    }

    if(player->HasCamp())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("camp", "sys_already_camping"), player);
    }

	Camp* camp = new (Camp);
	camp->mPosition = position;

	camp->mDirection.x = 0;
	camp->mDirection.y = 1;
	camp->mDirection.z = 0;

	camp->mDirection.w = 1;

	camp->setParentId(parentId);
	camp->setCustomName(customName.getAnsi());
	camp->setMaxCondition(100);

	camp->setPlayerStructureFamily(PlayerStructure_Camp);
	camp->setTangibleGroup(TanGroup_Structure);

	camp->setId(gWorldManager->getRandomNpId());

	camp->setOwner(player->getId());

	camp->setModelString(deedData->structureObjectString);
	camp->setName(deedData->stf_name.getAnsi());
	camp->setNameFile(deedData->stf_file.getAnsi());

	//create it in the world
	gWorldManager->addObject(camp);	

	//now get all the respective templates chairs/fires/torches and stuff

	StructureItemList*	sIL = gStructureManager->getStructureItemList();
	StructureItemList::iterator sILIt = sIL->begin();

	CampTerminal* terminal(0);

	while(sILIt != sIL->end())
	{
		if((*sILIt)->structure_id == deedData->structure_type)
		{
			//is this our terminal???
			TangibleObject* tO;
			if((*sILIt)->tanType == TanGroup_Terminal)
			{
				tO = gNonPersistantObjectFactory->spawnTerminal((*sILIt),0,player->mPosition,"",player,deedData);
				terminal = dynamic_cast<CampTerminal*>(tO);
				terminal->setOwner(player->getId());
				terminal->setCamp(camp->getId());

			}
			else
				tO = gNonPersistantObjectFactory->spawnTangible((*sILIt),0,player->mPosition,"",player);

			if(tO)
			{
				camp->getItemList()->push_back(tO);
			}
		}
		sILIt++;
	}

	//add a camp region
	std::shared_ptr<CampRegion> camp_region = std::make_shared<CampRegion>();

	camp_region->setId(gWorldManager->getRandomNpId());

	if(terminal)
		terminal->setCampRegion(camp_region->getId());

	camp_region->mPosition = player->mPosition;


	//@todo need to update these sizes so they are more accurate.
	//set the bounds of the camp (so we know when the camp is abandoned

	switch(typeId)
	{
	case ItemType_Camp_basic: 
		camp_region->setWidth(15.0); //11 Width + 2 buffer on each side
		camp_region->setHeight(15.0); //11 Height + 2 buffer on each side
		break;
	case ItemType_Camp_elite:
		camp_region->setWidth(18.0); //14 width + 2 buffer on each side
		camp_region->setHeight(26.0); //22 height + 2 buffer on each side
		break;
	case ItemType_Camp_improved:
		camp_region->setWidth(21.0); //17 width + 2 buffer on each side
		camp_region->setHeight(18.0); //14 height + 2 buffer on each side
		break;
	case ItemType_Camp_luxury: 
		camp_region->setWidth(46.0); //42 width + 2 buffer on each side
		camp_region->setHeight(45.0); //41 height + 2 buffer on each side
		break;
	case ItemType_Camp_multi:
		camp_region->setWidth(32.0); //28 height + 2 buffer on each side
		camp_region->setHeight(34.0); //30 height + 2 buffer on each side
		break;
	case ItemType_Camp_quality:
		camp_region->setWidth(24.0); //20 width + 2 buffer on each side
		camp_region->setHeight(25.0); //21 height + 2 buffer on each side
		break;
	default:
		camp_region->setWidth(5.0);
		camp_region->setHeight(5.0);
	}

	camp_region->setOwner(player->getId());
	camp_region->setCamp(camp->getId());
	camp_region->setParentId(0);
	camp_region->setHealingModifier(deedData->healing_modifier);
	camp_region->setActive(true);
	gWorldManager->addObject(camp_region);

	int8 name[64];
	sprintf(name,"%s %s",player->getFirstName().getAnsi(),player->getLastName().getAnsi());
	camp_region->setCampOwnerName(name);

	player->setHasCamp(true);

	return true;
}

//================================================================================
//FORAGING!
//================================================================================
void ScoutManager::successForage(PlayerObject* player)
{
    //gLogger->log(LogManager::DEBUG,"FORAGING ATTEMPT SUCCESS!", FOREGROUND_RED);
    //Chance of success = sqrt(skill)/20 + 0.15
    //Chance in down = chance/2

    //First lets calc our chance to 'win'
    //This is the magic formula!
    double chance = std::sqrt((double)player->getSkillModValue(9))/20 + 0.15;

    if(!gStructureManager->checkCityRadius(player))
        chance = chance*50;
    else
        chance = chance*100;

    if((gRandom->getRand() % 100) <= chance)
    {
        // YOU WIN!

        //There are two types of item to get. Bait (not rare) and Items (rare)

        //Determine Bait or Item

        uint32 itemType = 0;
        uint32 itemFamily = 0;

        uint itemCount = 0;

        if(gRandom->getRand() % 4 == 1) //Item
        {
            itemFamily = ItemFamily_Foods;

            //uint MaxItemCount = 0;
            ////Determine #
            //uint32 skillModValue = player->getSkillModValue(9);
            //
            //if(skillModValue < 25)
            //    MaxItemCount = 1;
            //else if(skillModValue < 50)
            //    MaxItemCount = 2;
            //else
            //    MaxItemCount = 3;

            //Determine Type

            uint32 temp = gRandom->getRand() % 13;

            switch(gWorldManager->getZoneId())
            {
            case 0://Corellia
            case 5://Naboo
            case 6://Rori
            case 7://Talus
            case 8://Tatooine
            {
                //Starter Planets (11 possible items)
                switch(temp)
                {
                case 0:
                    itemType = ItemType_Foraged_Berries;
                    break;
                case 1:
                    itemType = ItemType_Foraged_Bugs;
                    break;
                case 2:
                    itemType = ItemType_Sijjo_Sewi;
                    break;
                case 3:
                    itemType = ItemType_Flurr_Cle_Onion;
                    break;
                case 4:
                    itemType = ItemType_Schule_Nef;
                    break;
                case 5:
                    itemType = ItemType_Alever_Twethpek;
                    break;
                case 6:
                    itemType = ItemType_Jar_Foraged_Fungus;
                    break;
                case 7:
                    itemType = ItemType_Jar_Foraged_Grubs;
                    break;
                case 8:
                case 12:
                    itemType = ItemType_Ko_Do_Fruit;
                    break;
                case 9:
                case 11:
                    itemType = ItemType_Maroj_Melon;
                    break;
                case 10:
                    itemType = ItemType_Sosi_Hodor;
                    break;
                }
                break;
            }
            default:
            {
                //Adventure Planets (8 possible items)
                switch(temp)
                {
                case 5:
                case 0:
                    itemType = ItemType_Alever_Twethpek;
                    break;
                case 6:
                case 1:
                    itemType = ItemType_Jar_Foraged_Fungus;
                    break;
                case 7:
                case 2:
                    itemType = ItemType_Jar_Foraged_Grubs;
                    break;
                case 8:
                case 3:
                    itemType = ItemType_Ko_Do_Fruit;
                    break;
                case 9:
                case 4:
                    itemType = ItemType_Maroj_Melon;
                    break;
                case 10:
                    itemType = ItemType_Sosi_Hodor;
                    break;
                case 11:
                    itemType = ItemType_EsostEw_Zann;
                    break;
                case 12:
                    itemType = ItemType_Wild_Snaff;
                    break;
                }
                break;
            }
            }
        }
        else //Bait
        {
            itemFamily = ItemFamily_BugJar;
            uint MaxItemCount = 0;
            //Determine #
            uint32 skillModValue = player->getSkillModValue(9);

            if(skillModValue <= 25)
                MaxItemCount = 2;
            else if(skillModValue <= 50)
                MaxItemCount = 3;
            else if(skillModValue <= 75)
                MaxItemCount = 4;
            else
                MaxItemCount = 5;

            itemCount = gRandom->getRand() & 1; //Faster than rand % 2 but means same thing.
            if(itemCount == 0)
                itemCount = MaxItemCount - 1;//Lower
            if(itemCount == 1)
                itemCount = MaxItemCount; //Upper

            //Determine Type

            switch(gRandom->getRand() % 3)
            {
            case 0:
                itemType = ItemType_Insect_Bait;
                break;
            case 1:
                itemType = ItemType_Worm_Bait;
                break;
            case 2:
                itemType = ItemType_Grub_Bait;
                break;
            }
        }

        //gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_success");
        //gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_noroom");

        Inventory* inventory = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

        if(!inventory)
            return;

        if(!inventory->checkCapacity(1, player, false))
            gMessageLib->SendSystemMessage(::common::OutOfBand("skl_use", "sys_forage_noroom"), player);
        else
            gMessageLib->SendSystemMessage(::common::OutOfBand("skl_use", "sys_forage_success"), player);

        gObjectFactory->requestNewDefaultItemWithUses(inventory, itemFamily, itemType, inventory->getId(),99, glm::vec3(),"",itemCount);
    }
    else
    {
        //YOU LOSE! GOOD DAY SIR!
        gMessageLib->SendSystemMessage(::common::OutOfBand("skl_use", "sys_forage_fail"), player);
    }

    player->setForaging(false);
}






//================================================================================
//Harvesting!
//================================================================================
namespace {
// How close a scout must stand to harvest a corpse, in metres.
const float kHarvestRange = 10.0f;

// "Wild Meat" -> "meat_wild_tatooine": the planet's resource type of a
// creature's meat, hide or bone (resource_template.type_swg).
std::string resourceTypeKey(const std::string& kind, const std::string& value, const std::string& planet)
{
    std::string word = value.substr(0, value.find(' '));
    std::transform(word.begin(), word.end(), word.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (word == "domestic." || word == "domestic")
    {
        word = "domesticated";
    }
    return kind + "_" + word + "_" + planet;
}

// The planet's current spawn of a resource type, if any.
Resource* currentResource(const std::string& typeKey)
{
    ResourceIdMap* resources = gResourceManager->getResourceIdMap();
    for (ResourceIdMap::iterator it = resources->begin(); it != resources->end(); ++it)
    {
        Resource* resource = it->second;
        if (resource && resource->getCurrent() && resource->getType()
                && typeKey == resource->getType()->getDescriptor().getAnsi())
        {
            return resource;
        }
    }
    return NULL;
}

// Whether the amount fits in containers of this resource already in the
// inventory; if not, a new container needs a free inventory slot.
bool fitsInExistingContainers(PlayerObject* player, Resource* resource, uint32 amount)
{
    ObjectIDList* objects = player->getInventory()->getObjects();
    uint32 room = 0;
    for (ObjectIDList::iterator it = objects->begin(); it != objects->end(); ++it)
    {
        ResourceContainer* container = dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(*it));
        if (container && container->getResourceId() == resource->getId() && container->getAmount() < container->getMaxAmount())
        {
            room += container->getMaxAmount() - container->getAmount();
        }
    }
    return room >= amount;
}

// Puts an amount of a resource in the player's inventory: tops up a
// container of the same resource, then starts new ones.
void addToInventory(PlayerObject* player, Resource* resource, uint32 amount)
{
    Inventory* inventory = player->getInventory();
    ObjectIDList* objects = inventory->getObjects();
    for (ObjectIDList::iterator it = objects->begin(); it != objects->end() && amount > 0; ++it)
    {
        ResourceContainer* container = dynamic_cast<ResourceContainer*>(gWorldManager->getObjectById(*it));
        if (!container || container->getResourceId() != resource->getId() || container->getAmount() >= container->getMaxAmount())
        {
            continue;
        }
        uint32 add = std::min(amount, container->getMaxAmount() - container->getAmount());
        container->setAmount(container->getAmount() + add);
        amount -= add;
        gMessageLib->sendResourceContainerUpdateAmount(container, player);
        std::stringstream query;
        query << "UPDATE " << gWorldManager->getDatabase()->galaxy() << ".resource_containers SET amount="
              << container->getAmount() << " WHERE id=" << container->getId();
        gWorldManager->getDatabase()->executeAsyncSql(query);
    }
    if (amount > 0)
    {
        gObjectFactory->requestNewResourceContainer(inventory, resource->getId(), inventory->getId(), 99, amount);
    }
}
}

// A scout harvests a corpse's meat, hide or bone (or whichever it has, for
// "harvest corpse"): the creature table's quantity (creature_meat_qty...)
// plus the creature_harvesting skill mod (rules.harvest), as the planet's
// current spawn of that resource type. Once per corpse.
void ScoutManager::handleHarvestCorpse(PlayerObject* player, CreatureObject* target, HarvestSelection harvest)
{
    AttackableCreature* creature = dynamic_cast<AttackableCreature*>(target);
    if (!player || !creature || player->isDead() || player->isIncapacitated())
    {
        return;
    }
    if (!creature->isDead() || !player->checkSkill(31) || !creature->allowedToLoot(player->getId(), player->getGroupId()))
    {
        return;
    }
    if (glm::distance(player->mPosition, creature->mPosition) > kHarvestRange)
    {
        gMessageLib->SendSystemMessage(L"You are too far away to harvest that corpse.", player);
        return;
    }
    if (creature->isHarvested())
    {
        gMessageLib->SendSystemMessage(L"That corpse has already been harvested.", player);
        return;
    }

    struct Part { HarvestSelection selection; const char* kind; const char* attribute; const char* quantity; };
    const Part parts[] = {
        { HARVEST_MEAT, "meat", "res_meat", "creature_meat_qty" },
        { HARVEST_HIDE, "hide", "res_hide", "creature_hide_qty" },
        { HARVEST_BONE, "bone", "res_bone", "creature_bone_qty" },
    };
    std::vector<const Part*> available;
    for (const Part& part : parts)
    {
        if ((harvest == HARVEST_ANY || harvest == part.selection) && creature->hasAttribute(part.attribute)
                && creature->hasInternalAttribute(part.quantity) && creature->getInternalAttribute<int32>(part.quantity) > 0)
        {
            available.push_back(&part);
        }
    }
    if (available.empty())
    {
        gMessageLib->SendSystemMessage(L"There is nothing of that kind to harvest from this corpse.", player);
        return;
    }
    const Part* part = available[gRandom->getRand() % available.size()];

    std::string typeKey = resourceTypeKey(part->kind, creature->getAttribute<std::string>(part->attribute),
                                          gWorldManager->getPlanetNameThis());
    Resource* resource = currentResource(typeKey);
    if (!resource)
    {
        LOG(WARNING) << "Harvest: no current spawn of " << typeKey;
        gMessageLib->SendSystemMessage(L"There is nothing of that kind to harvest from this corpse.", player);
        return;
    }

    int32 quantity = creature->getInternalAttribute<int32>(part->quantity);
    int32 skill = player->getSkillModValue(SMod_creature_harvesting);
    int amount = quantity, xp = quantity;
    if (gCombatManager)
    {
        gCombatManager->rules().harvest(quantity, skill, amount, xp);
    }
    if (amount <= 0)
    {
        gMessageLib->SendSystemMessage(L"There is nothing of that kind to harvest from this corpse.", player);
        return;
    }

    // No room for a new container: nothing is harvested (checkCapacity tells
    // the player the inventory is full).
    if (!fitsInExistingContainers(player, resource, static_cast<uint32>(amount))
            && !player->getInventory()->checkCapacity(1, player))
    {
        return;
    }

    creature->setHarvested();
    addToInventory(player, resource, static_cast<uint32>(amount));
    if (xp > 0)
    {
        gSkillManager->addExperience(XpType_scout, xp, player);
    }
    std::string name = resource->getName().getAnsi();
    gMessageLib->SendSystemMessage(L"You harvest " + std::to_wstring(amount) + L" units of "
        + std::wstring(name.begin(), name.end()) + L".", player);
    LOG(INFO) << "Harvest by " << player->getId() << " from " << creature->getId() << ": " << amount
              << " " << typeKey << " (" << resource->getName().getAnsi() << "), " << xp << " scout XP";
}

uint32 ScoutManager::getHarvestSkillFactor(CreatureObject* object)
{
    return 0;
}


uint32 ScoutManager::getCreatureFactor(CreatureObject* object)
{
    if(!object)
        return 0;

    uint16 CL = object->getCL();

    if(CL < 9)
    {
        //Equation Adjustments
        switch(CL)
        {
        case 1:
            return 7;
        case 2:
            return 16;
        case 3:
            return 23;
        case 4:
            return 27;
        case 5:
            return 34;
        case 6:
            return 42;
        case 7:
            return 50;
        case 8:
            return 59;
        default:
        {   //Actual Equation:
            // Creature Factor = 0.1322(Challenge Level)^2 + 9.6078(Challenge Level) - 28.103
            double temp = (0.1322*(CL*CL)) + 9.6078*CL - 28.103;

            if(temp > 2000)
                return 2000;
            else
                return (uint32)temp;
        }
        }
    }
    return 0;
}

