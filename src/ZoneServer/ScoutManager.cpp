/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

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


void ScoutManager::createCamp(uint32 typeId,uint64 parentId,Anh_Math::Vector3 position,string customName, PlayerObject* player)
//gObjectFactory->requestNewDefaultItem
//(this,11,1320,entertainer->getId(),99,Anh_Math::Vector3(),"");
{
	
	//get blueprint out of the db

	StructureDeedLink*	deedData = 	gStructureManager->getDeedData(typeId);	
	
	if(!deedData)
		return;

	if(!player->checkSkill(deedData->skill_Requirement))
	{
		gMessageLib->sendSystemMessage(player,L"","camp","sys_nsf_skill");
		return;
	}

	if(!gStructureManager->checkCampRadius(player))
	{
		gMessageLib->sendSystemMessage(player,L"","camp","error_camp_too_close");
		return;
	}

	if(!gStructureManager->checkCityRadius(player))
	{
		gMessageLib->sendSystemMessage(player,L"","camp","error_nobuild");
		return;
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

	CampTerminal* terminal;

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
	region->setWidth(5.0);
	region->setHeight(5.0);
	
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
	

}
