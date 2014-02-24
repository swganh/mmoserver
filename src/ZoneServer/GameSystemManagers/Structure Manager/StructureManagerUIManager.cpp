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
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/StructureManager.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/HarvesterObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/HouseObject.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/FactoryObject.h"
#include "ZoneServer/GameSystemManagers/Crafting Manager/ManufacturingSchematic.h"

#include "Zoneserver/Objects/Inventory.h"
#include "Zoneserver/Objects/Datapad.h"
#include "Zoneserver/Objects/Bank.h"
#include "ZoneServer/Objects/Object/ObjectFactory.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"

#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceContainer.h"
#include "ZoneServer/GameSystemManagers/Resource Manager/ResourceType.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/PlayerStructure.h"
#include "ZoneServer/WorldManager.h"

#include "ZoneServer/GameSystemManagers/UI Manager/UIManager.h"
#include "Utils/colors.h"

#include "ZoneServer\Services\equipment\equipment_service.h"
//#include "NetworkManager/DispatchClient.h"


void StructureManager::createNewFactorySchematicBox(PlayerObject* player, FactoryObject* factory)
{


    BString wText = "Current schematic installed: ";

    if(factory->getManSchemID())
    {
        BString name = factory->getSchematicCustomName();
        name.convert(BSTRType_ANSI);
        wText << name.getAnsi();


        if(!factory->getSchematicCustomName().getLength())
        {
            wText <<"@"<<factory->getSchematicFile().getAnsi()<<":"<<factory->getSchematicName().getAnsi();
        }
    }


    StringVector attributesMenu;
	uint32 enum_ = Window_Query_Add_Schematic;

	std::shared_ptr<structure_async_container> container = std::make_shared<structure_async_container>();

	container->mQueryType = Window_Query_Add_Schematic;
	//std::shared_ptr<structure_async_container> container = std::make_shared<structure_async_container>(enum_);

	container->setStructureId(factory->getId());

    //now get all man schematics in the players datapad
    Datapad* datapad								= player->getDataPad();
    ManufacturingSchematicList*				mList	= datapad->getManufacturingSchematics();
    ManufacturingSchematicList::iterator	mListIt	= mList->begin();

    while(mListIt != mList->end())
    {
        std::stringstream lText;

        ManufacturingSchematic* man = (*mListIt);
        if(!man)
        {
            mListIt++;
            continue;

        }

        Item* item = man->getItem();
        if(!item)
        {
            mListIt++;
            continue;

        }

        std::string name = std::string(item->getCustomName().begin(), item->getCustomName().end());


        //string name = factory->getSchematicCustomName();
      
        lText << name.c_str();

        if(!name.length())
        {
            lText <<"@"<<item->getNameFile().getAnsi()<<":"<<item->getName().getAnsi();
        }

		attributesMenu.push_back(lText.str());
        container->SortedList.push_back(man->getId());

        mListIt++;
    }

    uint8 LBType = SUI_LB_CANCEL_SCHEMATIC_USE;

    if(factory->getManSchemID())
        LBType =  SUI_LB_CANCEL_SCHEMATIC_REMOVEUSE;


    container->PlayerId		= player->getId();


    gUIManager->createNewListBox(this,"handleUpdateSchematic","@sui:swg", wText, attributesMenu, player, SUI_Window_Factory_Schematics,LBType,factory->getId(),0, container);
}

