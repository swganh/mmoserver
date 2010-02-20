/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "ScoutManager.h"
#include "Camp.h"
#include "CampRegion.h"
#include "CampTerminal.h"
#include "Item_Enums.h"
#include "nonPersistantObjectFactory.h"
#include "PlayerObject.h"
#include "Scout.h"
#include "StructureManager.h"
#include "TangibleObject.h"
#include "UIManager.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"
ScoutManager*	ScoutManager::mSingleton = NULL;

ScoutManager::~ScoutManager(void)
{
}


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

	PlayerObjectSet*			inRangePlayers	= player->getKnownPlayers();
	PlayerObjectSet::iterator	it				= inRangePlayers->begin();
	while(it != inRangePlayers->end())
	{
		PlayerObject* targetObject = (*it);
		gMessageLib->sendCreateTangible(camp,targetObject);
		targetObject->addKnownObjectSafe(camp);
		camp->addKnownObjectSafe(targetObject);
		++it;
	}

	gMessageLib->sendCreateTangible(camp,player);
	player->addKnownObjectSafe(camp);
	camp->addKnownObjectSafe(player);
	gMessageLib->sendDataTransform(camp);

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

	return true;
}
