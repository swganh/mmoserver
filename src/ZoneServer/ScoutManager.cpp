/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include <list>
#include "ScoutManager.h"
#include "Camp.h"
#include "CampRegion.h"
#include "CampTerminal.h"
#include "Item_Enums.h"
#include "nonPersistantObjectFactory.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "Scout.h"
#include "StructureManager.h"
#include "TangibleObject.h"
#include "UIManager.h"
#include "WorldManager.h"
#include "QTRegion.h"
#include "QuadTree.h"
#include "ZoneTree.h"
#include "Inventory.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"
#include "Utils/rand.h"
ScoutManager*	ScoutManager::mSingleton = NULL;


ScoutManager::ScoutManager()
{
	mSI = gWorldManager->getSI();
	pHead = NULL;
}

ScoutManager::~ScoutManager(void)
{
}

//================================================================================
//CAMPS!
//================================================================================
bool ScoutManager::createCamp(uint32 typeId,uint64 parentId,Anh_Math::Vector3 position,string customName, PlayerObject* player)
//gObjectFactory->requestNewDefaultItem
//(this,11,1320,entertainer->getId(),99,Anh_Math::Vector3(),"");
{

	//get blueprint out of the db

	StructureDeedLink*	deedData = 	gStructureManager->getDeedData(typeId);
	if(!deedData)
		return false;

	if(!player->checkSkill(deedData->skill_Requirement))
	{
		gMessageLib->sendSystemMessage(player,L"","camp","sys_nsf_skill");
		return false;
	}

	if(!gStructureManager->checkCampRadius(player))
	{
		gMessageLib->sendSystemMessage(player,L"","camp","error_camp_too_close");
		return false;
	}

	if(!gStructureManager->checkCityRadius(player))
	{
		gMessageLib->sendSystemMessage(player,L"","camp","error_nobuild");
		return false;
	}

	if(player->HasCamp())
	{
		gMessageLib->sendSystemMessage(player,L"","camp","sys_already_camping");
	}

	Camp* camp = new (Camp);
	camp->mPosition = position;

	camp->mDirection.mX = 0;
	camp->mDirection.mY = 1;
	camp->mDirection.mZ = 0;

	camp->mDirection.mW = 1;

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
	gWorldManager->createObjectinWorld(player,camp);	
	

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
	//RegionObject* region = dynamic_cast<RegionObject*>(object);
	CampRegion* region = new(CampRegion);
	region->setId(gWorldManager->getRandomNpId());

	if(terminal)
		terminal->setCampRegion(region->getId());

	region->mPosition = player->mPosition;


	//@todo need to update these sizes so they are more accurate.
	//set the bounds of the camp (so we know when the camp is abandoned

	switch(typeId)
	{
	case ItemType_Camp_basic: 
		region->setWidth(15.0); //11 Width + 2 buffer on each side
		region->setHeight(15.0); //11 Height + 2 buffer on each side
		break;
	case ItemType_Camp_elite:
		region->setWidth(18.0); //14 width + 2 buffer on each side
		region->setHeight(26.0); //22 height + 2 buffer on each side
		break;
	case ItemType_Camp_improved:
		region->setWidth(21.0); //17 width + 2 buffer on each side
		region->setHeight(18.0); //14 height + 2 buffer on each side
		break;
	case ItemType_Camp_luxury: 
		region->setWidth(46.0); //42 width + 2 buffer on each side
		region->setHeight(45.0); //41 height + 2 buffer on each side
		break;
	case ItemType_Camp_multi:
		region->setWidth(32.0); //28 height + 2 buffer on each side
		region->setHeight(34.0); //30 height + 2 buffer on each side
		break;
	case ItemType_Camp_quality:
		region->setWidth(24.0); //20 width + 2 buffer on each side
		region->setHeight(25.0); //21 height + 2 buffer on each side
		break;
	default:
		region->setWidth(5.0);
		region->setHeight(5.0);
	}

	//region->setHeight(5.0);

	//important if !0 it will never get the relevant subzone!
	region->setSubZoneId(0);
	region->setOwner(player->getId());
	region->setCamp(camp->getId());
	region->setParentId(0);
	region->setHealingModifier(deedData->healing_modifier);
	region->setActive(true);
	gWorldManager->addObject(region);

	int8 name[64];
	sprintf(name,"%s %s",player->getFirstName().getAnsi(),player->getLastName().getAnsi());
	region->setCampOwnerName(BString(name));

	player->setHasCamp(true);

	return true;
}