//============================================================================================
// Creates the status box for a structure
//
void StructureManager::createNewStructureStatusBox(PlayerObject* player, PlayerStructure* structure)
{
    //player_structure structure_name_prompt

	std::shared_ptr<structure_async_container> container = std::make_shared<structure_async_container>();
	container->setStructureId(structure->getId());

    BString wText = "Structure Name: ";
	std::string name(structure->getCustomName().begin(), structure->getCustomName().end());
    
    wText << name.c_str();


    if(!structure->getCustomName().length())
    {
        wText = "@player_structure:structure_name_prompt ";
        wText <<"@"<<structure->getNameFile().getAnsi()<<":"<<structure->getName().getAnsi();
    }

    StringVector attributesMenu;

    //Owner
    int8 text[128];
    sprintf(text,"Owner:%s",structure->getOwnersName().getAnsi());
    attributesMenu.push_back(text);


    //private vs public
    HouseObject* house = dynamic_cast<HouseObject*>(structure);
    if(house && house->getPublic())
    {
        sprintf(text,"This structure is public");
    }
    else
    {
        sprintf(text,"This structure is private");
    }

    attributesMenu.push_back(text);

    // condition
    uint32 currentCondition = structure->getMaxCondition() - structure->getDamage();

    sprintf(text,"Condition: %u%s",(uint32)(currentCondition/(structure->getMaxCondition() /100)),"%");

    attributesMenu.push_back(text);


    //Maintenance Pool
    float maint = (float)structure->getCurrentMaintenance();
    float rate  = (float)structure->getMaintenanceRate();
    uint32 hours , days, minutes;


    days = (uint32)(maint / (rate *24));
    maint -= days *(rate*24);

    hours = (uint32)(maint / rate);
    maint -= (uint32)(hours *rate);

    minutes = (uint32)(maint/(rate/60));

    sprintf(text,"Maintenance Pool: %u(%u days, %u hours, %u minutes)",(uint32)structure->getCurrentMaintenance(),days,hours,minutes);
    attributesMenu.push_back(text);

    //Maintenance rate
    sprintf(text,"Maintenance Rate: %u/hr",(uint32)rate);
    attributesMenu.push_back(text);

    //Power Pool do not display with houses or powerharvesters
    if(structure->getPlayerStructureFamily() == PlayerStructure_Harvester)
    {
        HarvesterObject* hO = dynamic_cast<HarvesterObject*>(structure);
        HarvesterFamily haFa = hO->getHarvesterFamily();

        if((haFa == HarvesterFamily_Fusion)||(haFa == HarvesterFamily_Solar)||(haFa == HarvesterFamily_Wind))
        {
            gUIManager->createNewListBox(this,"structurestatus","@player_structure:structure_status_t", wText, attributesMenu, player, SUI_Window_Structure_Status,SUI_LB_CANCELREFRESH,structure->getId(), 32, container);
            return;
        }

    }

    //no powerpool for houses :)
    if(structure->getPlayerStructureFamily() == PlayerStructure_House)
    {
        gUIManager->createNewListBox(this,"structurestatus","@player_structure:structure_status_t", wText, attributesMenu, player, SUI_Window_Structure_Status,SUI_LB_CANCELREFRESH,structure->getId(), 32, container);
        return;
    }

    uint32 power = structure->getCurrentPower();
    rate = (float)structure->getPowerConsumption();

    days = (uint32)(power / (rate *24));
    power -=(uint32)( days *(rate*24));

    hours = (uint32)(power / rate);
    power -= (uint32)(hours *rate);

    minutes = (uint32)(power/ (rate/60));

    sprintf(text,"Power Reserves: %u(%u days, %u hours, %u minutes)",structure->getCurrentPower(),days,hours,minutes);
    attributesMenu.push_back(text);

    //Power Consumption
    sprintf(text,"Power Consumption: %u units/hr",structure->getPowerConsumption());
    attributesMenu.push_back(text);


    //answer = x/(total/100);
    //answer = x*(total/100);
    // total = 100%

    gUIManager->createNewListBox(this,"structurestatus","@player_structure:structure_status_t", wText, attributesMenu, player, SUI_Window_Structure_Status,SUI_LB_CANCELREFRESH,structure->getId(), 32, container);
}



//============================================================================================
// Renames a structure
//
void StructureManager::createRenameStructureBox(PlayerObject* player, PlayerStructure* structure)
{

    BString text = "Please enter the new name you would like for this object.";

    int8 caption[32];
    sprintf(caption,"NAME THE OBJECT");

    StringVector vector;


    int8 sName[128];

    std::string name(structure->getCustomName().begin(), structure->getCustomName().end());
    
    sprintf(sName,"%s",name.c_str());
    if(!name.length())
    {
        sprintf(sName,"@%s:%s",structure->getNameFile().getAnsi(),structure->getName().getAnsi());

    }

    vector.push_back(sName);

	std::shared_ptr<structure_async_container> container = std::make_shared<structure_async_container>();
	container->setStructureId(structure->getId());

    gUIManager->createNewInputBox(this,sName,caption,text.getAnsi(),vector,player,SUI_IB_NODROPDOWN_OKCANCEL,SUI_Window_Structure_Rename,68, container);

}

//============================================================================================
//Transfers power between inventory and harvester
//
void StructureManager::createPowerTransferBox(PlayerObject* player, PlayerStructure* structure)
{

    int8 text[255];

    uint32 structurePower = structure->getCurrentPower();
    uint32 playerPower = gStructureManager->getCurrentPower(player);

    sprintf(text,"Select the amount of power you would like to deposit.\xa\xa Current Power Value = %u ",structurePower);

    int8 caption[32];
    sprintf(caption,"SELECT AMOUNT");
    int8 sName[128];

    std::string name(structure->getCustomName().begin(), structure->getCustomName().end());
    
    sprintf(sName,"%s",name.c_str());
    if(!name.length())
    {
        sprintf(sName,"@%s:%s",structure->getNameFile().getAnsi(),structure->getName().getAnsi());

    }

	std::shared_ptr<structure_async_container> container = std::make_shared<structure_async_container>();
	container->setStructureId(structure->getId());

    gUIManager->createNewTransferBox(this, sName, caption, text, "Total Energy", "To Deposit", playerPower, 0, player, SUI_Window_Deposit_Power, container);

}


//============================================================================================
//	 transfers maintenance between player and structure
//

void StructureManager::createPayMaintenanceTransferBox(PlayerObject* player, PlayerStructure* structure)
{
    int32 structureFunds = structure->getCurrentMaintenance();

    int8 text[255];
    sprintf(text,"Select the total amount you would like to pay to the existing maintenance pool.\xa\xa Current maintenance pool: %u cr.",structureFunds);

    int8 caption[32];
    sprintf(caption,"SELECT AMOUNT");
    int8 sName[128];

	std::string name(structure->getCustomName().begin(), structure->getCustomName().end());

    sprintf(sName,"%s",name.c_str());
    if(!name.length())
    {
        sprintf(sName,"@%s:%s",structure->getNameFile().getAnsi(),structure->getName().getAnsi());

    }

	auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto inventory	= dynamic_cast<Inventory*>(equip_service->GetEquippedObject(player, "inventory"));
	auto bank		= dynamic_cast<Bank*>(equip_service->GetEquippedObject(player, "bank"));

	if(!inventory)	{
		LOG (error) << "StructureManager::createPayMaintenanceTransferBox : No inventory for " << player->getId();
		return;
	}

	if(!bank)	{
		LOG (error) << "StructureManager::createPayMaintenanceTransferBox : No bank for " << player->getId();
		return;
	}

    uint32 funds = inventory->getCredits() + bank->getCredits();
        
    std::shared_ptr<structure_async_container> container = std::make_shared<structure_async_container>();
	container->setStructureId(structure->getId());

    gUIManager->createNewTransferBox(this,sName,caption,text,"Total Funds","To Pay",funds,structureFunds,player,SUI_Window_Pay_Maintenance, container);

}