//================================================================================
//FORAGING!
//================================================================================

class ForageAttempt
{
public:
	ForageAttempt::ForageAttempt(PlayerObject* player, uint64 time)
	{
		startTime = time;
		playerID = player->getId();
		completed = false;
	}

	uint64 startTime;
	uint64 playerID;
	bool completed;
};

class ForagePocket
{
public:
	ForagePocket::ForagePocket(PlayerObject* player, ZoneTree* mSI)
	{
		region = mSI->getQTRegion(player->mPosition.mX,player->mPosition.mZ);

		innerRect = Anh_Math::Rectangle(player->mPosition.mX - 10,player->mPosition.mZ - 10,20,20);
		outterRect = Anh_Math::Rectangle(player->mPosition.mX - 30,player->mPosition.mZ - 30,60,60);

		pNext = NULL;
	}

	bool containsPlayer(PlayerObject* player)
	{
		if( region->mTree->ObjectContained(&innerRect, player) || region->mTree->ObjectContained(&outterRect, player) )
			return true;
		else
			return false;
	}

	void addAttempt(ForageAttempt* attempt)
	{
		attempts.push_back(attempt);
	}

	bool updateAttempts(uint64 currentTime); //if True Delete this Pocket, if False don't

	ForagePocket* pNext;

private:
	std::list<ForageAttempt*> attempts;

	QTRegion* region;
	Anh_Math::Rectangle innerRect;
	Anh_Math::Rectangle outterRect;
};

void ScoutManager::startForage(PlayerObject* player)
{
	player->setForaging(true);

	//Starts the Foraging Animation
	//player->setPosture(CreaturePosture_SkillAnimating);
	//player->setCurrentAnimation(BString("std_forage"));
	
	//gMessageLib->sendPostureUpdate(player);
	gMessageLib->sendCreatureAnimation(player, "forage");

	//player->setStationary(false);
	//gMessageLib->sendStationaryFlagUpdate(player);

	//Use up some action!
	player->getHam()->updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints, -100);

	//Creates a ForageAttempt object for tracking the forage operation
	ForageAttempt* attempt = new ForageAttempt(player, gWorldManager->GetCurrentGlobalTick());

	//FIND THE APPROPRIATE FORAGEPocket
	ForagePocket* it = pHead;
	while(it != NULL)
	{
		if(it->containsPlayer(player))
		{
			it->addAttempt(attempt);
			return;
		}
		it = it->pNext;
	}

	//None of them contained the player. We need to make new one.

	ForagePocket* new_pocket = new ForagePocket(player, mSI);
	it = pHead;
	ForagePocket* previousHead = NULL;
	while(it != NULL)
	{
		previousHead = it;
		it = it->pNext;
	}

	if(previousHead == NULL)
		pHead = new_pocket;
	else
		previousHead->pNext = new_pocket;

	new_pocket->addAttempt(attempt);
	//gLogger->logMsg("NEW FORAGING ATTEMPT", FOREGROUND_RED);
}

void ScoutManager::forageUpdate()
{
	ForagePocket* it = pHead;
	ForagePocket* previousHead = NULL;
	while(it != NULL)
	{
		if(it->updateAttempts(gWorldManager->GetCurrentGlobalTick())) //If true we delete this Pocket
		{
			//gLogger->logMsg("FORAGING TICK", FOREGROUND_RED);
			if(previousHead == NULL)
			{
				pHead = it->pNext;
			}
			else
			{
				previousHead->pNext = it->pNext;
			}
				delete it;
				it = previousHead;
		}
		if(it != NULL)
			it = it->pNext;
	}
}

void ScoutManager::successForage(PlayerObject* player)
{
	//gLogger->logMsg("FORAGING ATTEMPT SUCCESS!", FOREGROUND_RED);
	//Chance of success = sqrt(skill)/20 + 0.15
	//Chance in down = chance/2

	//First lets calc our chance to 'win'
	//This is the magic formula!	
	double chance = std::sqrt((double)player->getSkillModValue(9))/20 + 0.15;

	if(!gStructureManager->checkCityRadius(player)) chance = chance/2;

	if((gRandom->getRand() % 100) <= chance*100)
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

			uint MaxItemCount = 0;
			//Determine #
			uint32 skillModValue = player->getSkillModValue(9);
			
			if(skillModValue < 25)
				MaxItemCount = 1;
			else if(skillModValue < 50)
				MaxItemCount = 2;
			else
				MaxItemCount = 3;

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
			gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_noroom");
		else
			gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_success");

		gObjectFactory->requestNewDefaultItem(inventory, itemFamily, itemType, inventory->getId(),99,Anh_Math::Vector3(),"");
	}
	else
	{
		//YOU LOSE!
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_fail");
	}

	player->setForaging(false);
}

void ScoutManager::failForage(PlayerObject* player, forageFails fail)
{
	switch(fail)
	{
	case NOT_OUTSIDE:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_inside");
		return;
	case PLAYER_MOVED:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_movefail");
		break;
	case ACTION_LOW:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_attrib");
		return;
	case IN_COMBAT:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_cant");
		return;
	case AREA_EMPTY:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_empty");
		break;
	case ENTERED_COMBAT:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_combatfail");
		break;
	case NO_SKILL:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_noskill");
		return;
	case ALREADY_FORAGING:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_already");
		return;
	case GOT_DISCONNECTED:
		return;
	}

	player->setForaging(false);
	player->setPosture(CreaturePosture_Upright);
	gMessageLib->sendPostureUpdate(player);

}

bool ForagePocket::updateAttempts(uint64 currentTime)
{	
	if(attempts.empty())
		return true;

	std::list<ForageAttempt*>::iterator it = attempts.begin();
	unsigned int AttemptCount = 0;
	while(it != attempts.end())
	{
		if((currentTime - (*it)->startTime) >= 300000) //5minutes until we reopen the pocket
		{
			it = attempts.erase(it);
			AttemptCount--;
			//gLogger->logMsg("POCKET REMOVED DUE TO TIMER", FOREGROUND_RED);
		}
		else if((currentTime - (*it)->startTime) >= 8000 && !(*it)->completed)
		{
			//gLogger->logMsg("ATTEMPT FINISHED", FOREGROUND_RED);
			PlayerObject* player = (PlayerObject*)gWorldManager->getObjectById((*it)->playerID);
			if(player != NULL)
			{
				if(player->checkState(CreatureState_Combat))
				{
					ScoutManager::failForage(player, ENTERED_COMBAT);
					(*it)->completed = true;
					it++; AttemptCount++;
					continue;
				}

				if(region->mTree->ObjectContained(&innerRect, player) && AttemptCount < 4)
				{
					//The player has a chance to get something
					ScoutManager::successForage(player);
					(*it)->completed = true;
				}
				else
				{
					(*it)->completed = true;
					ScoutManager::failForage(player, AREA_EMPTY);
				}
			}
			it++;
			AttemptCount++;
		}
		else
		{
			PlayerObject* player = (PlayerObject*)gWorldManager->getObjectById((*it)->playerID);
			if(player && player->checkState(CreatureState_Combat))
			{
				ScoutManager::failForage(player, ENTERED_COMBAT);
				(*it)->completed = true;
			}

			it++;
			AttemptCount++;
		}
	}

	return false;
}