void StructureManager::createNewStructureDeleteConfirmBox(PlayerObject* player, PlayerStructure* structure)
{
    //we only want this window open once
    if(player->checkUIWindow("handle Structure Destroy Confirmation"))
    {
        //gMessageLib->sendSystemMessage(player, L"You cannot open this window several times");
        return;
    }

    BString text = "Your structure";
    if(structure->getRedeed())
    {
        text <<"\\#"SOE_GREEN"WILL\\#FFFFFF ";
    }
    else
        text <<"\\#"SOE_RED"WILL NOT \\#FFFFFF";

    text <<"be redeeded. If you wish to continue with destroying your structure, please enter the following code into the input box.\xa\xa";

    int8 code [32];
    structure->setCode();
    sprintf(code,"code: %s",structure->getCode().getAnsi());
    text << code;

    int8 caption[32];
    sprintf(caption,"CONFIRM STRUCTURE DESTRUCTION");

    StringVector vector;

	std::shared_ptr<structure_async_container> container = std::make_shared<structure_async_container>();
	container->setStructureId(structure->getId());

    gUIManager->createNewInputBox(this,"handle Structure Destroy Confirmation",caption,text.getAnsi(),vector,player,SUI_IB_NODROPDOWN_OKCANCEL,SUI_Window_Structure_Delete_Confirm,6, container);

}

void StructureManager::createNewStructureDestroyBox(PlayerObject* player, PlayerStructure* structure, bool redeed)
{
    //we only want this window open once
    if(player->checkUIWindow("handle Structure Destroy")||player->checkUIWindow("handle Structure Destroy Confirmation"))
    {
        //gMessageLib->sendSystemMessage(player, L"You cannot open this window several times");
        return;
    }

    StringVector attributesMenu;

    BString text = "You have elected to destroy a structure. Pertinent structure data can be found in the list below. Please complete the following steps to confirm structure deletion.\xa\xa";
    text <<"If you wish to redeed your structure, all structure data must be \\#"SOE_GREEN" GREEN\\#. To continue with structure deletion, click YES. Otherwise, please click NO.\xa";

    if(structure->canRedeed())
    {
        text <<"WILL REDEED:\\#"SOE_GREEN" YES";

        int8 redeedText[64];
        sprintf(redeedText,"CAN REDEED: \\#"SOE_GREEN" YES");
        attributesMenu.push_back(redeedText);
    }
    else
    {
        text <<"WILL REDEED:\\#"SOE_RED" NO";

        int8 redeedText[64];
        sprintf(redeedText,"CAN REDEED:\\#"SOE_RED" NO");
        attributesMenu.push_back(redeedText);
    }

    uint32 maxCond = structure->getMaxCondition();
    uint32 cond = structure->getCondition();

    if( cond < maxCond)
    {
        int8 condition[64];
        sprintf(condition," - CONDITION: \\#"SOE_RED"%u/%u",cond,maxCond);
        attributesMenu.push_back(condition);

    }
    else
    {
        int8 condition[64];
        sprintf(condition," - CONDITION: \\#"SOE_GREEN"%u/%u",cond,maxCond);
        attributesMenu.push_back(condition);
    }

    uint32 maintIs = structure->getCurrentMaintenance();
    uint32 maintNeed = structure->getMaintenanceRate()*45;

    if(maintIs >= maintNeed)
    {
        int8 maintenance[128];
        sprintf(maintenance," - MAINTENANCE: \\#"SOE_GREEN"%u/%u",maintIs,maintNeed);
        attributesMenu.push_back(maintenance);
    }
    else
    {
        int8 maintenance[128];
        sprintf(maintenance," - MAINTENANCE: \\#"SOE_RED"%u/%u",maintIs,maintNeed);
        attributesMenu.push_back(maintenance);
    }

    int8 sName[128];

    std::string name(structure->getCustomName().begin(), structure->getCustomName().end());
    
    sprintf(sName,"%s",name.c_str());
    if(!name.length())
    {
        sprintf(sName,"@%s:%s",structure->getNameFile().getAnsi(),structure->getName().getAnsi());

    }

    //answer = x/(total/100);
    //answer = x*(total/100);
    // total = 100%

    //make sure we can only have one structure destroybox open ...

	std::shared_ptr<structure_async_container> container = std::make_shared<structure_async_container>();
	container->setStructureId(structure->getId());

    gUIManager->createNewListBox(this,"handle Structure Destroy",sName, text.getAnsi(), attributesMenu, player, SUI_Window_Structure_Delete,SUI_LB_YESNO, 0, 32, container);
}

