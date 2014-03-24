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

#include "ZoneServer/ProfessionManagers/Entertainer Manager/EntertainerManager.h"

#include "anh/logger.h"

#include "ZoneServer/GameSystemManagers/Group Manager/GroupManager.h"
#include "ZoneServer/GameSystemManagers/Group Manager/GroupManagerCallbackContainer.h"


#include "Zoneserver/GameSystemManagers/Buff Manager/Buff.h"
#include "ZoneServer/Objects/Instrument.h"
//#include "ZoneServer/GameSystemManagers/Mission Manager/MissionManager.h"
#include "ZoneServer/Objects/nonPersistantObjectFactory.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/PlayerEnums.h"
#include "ZoneServer/GameSystemManagers/State Manager//StateManager.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIManager.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/GameSystemManagers/Spatial Index Manager/SpatialIndexManager.h"
#include "ZoneServer/Objects/Weapon.h"

#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/Transaction.h"
#include "Common/atMacroString.h"
#include "Utils/utils.h"

#include "ZoneServer\Services\equipment\equipment_service.h"
#include "ZoneServer\Services\ham\ham_service.h"

bool EntertainerManager::mInsFlag = false;
EntertainerManager*	EntertainerManager::mSingleton = NULL;

//======================================================================================================================

EntertainerManagerAsyncContainer::EntertainerManagerAsyncContainer(EMQueryType qt,DispatchClient* client) :
    mQueryType(qt),
    mClient(client)
{
}

//======================================================================================================================

EntertainerManagerAsyncContainer::~EntertainerManagerAsyncContainer()
{}

//======================================================================================================================

EntertainerManager::EntertainerManager(swganh::app::SwganhKernel*	kernel)
{
    kernel_ = kernel;
    EntertainerManagerAsyncContainer* asyncContainer;

	LOG (info) << "EntertainerManager::EntertainerManager";

    // load our performance Data
    asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_LoadPerformances, 0);
    
	kernel_->GetDatabase()->executeSqlAsync(this,asyncContainer,"SELECT performanceName, instrumentAudioId, InstrumenType, danceVisualId,	actionPointPerLoop"
                                                   ",loopDuration,	florushXpMod,	healMindWound,	healShockWound, MusicVisualId FROM %s.entertainer_performances",
                                                   kernel_->GetDatabase()->galaxy());


    // load our attribute data for ID
    asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_LoadIDAttributes, 0);
    kernel_->GetDatabase()->executeSqlAsync(this,asyncContainer,"SELECT CustomizationCRC, SpeciesCRC, Atr1ID, Atr1Name, Atr2ID, Atr2Name, XP, Hair, divider FROM %s.id_attributes",
                                                   kernel_->GetDatabase()->galaxy());


    // load our holoemote Data
    asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_LoadHoloEmotes, 0);
    kernel_->GetDatabase()->executeSqlAsync(this,asyncContainer,"SELECT crc, effect_id, name FROM %s.holoemote",kernel_->GetDatabase()->galaxy());

}


//======================================================================================================================

EntertainerManager::~EntertainerManager()
{
    mInsFlag = false;
    delete(mSingleton);

}
//======================================================================================================================

EntertainerManager*	EntertainerManager::Init(swganh::app::SwganhKernel*	kernel)
{
    if(!mInsFlag)
    {
        mSingleton = new EntertainerManager(kernel);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;

}

//======================================================================================================================

void EntertainerManager::Shutdown()
{
    mPerformanceList.clear();
    mIDList.clear();
}


bool	EntertainerManager::checkAudience(PlayerObject* entertainer,CreatureObject* audience)
{
    AudienceList* mAudienceList = entertainer->getAudienceList();
    AudienceList::iterator it = mAudienceList->begin();
    while(it != mAudienceList->end())
    {
        if ((*it) == audience->getId())
        {
            return true;
        }
        ++it;
    }
    return false;
}

void EntertainerManager::handleGroupManagerCallback(uint64 playerId, GroupManagerCallbackContainer* container)
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

    bool notLeader = true;

    if(player)
    {
        switch(container->operation)
        {
        case GROUPMANAGERCALLBACK_STARTBAND: //Start Band
        {
            if(container->isLeader)
				_handleCompleteStartBand(player, container->arg);
            else
                notLeader = false;
        }
        break;

        case GROUPMANAGERCALLBACK_STOPBAND: //Stop Band
        {
            if(container->isLeader)
                _handleCompleteStopBand(player);
            else
                notLeader = false;
        }
        break;

        case GROUPMANAGERCALLBACK_BANDFLOURISH: //Band Flourish
        {
            if(container->isLeader)
                _handleCompleteBandFlourish(player, container->flourishId);
            else
                notLeader = false;
        }
        break;
        }
    }

    if(!notLeader)
    {
        //You cannot do that because you're not the damn Leader...Stupid!
        PlayerObject* notLeader = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(container->requestingPlayer));
		
        if(notLeader)
            gMessageLib->SendSystemMessage(::common::OutOfBand("group", "must_be_leader"), notLeader);
			
    }
}

//======================================================================================================================
//adds a player to the entertainers audience
//======================================================================================================================

void EntertainerManager::addAudience(PlayerObject* entertainer,CreatureObject* audience)
{
	entertainer->getAudienceList()->push_back(audience->getId());
}

//======================================================================================================================
//shows a list of all the outcasts
//======================================================================================================================

void EntertainerManager::showOutcastList(PlayerObject* entertainer)
{
    int8 sql[1000];
    int8 str1[1000];

    DenyServiceList*	deniedAudienceList	= entertainer->getDenyAudienceList();
    DenyServiceList::iterator denieIt = deniedAudienceList->begin();

	sprintf(sql,"SELECT firstname FROM %s.characters where id = ",kernel_->GetDatabase()->galaxy());

    BStringVector availableOutCasts;
    uint32 nr = 0;
    while(denieIt != deniedAudienceList->end())
    {
        uint64 id = (*denieIt);
        if (nr == 0)
        {
            sprintf(str1,"%s %"PRIu64"",sql,id);
        }
        else
        {
            sprintf(str1,"%s or %"PRIu64"",sql,id);
        }
        strcpy(sql,str1);
        //outcastIteration	= dynamic_cast<PlayerObject*> (gWorldManager->getObjectById(id));
        //availableOutCasts.push_back(outcastIteration->getFirstName());

        denieIt++;
        nr++;
    }

    if(nr > 0)
    {
        EntertainerManagerAsyncContainer* asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_DenyServiceListNames,0);
        asyncContainer->performer = entertainer;
        kernel_->GetDatabase()->executeSqlAsync(this,asyncContainer,sql);


        //gUIManager->createNewOutcastSelectBox(entertainer,"handleselectoutcast","select whom to delete from your deny service list","",availableOutCasts,entertainer,SUI_LB_OK);
    }
    else
    {
        gMessageLib->SendSystemMessage(L"Your deny service list is empty.", entertainer);
    }
}

//======================================================================================================================
//toggle a player on the entertainers denyServiceList
//======================================================================================================================

void EntertainerManager::toggleOutcastId(PlayerObject* entertainer,uint64 outCastId, BString outCastName)
{
    //check if the player is already on our list - add or remove the player

    DenyServiceList*	deniedAudienceList	= entertainer->getDenyAudienceList();
    DenyServiceList::iterator denieIt = deniedAudienceList->begin();

    bool found = false;
    while(denieIt != deniedAudienceList->end())
    {
        if (outCastId == (*denieIt))
        {
            found = true;
            //found it already on our list?????? -> remove it
            deniedAudienceList->erase(denieIt);
            break;
        }
        denieIt++;
    }


    if (found)
    {
        //tell its gone and dustoff
        PlayerObject* outCast = (PlayerObject* )gWorldManager->getObjectById(outCastId);
        if(outCast)
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("performance","deny_service_remove_other", entertainer->getId(), 0, 0, 0, 0.0f), outCast);
            gMessageLib->SendSystemMessage(::common::OutOfBand("performance","deny_service_remove_self", 0, outCastId, 0, 0, 0.0f), entertainer);
        } else {
            outCastName.convert(BSTRType_Unicode16);
            gMessageLib->SendSystemMessage(::common::OutOfBand("performance","deny_service_remove_self", L"", outCastName.getUnicode16(), L"", 0, 0.0f), entertainer);
        }
        //remove it from the db
        int8 sql[150];
        sprintf(sql,"DELETE FROM %s.entertainer_deny_service WHERE entertainer_id = '%"PRIu64"' and outcast_id = '%"PRIu64"'",kernel_->GetDatabase()->galaxy(), entertainer->getId(), outCastId);


        EntertainerManagerAsyncContainer* asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_NULL,0);
        kernel_->GetDatabase()->executeSqlAsync(this,asyncContainer,sql);

        return;
    }

    //add it
    PlayerObject* outCast = (PlayerObject* )gWorldManager->getObjectById(outCastId);
    if(outCast)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance","deny_service_add_other", entertainer->getId(), 0, 0, 0, 0.0f), outCast);
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance","deny_service_add_self", 0, outCastId, 0, 0, 0.0f), entertainer);
    } else {
        outCastName.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance","deny_service_add_self", L"", outCastName.getUnicode16(), L"", 0, 0.0f), entertainer);
    }
    deniedAudienceList->push_back(outCastId);

    //add it to the db
    int8 sql[100];
    sprintf(sql,"INSERT INTO %s.entertainer_deny_service VALUES(%"PRIu64",%"PRIu64")",kernel_->GetDatabase()->galaxy(),entertainer->getId(),outCastId);

    EntertainerManagerAsyncContainer* asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_NULL,0);
    kernel_->GetDatabase()->executeSqlAsync(this,asyncContainer,sql);


}

//======================================================================================================================
//verifies if a player is on an entertainers denyservice List
//======================================================================================================================
bool	EntertainerManager::checkDenyServiceList(PlayerObject* audience, PlayerObject* entertainer)
{
    DenyServiceList*	deniedAudienceList	= entertainer->getDenyAudienceList();
    DenyServiceList::iterator denieIt = deniedAudienceList->begin();

    //bool found = false;
    while(denieIt != deniedAudienceList->end())
    {
        if (audience->getId() == (*denieIt))
        {
            return true;
        }
        denieIt++;
    }
    return false;
}

//======================================================================================================================
//verifies if a player exists in the db
//======================================================================================================================
void EntertainerManager::verifyOutcastName(PlayerObject* entertainer,BString outCastName)
{
    int8 sql[256], name[50];
    kernel_->GetDatabase()->escapeString(name,outCastName.getAnsi(),outCastName.getLength());

    //we'll need the id only
    sprintf(sql,"SELECT id FROM %s.characters c WHERE c.firstname = '%s'",kernel_->GetDatabase()->galaxy(),name);


    EntertainerManagerAsyncContainer* asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_DenyServiceFindName,0);
    asyncContainer->performer = entertainer;
    asyncContainer->outCastName = outCastName;

    kernel_->GetDatabase()->executeSqlAsync(this,asyncContainer,sql);


}

//======================================================================================================================
//adds a player to the entertainers denyServiceList
//======================================================================================================================
void EntertainerManager::addOutcastName(PlayerObject* entertainer,PlayerObject* outcast)
{
    entertainer->getDenyAudienceList()->push_back(outcast->getId());
}

//======================================================================================================================
//removes a player from the audiencelist of an entertainer
//======================================================================================================================
void EntertainerManager::removeAudience(PlayerObject* mEntertainer,uint64 id)
{
    AudienceList* mAudienceList = mEntertainer->getAudienceList();
    AudienceList::iterator it = mAudienceList->begin();
    while(it != mAudienceList->end())
    {
        if ((*it) == id)
        {
            mAudienceList->erase(it);
            return;
        }
        ++it;
    }
}

//======================================================================================================================
//looks up the data for a specific performance
//======================================================================================================================
PerformanceStruct* EntertainerManager::getPerformance(BString performance,uint32 type)
{
    PerformanceList::iterator it = mPerformanceList.begin();
    //bool found = false;
    while(it != mPerformanceList.end())
    {
        if (BString((*it)->performanceName).getCrc() == performance.getCrc() )
        {
            if ((*it)->requiredInstrument == type )
            {
                return (*it);
            }
        }
        it++;
    }

    return NULL;
}

//======================================================================================================================
//looks up the data for a specific ImageDesign Attribute
//======================================================================================================================
IDStruct* EntertainerManager::getIDAttribute(uint32 CustomizationCRC,uint32 SpeciesCRC)
{
    IdList::iterator it = mIDList.begin();
    //bool found = false;
    while(it != mIDList.end())
    {
        if (((*it)->CustomizationCRC == CustomizationCRC)&&((*it)->SpeciesCRC == SpeciesCRC))
        {
            return (*it);
        }
        it++;
    }

    return NULL;
}

IDStruct* EntertainerManager::getIDAttribute(uint32 CustomizationCRC)
{
    IdList::iterator it = mIDList.begin();
    //bool found = false;
    while(it != mIDList.end())
    {
        if ((*it)->CustomizationCRC == CustomizationCRC)
        {
            return (*it);
        }
        it++;
    }

    return NULL;
}

//======================================================================================================================
//looks up the data for a specific performance
//======================================================================================================================

PerformanceStruct* EntertainerManager::getPerformance(BString performance)
{

    PerformanceList::iterator it = mPerformanceList.begin();
    //bool found = false;
    while(it != mPerformanceList.end())
    {
        if (BString((*it)->performanceName).getCrc() == performance.getCrc() )
        {
            return (*it);
        }
        it++;
    }

    return NULL;
}


//=======================================================================================================================
void EntertainerManager::handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result)
{
    EntertainerManagerAsyncContainer* asynContainer = (EntertainerManagerAsyncContainer*)ref;

    switch(asynContainer->mQueryType)
    {
    case EMQuery_IDFinances:
    {
        uint32 error;
        swganh::database::DataBinding* binding = kernel_->GetDatabase()->createDataBinding(1);
        binding->addField(swganh::database::DFT_uint32,0,4);
        result->getNextRow(binding,&error);
        if (error == 0)
        {
            //proceed as normal our transaction has been a success
            asynContainer->customer->getTrade()->updateCash(-asynContainer->amountcash);
            asynContainer->customer->getTrade()->updateBank(-asynContainer->amountbank);

            asynContainer->performer->getTrade()->updateBank(asynContainer->amountbank+asynContainer->amountcash);

        }
        else
        {
            LOG(error) << "Image design transaction failed";
            // oh woe we need to rollback :(
            // (ie do nothing)

        }
    }
    break;

    case EMQuery_LoadHoloEmotes:
    {
        HoloStruct* holo;

        swganh::database::DataBinding* binding = kernel_->GetDatabase()->createDataBinding(3);
        binding->addField(swganh::database::DFT_uint32,offsetof(HoloStruct,pCRC),4,0);
        binding->addField(swganh::database::DFT_uint32,offsetof(HoloStruct,pId),4,1);
        binding->addField(swganh::database::DFT_string,offsetof(HoloStruct,pEmoteName),32,2);

        uint64 count;
        count = result->getRowCount();

        for(uint64 i = 0; i < count; i++)
        {
            holo = new(HoloStruct);
            result->getNextRow(binding,holo);
            mHoloList.push_back(holo);
            BString emote("holoemote_");
            emote << holo->pEmoteName;
            holo->pClientCRC = emote.getCrc();

            emote = holo->pEmoteName;
            holo->pCRC = emote.getCrc();
        }

        //LOG(info) << "Loaded " << count << " holo emotes";

    }
    break;

    case EMQuery_IDMigrateStats:
    {
        int8 sql[1024];
        StatTargets theTargets;

        swganh::database::DataBinding* binding = kernel_->GetDatabase()->createDataBinding(9);
        binding->addField(swganh::database::DFT_uint32,offsetof(StatTargets,TargetHealth),4,0);
        binding->addField(swganh::database::DFT_uint32,offsetof(StatTargets,TargetStrength),4,1);
        binding->addField(swganh::database::DFT_uint32,offsetof(StatTargets,TargetConstitution),4,2);
        binding->addField(swganh::database::DFT_uint32,offsetof(StatTargets,TargetAction),4,3);
        binding->addField(swganh::database::DFT_uint32,offsetof(StatTargets,TargetQuickness),4,4);
        binding->addField(swganh::database::DFT_uint32,offsetof(StatTargets,TargetStamina),4,5);
        binding->addField(swganh::database::DFT_uint32,offsetof(StatTargets,TargetMind),4,6);
        binding->addField(swganh::database::DFT_uint32,offsetof(StatTargets,TargetFocus),4,7);
        binding->addField(swganh::database::DFT_uint32,offsetof(StatTargets,TargetWillpower),4,8);

        uint64	count	= result->getRowCount();
        //make sure we have some values set in case the statmigration table wont hold anything on us
        if(count != 1)
        {
            //no stats set, do nothing
        }
        else
        {
			auto ham = kernel_->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");

            //stats set update ham plus check
            result->getNextRow(binding,&theTargets);
            

            uint32 currentAmount = 0;
			for(uint32 i = 0; i<9; i++)			{
				currentAmount += asynContainer->customer->GetCreature()->GetStatBase(i);
			}

            uint32 nextAmount = theTargets.TargetHealth;
            nextAmount += theTargets.TargetStrength;
            nextAmount += theTargets.TargetConstitution;
            nextAmount += theTargets.TargetAction;
            nextAmount += theTargets.TargetQuickness;
            nextAmount += theTargets.TargetStamina;
            nextAmount += theTargets.TargetMind;
            nextAmount += theTargets.TargetFocus;
            nextAmount += theTargets.TargetWillpower;

            if(currentAmount == nextAmount)
            {
                int32 value;

				value = theTargets.TargetHealth - asynContainer->customer->GetCreature()->GetStatBase(HamBar_Health); 
				ham->UpdateBaseHitpoints(asynContainer->customer->GetCreature(),HamBar_Health, value);

                value = theTargets.TargetStrength - asynContainer->customer->GetCreature()->GetStatBase(HamBar_Strength); 
                ham->UpdateBaseHitpoints(asynContainer->customer->GetCreature(),HamBar_Strength, value);				

                value = theTargets.TargetConstitution - asynContainer->customer->GetCreature()->GetStatBase(HamBar_Constitution);
                ham->UpdateBaseHitpoints(asynContainer->customer->GetCreature(),HamBar_Constitution, value);


				value = theTargets.TargetHealth - asynContainer->customer->GetCreature()->GetStatBase(HamBar_Action); 
				ham->UpdateBaseHitpoints(asynContainer->customer->GetCreature(),HamBar_Action, value);

                value = theTargets.TargetStrength - asynContainer->customer->GetCreature()->GetStatBase(HamBar_Quickness); 
                ham->UpdateBaseHitpoints(asynContainer->customer->GetCreature(),HamBar_Quickness, value);				

                value = theTargets.TargetConstitution - asynContainer->customer->GetCreature()->GetStatBase(HamBar_Stamina);
                ham->UpdateBaseHitpoints(asynContainer->customer->GetCreature(),HamBar_Stamina, value);


				value = theTargets.TargetHealth - asynContainer->customer->GetCreature()->GetStatBase(HamBar_Mind); 
				ham->UpdateBaseHitpoints(asynContainer->customer->GetCreature(),HamBar_Mind, value);

                value = theTargets.TargetStrength - asynContainer->customer->GetCreature()->GetStatBase(HamBar_Focus); 
                ham->UpdateBaseHitpoints(asynContainer->customer->GetCreature(),HamBar_Focus, value);				

                value = theTargets.TargetConstitution - asynContainer->customer->GetCreature()->GetStatBase(HamBar_Willpower);
                ham->UpdateBaseHitpoints(asynContainer->customer->GetCreature(),HamBar_Willpower, value);


                //now the db
                EntertainerManagerAsyncContainer* asyncContainer;

                asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_NULL,0);
                sprintf(sql,"UPDATE %s.character_attributes SET health_max = %i, strength_max = %i, constitution_max = %i, action_max = %i, quickness_max = %i, stamina_max = %i, mind_max = %i, focus_max = %i, willpower_max = %i where character_id = %"PRIu64"",kernel_->GetDatabase()->galaxy(),theTargets.TargetHealth,theTargets.TargetStrength,theTargets.TargetConstitution, theTargets.TargetAction,theTargets.TargetQuickness,theTargets.TargetStamina,theTargets.TargetMind ,theTargets.TargetFocus ,theTargets.TargetWillpower ,asynContainer->customer->getId());
                kernel_->GetDatabase()->executeSqlAsync(this,asyncContainer,sql);

                asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_NULL,0);
                sprintf(sql,"UPDATE %s.character_attributes SET health_current = %i, strength_current = %i, constitution_current = %i, action_current = %i, quickness_current = %i, stamina_current = %i, mind_current = %i, focus_current = %i, willpower_current = %i where character_id = %"PRIu64"",kernel_->GetDatabase()->galaxy(),theTargets.TargetHealth,theTargets.TargetStrength,theTargets.TargetConstitution, theTargets.TargetAction,theTargets.TargetQuickness,theTargets.TargetStamina,theTargets.TargetMind ,theTargets.TargetFocus ,theTargets.TargetWillpower ,asynContainer->customer->getId());
                kernel_->GetDatabase()->executeSqlAsync(this,asyncContainer,sql);

                gSkillManager->addExperience(XpType_imagedesigner,2000,asynContainer->performer);
            }
            else
            {
                //somebodies trying to cheat here
                LOG(error) << "newHamCount != oldHamCount ";
            }


        }

        kernel_->GetDatabase()->destroyDataBinding(binding);


    }
    break;

    case EMQuery_IDMoneyTransaction:
    {
        uint32 error;
        swganh::database::DataBinding* binding = kernel_->GetDatabase()->createDataBinding(1);
        binding->addField(swganh::database::DFT_uint32,0,4);
        result->getNextRow(binding,&error);
        if (error == 0)
        {
            if(asynContainer->customer != NULL && asynContainer->customer->getConnectionState() == PlayerConnState_Connected)
            {
                asynContainer->customer->updateInventoryCredits(0-asynContainer->amountcash);
                asynContainer->customer->updateBankCredits(0-asynContainer->amountbank);
            }

            //CAVE player2 does NOT exist if the seller is NOT online
            if(asynContainer->performer != NULL && asynContainer->performer->getConnectionState() == PlayerConnState_Connected)
                asynContainer->performer->updateBankCredits(asynContainer->amountcash+asynContainer->amountbank);
        }
    }
    break;

    case EMQuery_DenyServiceListNames:
    {
        BString outCast;
        outCast.setLength(40);
        StringVector availableOutCasts;

        swganh::database::DataBinding* binding;
        binding = kernel_->GetDatabase()->createDataBinding(1);
        binding->addField(swganh::database::DFT_bstring,0,36);
        uint64 count;
        count = result->getRowCount();
        if (count > 0)
        {
            for(uint64 i = 0; i < count; i++)
            {
                result->getNextRow(binding,&outCast);
				availableOutCasts.push_back(outCast.getAnsi());
            }

            gUIManager->createNewListBox(asynContainer->performer,"handleselectoutcast","select whom to delete from your deny service list","",availableOutCasts,asynContainer->performer,SUI_Window_SelectOutcast_Listbox,SUI_LB_OK);
        }
    }
    break;
    case EMQuery_DenyServiceFindName:
    {
        uint64 outCastId;
        outCastId = 0;

        swganh::database::DataBinding* binding;
        binding = kernel_->GetDatabase()->createDataBinding(1);
        binding->addField(swganh::database::DFT_uint64,0,8);
        uint64 count;
        count = result->getRowCount();
        if (count == 1)
        {
            result->getNextRow(binding,&outCastId);
        }

        kernel_->GetDatabase()->destroyDataBinding(binding);

        PlayerObject* entertainer = asynContainer->performer;
        //is it valid?
        if(outCastId != 0)
        {
            //ok the player exists and will now be added to / removed from our DenyserviceList
            toggleOutcastId(entertainer,outCastId,asynContainer->outCastName);

        }
        else
        {
            int8 str[111];
            sprintf(str,"'%s' does not exist in the known universe",asynContainer->outCastName.getAnsi());
            BString sstr;
            sstr = BString(str);
            sstr.convert(BSTRType_Unicode16);
            gMessageLib->SendSystemMessage(sstr.getUnicode16(), entertainer);
        }
    }
    break;

    case EMQuery_LoadIDAttributes:
    {

        swganh::database::DataBinding* binding;
        binding = kernel_->GetDatabase()->createDataBinding(9);
        binding->addField(swganh::database::DFT_uint32,offsetof(IDStruct,CustomizationCRC),4,0);
        binding->addField(swganh::database::DFT_uint32,offsetof(IDStruct,SpeciesCRC),4,1);
        binding->addField(swganh::database::DFT_uint32,offsetof(IDStruct,Atr1ID),4,2);
        binding->addField(swganh::database::DFT_string,offsetof(IDStruct,Atr1Name),10,3);
        binding->addField(swganh::database::DFT_uint32,offsetof(IDStruct,Atr2ID),4,4);
        binding->addField(swganh::database::DFT_string,offsetof(IDStruct,Atr2Name),10,5);
        binding->addField(swganh::database::DFT_uint32,offsetof(IDStruct,XP),4,6);
        binding->addField(swganh::database::DFT_uint8,offsetof(IDStruct,hair),1,7);
        binding->addField(swganh::database::DFT_uint32,offsetof(IDStruct,divider),4,8);

        uint64 count;
        count = result->getRowCount();
        for(uint64 i = 0; i < count; i++)
        {
            IDStruct* idData = new(IDStruct);
            result->getNextRow(binding,idData);
            mIDList.push_back(idData);
        }

        LOG(info) << "Loaded " << count << " image designer attributes";
    }
    break;


    case EMQuery_LoadPerformances:
    {
        swganh::database::DataBinding* binding;
        binding = kernel_->GetDatabase()->createDataBinding(10);
        binding->addField(swganh::database::DFT_string,offsetof(PerformanceStruct,performanceName),32,0);
        binding->addField(swganh::database::DFT_uint32,offsetof(PerformanceStruct,instrumentAudioId),4,1);
        binding->addField(swganh::database::DFT_uint32,offsetof(PerformanceStruct,requiredInstrument),4,2);
        binding->addField(swganh::database::DFT_uint32,offsetof(PerformanceStruct,danceVisualId),4,3);
        binding->addField(swganh::database::DFT_uint32,offsetof(PerformanceStruct,actionPointPerLoop),4,4);
        binding->addField(swganh::database::DFT_uint32,offsetof(PerformanceStruct,loopDuration),4,5);
        binding->addField(swganh::database::DFT_uint32,offsetof(PerformanceStruct,florishXpMod),4,6);
        binding->addField(swganh::database::DFT_uint32,offsetof(PerformanceStruct,healMindWound),4,7);
        binding->addField(swganh::database::DFT_uint32,offsetof(PerformanceStruct,healShockWound),4,8);
        binding->addField(swganh::database::DFT_uint32,offsetof(PerformanceStruct,musicVisualId),4,9);


        uint64 count;
        count = result->getRowCount();
        for(uint64 i = 0; i < count; i++)
        {
            PerformanceStruct* mPerformanceData = new(PerformanceStruct);
            result->getNextRow(binding,mPerformanceData);
            mPerformanceList.push_back(mPerformanceData);

        }

        LOG(info) << "Loaded " << count << " performances";
    }
    break;

    default:
        break;
    }
    SAFE_DELETE(asynContainer);
}

//=======================================================================================================================
// changes the entertainers dance
//=======================================================================================================================
void	EntertainerManager::changeDance(PlayerObject* pEntertainer,BString performance)
{
    PerformanceStruct* mPerformance;
    if(pEntertainer->GetCreature()->getPerformingState()==PlayerPerformance_Dance)
    {
        mPerformance = getPerformance(performance);
        if(mPerformance != NULL)
        {
            pEntertainer->GetCreature()->setPerformance(mPerformance);

            //dance
            int8 text[32];
            sprintf(text,"dance_%u",mPerformance->danceVisualId);
            pEntertainer->GetCreature()->setCurrentAnimation(BString(text));

            //performancecounter
            gMessageLib->UpdateEntertainerPerfomanceCounter(pEntertainer->GetCreature());
            gMessageLib->sendAnimationString(pEntertainer->GetCreature());

        }
    }
}

//=======================================================================================================================
// changes the entertainers music
//=======================================================================================================================
void EntertainerManager::changeMusic(PlayerObject* entertainer,BString songString)
{
    PerformanceStruct* performance;

    //look up the old performance - get the Instrumenttype and then
    //grab the new performance for the respective Instrument
    if(entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Music)
    {
        PerformanceStruct* perStruct = (PerformanceStruct*)entertainer->GetCreature()->getPerformance();
        performance = getPerformance(songString,perStruct->requiredInstrument);
        if(performance != NULL)
        {
            entertainer->GetCreature()->setPerformance(performance);

            entertainer->GetCreature()->setPerformanceId(performance->instrumentAudioId);
            gMessageLib->sendPerformanceId(entertainer->GetCreature());
        }
    }
}

//=======================================================================================================================
// starts a music performance for the specified entertainer
//=======================================================================================================================
void	EntertainerManager::startMusicPerformance(PlayerObject* entertainer,BString performance)
{
    entertainer->setFlourishCount(0);
    PerformanceStruct* performanceStuct;

    if(entertainer->GetCreature()->getEntertainerListenToId() != 0)
        stopListening(entertainer);

    //need to get the equipped or targeted instrument

    //do we have an instrument?
    uint64 instrumentId = getInstrument(entertainer);
    if(instrumentId == 0)
    {
        entertainer->GetCreature()->setPerformingState(PlayerPerformance_None);
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_no_instrument"), entertainer);
        return;
    }

    //check if we are skilled enough to use the instrument
    if(!checkInstrumentSkill(entertainer,instrumentId))
    {
        entertainer->GetCreature()->setPerformingState(PlayerPerformance_None);
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_lack_skill_instrument"), entertainer);
        return;
    }


    //get instrument and performance specific information out of the entertainer_performances table

    Item* instrument = ((Item*) gWorldManager->getObjectById(instrumentId));

    if (instrument)
    {
        performanceStuct = getPerformance(performance,instrument->getItemType());

        if(performanceStuct == NULL)
        {
            entertainer->GetCreature()->setPerformingState(PlayerPerformance_None);
            gMessageLib->SendSystemMessage(L"Your instrument cannot be initialized.", entertainer);
            return;
        }
        //music

        entertainer->GetCreature()->setPerformance(performanceStuct);

        // This is the active instrument we will try to use.
        // We really need this info in several places...so save it.
        entertainer->setActiveInstrumentId(instrumentId);

        // Approach the instrument.
        if (!approachInstrument(entertainer, instrumentId))
        {
            // We where out of range. (using 6.0 m as default range,this value not verified).
            // TODO: Find the proper error-message, the one below is a "made up".
            gMessageLib->SendSystemMessage(::common::OutOfBand("system_msg", "out_of_range"), entertainer);
            return;
        }



        int8 text[32];
        //make sure we get the proper animation

        //sprintf(text,"music_5");
        sprintf(text,"music_%u",performanceStuct->musicVisualId);
        entertainer->GetCreature()->setCurrentAnimation(BString(text));
        gMessageLib->sendAnimationString(entertainer->GetCreature());

        entertainer->GetCreature()->setPerformanceId(performanceStuct->instrumentAudioId);
        gMessageLib->sendPerformanceId(entertainer->GetCreature());

        //performancecounter
        gMessageLib->UpdateEntertainerPerfomanceCounter(entertainer->GetCreature());
        //gMessageLib->sendEntertainerCreo6PartB(this);

        //posture
        entertainer->GetCreature()->SetPosture(CreaturePosture_SkillAnimating);
        gMessageLib->sendPostureUpdate(entertainer->GetCreature());
        //gMessageLib->sendSelfPostureUpdate(entertainer->GetCreature());

        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_start_self"), entertainer);

        //now add our scheduler
        entertainer->GetCreature()->setEntertainerTaskId(gWorldManager->addEntertainerToProccess(entertainer->GetCreature(),performanceStuct->loopDuration*1000));

        //If we have a mission in the current spot start the timer
        //gMissionManager->checkMusicianMission(entertainer);

    }
}

//=======================================================================================================================
// starts a dancing performance for the specified Entertainer
//=======================================================================================================================
void	EntertainerManager::startDancePerformance(PlayerObject* entertainer,BString performance)
{
    entertainer->setFlourishCount(0);
    PerformanceStruct* performanceStruct;

    performanceStruct = getPerformance(performance);
    if(performanceStruct != NULL)
    {
        entertainer->GetCreature()->setPerformance(performanceStruct);
        //dance
        int8 text[32];
        sprintf(text,"dance_%u",performanceStruct->danceVisualId);
        entertainer->GetCreature()->setCurrentAnimation(BString(text));

        //performancecounter
        gMessageLib->UpdateEntertainerPerfomanceCounter(entertainer->GetCreature());
        //gMessageLib->sendEntertainerCreo6PartB(this);

        //performance id
        //probably only set with Music!!
        entertainer->GetCreature()->setPerformanceId(0);
        gMessageLib->sendPerformanceId(entertainer->GetCreature());

        //posture
        entertainer->GetCreature()->SetPosture(CreaturePosture_SkillAnimating);
        gMessageLib->sendPostureUpdate(entertainer->GetCreature());

        gMessageLib->sendAnimationString(entertainer->GetCreature());

        entertainer->GetCreature()->setEntertainerWatchToId(entertainer->getId());
        entertainer->GetCreature()->setEntertainerListenToId(entertainer->getId());
        gMessageLib->sendListenToId(entertainer);

        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_start_self"), entertainer);

        //now add our scheduler
        entertainer->GetCreature()->setEntertainerTaskId(gWorldManager->addEntertainerToProccess(entertainer->GetCreature(),performanceStruct->loopDuration*1000));

        //If we have a mission in the current spot start the timer
        //gMissionManager->checkDancerMission(entertainer);

    }
}

//=======================================================================================================================
//stops the performance and removes all lists and ticks
//=======================================================================================================================
void EntertainerManager::stopEntertaining(PlayerObject* entertainer)
{

    //check if we are performing - if yes stop it

    if(entertainer->GetCreature()->getPerformingState() == PlayerPerformance_None)
        return;

    //remove us from the scheduler
    gWorldManager->removeEntertainerToProcess(entertainer->GetCreature()->getEntertainerTaskId());

    //stop our performance


    if (entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Music)
    {
        //
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_stop_self"), entertainer);
    }
    else
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_stop_self"), entertainer);
    }

    entertainer->GetCreature()->setPerformance(NULL);
    entertainer->GetCreature()->setCurrentAnimation("");
    entertainer->GetCreature()->setPerformancePaused(Pause_None);

    //posture
    if(entertainer->GetCreature()->GetPosture() == CreaturePosture_SkillAnimating)
    {
        gStateManager.setCurrentPostureState(entertainer->GetCreature(), CreaturePosture_Upright);
    }

    //gMessageLib->sendAnimationString(entertainer);

    //performance id
    entertainer->GetCreature()->setPerformanceId(0);
    gMessageLib->sendPerformanceId(entertainer->GetCreature());

    //stops music to be heard
    entertainer->GetCreature()->setEntertainerListenToId(0);
    entertainer->GetCreature()->setEntertainerWatchToId(0);
    gMessageLib->sendListenToId(entertainer);

    //iterate through the audience
    AudienceList* mAudienceList = entertainer->getAudienceList();
    AudienceList::iterator it = mAudienceList->begin();

    while (it != mAudienceList->end())
    {
		PlayerObject* audience = dynamic_cast<PlayerObject*> (gWorldManager->getObjectById(*it));
        if(audience && audience->isConnected())
        {
            if(entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Dance)
            {
                gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_stop_other", entertainer->getId(), 0, 0, 0, 0.0f), audience);
                audience->GetCreature()->setEntertainerWatchToId(0);
            }
            else
            {
                gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_stop_other", entertainer->getId(), 0, 0, 0, 0.0f), audience);
                audience->GetCreature()->setEntertainerListenToId(0);
                gMessageLib->sendListenToId(audience);
            }

            //audience->setTarget(NULL);
            //gMessageLib->sendTargetUpdateDeltasCreo6(audience);

            //caller will stop clap
            if((audience->GetCreature()->getEntertainerWatchToId() == 0)&&(audience->GetCreature()->getEntertainerListenToId() == 0))
            {
                gMessageLib->sendMoodString(audience->GetCreature(),BString(""));
                gMessageLib->sendWatchEntertainer(audience);
            }
        }

        //perhaps an npc??
        CreatureObject* npc = dynamic_cast<CreatureObject*> (gWorldManager->getObjectById((*it)));
        if(npc)
        {
            npc->setCurrentAnimation("");
            gMessageLib->sendAnimationString(npc);
            npc->setEntertainerListenToId(0);

        }
        removeAudience(entertainer,(*it));

        it = mAudienceList->begin();
    }
    mAudienceList->clear();

    //now that everythings set back set the performingstate to None
    entertainer->GetCreature()->setPerformingState(PlayerPerformance_None);

    //If we're in a mission and we've stopped the performance -- fail it
    //gMissionManager->missionFailedEntertainer(entertainer);

}

//=======================================================================================================================
// Gives Music / DanceXP
//=======================================================================================================================

void EntertainerManager::grantXP(PlayerObject* entertainer)
{
    // grant some xp

    float mPerformXP = 0;
    uint32 FlourishCount = entertainer->getFlourishCount();
    //since we can spam flourishes without them being executed (thats indeed working as intended since that was the case in precu to)
    //to make sure that the numbers are actually possible
    if((entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Music) && (FlourishCount > 1 ))
        FlourishCount = 1;

    if((entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Dance) && (FlourishCount > 2 ))
        FlourishCount = 2;


    mPerformXP = static_cast<float>(FlourishCount * ((PerformanceStruct*)entertainer->GetCreature()->getPerformance())->florishXpMod);
    entertainer->setFlourishCount(0);

    if (mPerformXP == 0)
    {
        //automatically deducts 1
        mPerformXP = entertainer->GetCreature()->getLastEntertainerXP();
    }
    else
    {
        //modify for watchers

        uint16 audienceNumbers = entertainer->getAudienceList()->size();


        //modify for performing groupmembers

        //iterate through the group
        uint32 performingMembers = 0;
        ObjectList members;
		members = gGroupManager->getInRangeGroupMembers(entertainer->GetCreature(), true);
        ObjectList::iterator memberIt = members.begin();
        while(memberIt != members.end())
        {
			CreatureObject* creature = dynamic_cast<CreatureObject*>(*memberIt );
            //check if we are performing
            if((creature))            {
				auto equip_service	= gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
				auto ghost			= dynamic_cast<PlayerObject*>(equip_service->GetEquippedObject(creature, "ghost"));

                if(creature->getPerformingState() != PlayerPerformance_None)
                {
                    performingMembers ++;
                    audienceNumbers +=	ghost->getAudienceList()->size();
                }
            }
            memberIt++;
        }

        uint32	bonusAudienceXp	=	static_cast<uint32>((mPerformXP/20)*audienceNumbers);
        uint32	bonusMembersXp	=	static_cast<uint32>((mPerformXP/20)*performingMembers);

        mPerformXP += bonusAudienceXp;
        mPerformXP += bonusMembersXp;

        mPerformXP += performingMembers;

    }

    if(mPerformXP != 0)
    {

        if(entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Music)
        {
            gSkillManager->addExperience(XpType_music,(uint32)mPerformXP,entertainer);
        }

        if(entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Dance)
        {
            gSkillManager->addExperience(XpType_dance,(uint32)mPerformXP,entertainer);
        }
    }
    entertainer->GetCreature()->setLastEntertainerXP(mPerformXP);

}

//=======================================================================================================================
// iterates through the Group and Pools the skillmods
//=======================================================================================================================
ModifierStruct EntertainerManager::getGroupHealSkillValues(PlayerObject* entertainer)
{
    ModifierStruct modStruct;

    modStruct.pHealingDanceMindMod    = 0;
    modStruct.pHealingDanceShockMod   = 0;
    modStruct.pHealingDanceWoundMod   = 0;
    modStruct.pHealingMusicMindMod    = 0;
    modStruct.pHealingMusicShockMod   = 0;
    modStruct.pHealingMusicWoundMod   = 0;

    int32 add;

    //iterate through the audience
    ObjectList members;
    members = gGroupManager->getInRangeGroupMembers(entertainer->GetCreature(), true);
    auto memberIt = members.begin();
    while(memberIt != members.end())
    {
		CreatureObject* creature = dynamic_cast<CreatureObject*>((*memberIt));
        //check if we are performing
        if(creature)        {
            if(creature->getPerformingState() != PlayerPerformance_None)
            {
                //now pool the modifiers
                modStruct.pHealingDanceMindMod   += entertainer->GetCreature()->getSkillModValue(SMod_healing_dance_mind);
                add = entertainer->GetCreature()->getSkillModValue(SMod_healing_dance_shock);
                if(add == -1000)
                    add = 0;
                modStruct.pHealingDanceShockMod   += add;
                modStruct.pHealingDanceWoundMod   += entertainer->GetCreature()->getSkillModValue(SMod_healing_dance_wound);



                modStruct.pHealingMusicMindMod   += entertainer->GetCreature()->getSkillModValue(SMod_healing_music_mind);
                add = entertainer->GetCreature()->getSkillModValue(SMod_healing_music_shock);
                if(add == -1000)
                    add = 0;
                modStruct.pHealingMusicShockMod   += add;
                modStruct.pHealingMusicWoundMod   += entertainer->GetCreature()->getSkillModValue(SMod_healing_music_wound);

            }
        }
        memberIt++;
    }
    return modStruct;
}

//=======================================================================================================================
// iterates through the buffmap and adjusts buffvalues as necessary
//=======================================================================================================================
void EntertainerManager::buff(PlayerObject* entertainer)
{
    PlayerObject* audience;

    BuffMap::iterator buffIt = entertainer->getEntertainerBuffMap()->begin();
    while(buffIt != entertainer->getEntertainerBuffMap()->end())
    {
        BuffStruct* buffStruct = (*buffIt).second;
        audience = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById((*buffIt).first));

        if(!audience)
        {
            entertainer->getEntertainerBuffMap()->erase(buffIt++);
            continue;
        }
        //check whether we are still grouped - if not discard
        if((entertainer->GetCreature()->getGroupId() != 0) &&(entertainer->GetCreature()->getGroupId() == audience->GetCreature()->getGroupId()))
        {

            uint32	flourishCount = entertainer->getFlourishCount();

            if(flourishCount > 5 )
                flourishCount = 5;

            //first the buffs length

            uint32 malus = flourishCount + 1;
            buffStruct->buffLengthSeconds += (60*malus);

            //determine the time the buff can last
            uint32 maxtime = 3600;
            if(!buffStruct->passive)
                maxtime  = 7200;

            //make sure we get not longer
            if(buffStruct->buffLengthSeconds > maxtime)
            {
                buffStruct->buffLengthSeconds = maxtime;
            }

            //now the amount we buff for
            float buffPercentageDance = static_cast<float>(entertainer->GetCreature()->getSkillModValue(SMod_healing_dance_mind));
            float buffPercentageMusic = static_cast<float>(entertainer->GetCreature()->getSkillModValue(SMod_healing_music_mind));

            if(buffPercentageDance > 125)
                buffPercentageDance = 125;

            if(buffPercentageMusic > 125)
                buffPercentageMusic = 125;

            if(entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Dance)
            {
                buffStruct->buffValuePercent += (buffPercentageDance/10)+1;
            }
            else
            {
                buffStruct->buffValuePercent += (buffPercentageMusic/10)+1;
            }

            if(buffStruct->buffValuePercent > 100.0)
                buffStruct->buffValuePercent = 100.0;

            buffIt++;

        }
        else
        {
            //we are not grouped anymore - just remove
            entertainer->getEntertainerBuffMap()->erase(buffIt++);
        }

    }
}

//=======================================================================================================================
// iterates through the audience heals mind wounds and BF due to stats and gives associated xp
//=======================================================================================================================
void EntertainerManager::heal(PlayerObject* entertainer)
{
	auto ham = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");

    //are we in a group???
    //If yes iterate through groupmembers of our profession (dancer musician or entertainer)
    //see if they are performing and stack the healing modifiers
    //please note that dance modifiers dont stack on musician modifiers
    //http://wiki2.swganh.org/index.php/Entertainer_%28Game_Mechanics%29

    /*
    battlefatigue exp = bf_amount healed x 2

    primary wound healing experience = primary_amount healed x 0.5

    secondary wound experience = secondary_amount healed x 0.1

    buffing experience = every 1% of pool buffed grants 1 experience point.

    For information on buffing experience, see dancer and musician sections on the wik
    */

    //healing only in proper places
    EMLocationType loc = entertainer->getPlayerLocation();
    if(loc != EMLocation_Cantina || loc != EMLocation_Camp || loc != EMLocation_PlayerStructure)
    {
        return;
    }

    ModifierStruct modStruct = getGroupHealSkillValues(entertainer);
    uint32 pBaseHealingMind = ((PerformanceStruct*)entertainer->GetCreature()->getPerformance())->healMindWound;
    uint32 pBaseHealingShock = ((PerformanceStruct*)entertainer->GetCreature()->getPerformance())->healShockWound;

    float pTotalMindHeal = 0;
    float pTotalShockHeal = 0;

    if(entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Dance)
    {
        //total healed per tick = base healing amount x ( (100 + healingmodifier) / 100 )

        //Mindwound		//willpower		//focus
        pTotalMindHeal = static_cast<float>(pBaseHealingMind * ((100+ modStruct.pHealingDanceWoundMod)/100));

        //bf
        pTotalShockHeal = static_cast<float>(pBaseHealingShock * ((100+ modStruct.pHealingDanceShockMod)/100));

    }
    else
    {
        //Mindwound		//willpower		//focus
        pTotalMindHeal = static_cast<float>(pBaseHealingMind * ((100+ modStruct.pHealingMusicWoundMod)/100));

        //bf
        pTotalShockHeal = static_cast<float>(pBaseHealingShock * ((100+ modStruct.pHealingMusicShockMod)/100));
    }

    //half the heal without flourishes
    if (entertainer->getFlourishCount() == 0)
    {
        pTotalMindHeal = pTotalMindHeal /2;
        pTotalShockHeal = pTotalShockHeal /2;
    }

    //check if we are in a cantina otherwise no BF heal
    if(loc != EMLocation_Cantina)
    {
        pTotalShockHeal = 0;
    }

    //heal our own wounds
    //Mind
	ham->UpdateWound(entertainer->GetCreature(), HamBar_Mind, static_cast<int32>(-pTotalMindHeal));
	ham->UpdateWound(entertainer->GetCreature(), HamBar_Willpower, static_cast<int32>(-pTotalMindHeal));
	ham->UpdateWound(entertainer->GetCreature(), HamBar_Focus, static_cast<int32>(-pTotalMindHeal));
    
    //heal bf
	ham->UpdateBattleFatigue(entertainer->GetCreature(),-pTotalShockHeal);
    
    //iterate through the audience
    AudienceList* mAudienceList = entertainer->getAudienceList();
    AudienceList::iterator it = mAudienceList->begin();

    //out of the loop we accumulate all heals and then calculate the xp at the end of the healingtick
    uint32 pCompleteBFHealAmount = 0;
    uint32 pCompleteMindHealAmount = 0;
    uint32 pCompleteSecondaryHealAmount = 0;

    while (it != mAudienceList->end())
    {
        PlayerObject* audience = dynamic_cast<PlayerObject*> (gWorldManager->getObjectById(*it));
        if(audience)
        {
			ham->UpdateWound(audience->GetCreature(), HamBar_Mind, static_cast<int32>(-pTotalMindHeal));
			ham->UpdateWound(audience->GetCreature(), HamBar_Willpower, static_cast<int32>(-pTotalMindHeal));
			ham->UpdateWound(audience->GetCreature(), HamBar_Focus, static_cast<int32>(-pTotalMindHeal));

            ham->UpdateBattleFatigue(audience->GetCreature(),-pTotalShockHeal);

            ++it;
        }
    }

    float pHealingXP;
    float pHealingXP2nd;
    pHealingXP = static_cast<float>(pCompleteBFHealAmount * 2);

    pHealingXP2nd = static_cast<float>(pCompleteSecondaryHealAmount /10);
    pHealingXP2nd += pCompleteMindHealAmount /2;

    auto members = gGroupManager->getInRangeGroupMembers(entertainer->GetCreature(), true);
    auto memberIt = members.begin();
    while(memberIt != members.end())    {
		CreatureObject* creature = dynamic_cast<CreatureObject*>(*memberIt);

        //check if we are performing
        if(creature->getPerformingState() != PlayerPerformance_None)
        {
			auto equip_service	= gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
			auto group_member		= dynamic_cast<PlayerObject*>(equip_service->GetEquippedObject(creature, "ghost"));

            if(((uint32)pHealingXP) != 0)
                gSkillManager->addExperience(XpType_entertainer_healing,(uint32)pHealingXP,group_member);

            if(((uint32)pHealingXP2nd) != 0)
                gSkillManager->addExperience(XpType_entertainer_healing,(uint32)pHealingXP2nd,group_member);


        }
        memberIt++;
    }
    //todo :
    //make Groupmembers entertaining share in the healing xp
    //find out how to adjust attributes and bf

}

//=======================================================================================================================
//checks the distance between entertainer and audience and
//determines if we still can watch or listen
//=======================================================================================================================
void EntertainerManager::CheckDistances(PlayerObject* entertainer)
{
    //iterate through the audience

	CreatureObject* entertainer_body = entertainer->GetCreature();
	

    if(!entertainer->getAudienceList())
    {
        DLOG(info) << "CheckDistances(PlayerObject* entertainer) getAudienceList does not exist !!!!!";
        return;
    }

    AudienceList* mAudienceList = entertainer->getAudienceList();
    AudienceList::iterator it = mAudienceList->begin();

    while (it != mAudienceList->end())
    {
        //are we in range?
        PlayerObject* audience = dynamic_cast<PlayerObject*> (gWorldManager->getObjectById((*it)));
        if(audience)        {
			CreatureObject* audience_body = audience->GetCreature();
            if(glm::distance(entertainer_body->mPosition, audience_body->mPosition) > 60)
            {
                if(entertainer_body->getPerformingState() == PlayerPerformance_Dance)
                {
                    stopWatching(audience,true);
                }
                else
                {
                    stopListening(audience,true);
                }

                it = mAudienceList->begin();

            }
            else
            {
                it++;
            }
        }
        else
        {
            it++;
        }
    }
}




//=======================================================================================================================
//makes the player Object stop watching
//=======================================================================================================================
void EntertainerManager::stopWatching(PlayerObject* audience,bool ooRange)
{
	auto ham = kernel_->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");

    PlayerObject* entertainer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(audience->GetCreature()->getEntertainerWatchToId()));

    if(audience->GetCreature()->getEntertainerWatchToId()== 0 )
    {
        //make sure that we are watching an entertainer
        //gMessageLib->sendSystemMessage(audience,L"","performance","dance_fail");
        return;
    }
    if(entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Dance)//who is dancing
    {
        audience->GetCreature()->setTarget(0);
        gMessageLib->sendTargetUpdateDeltasCreo6(audience->GetCreature());

        // the caller is now removed from the audienceList
		gEntertainerManager->removeAudience(entertainer,audience->getId());

        audience->GetCreature()->setEntertainerWatchToId(0);
        // make sure we are not listening to somebody before stopping being entertained
        if (audience->GetCreature()->getEntertainerListenToId()== 0 )
        {
            // caller will stop clap
            gMessageLib->sendMoodString(audience->GetCreature(),BString(""));
            gMessageLib->sendWatchEntertainer(audience);
        }

        if(ooRange)
            gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_watch_out_of_range", 0, entertainer->getId(), 0, 0, 0.0f), audience);
        else
            gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_watch_stop_self"), audience);


        // now see whether we are grouped and in the buff map
        if((entertainer->GetCreature()->getGroupId() != 0) &&(entertainer->GetCreature()->getGroupId() == audience->GetCreature()->getGroupId()))
        {
            //check whether we are at a valid location
            EMLocationType loc = entertainer->getPlayerLocation();
            if(loc != EMLocation_PlayerStructure || loc != EMLocation_Camp || loc != EMLocation_Cantina)
            {
                BuffMap::iterator buffIt = entertainer->getEntertainerBuffMap()->find(audience->getId());
                if(buffIt  != entertainer->getEntertainerBuffMap()->end())
                    entertainer->getEntertainerBuffMap()->erase(buffIt++);
                return;
            }

            // make sure we are buffing
            // finding the record means that we are applying the buff!!
            BuffMap::iterator buffIt = entertainer->getEntertainerBuffMap()->find(audience->getId());
            if(buffIt != entertainer->getEntertainerBuffMap()->end())
            {
                // get the relevant modificators
                // and make sure we apply them

                // SMod_healing_dance_mind determines how big the mindbuff is
                // 100% is the mind the customer has
                float	buffPercentageDance = (float)entertainer->GetCreature()->getSkillModValue(SMod_healing_dance_mind);
                buffPercentageDance = buffPercentageDance /100;

                // buffvaluepercent tells us how much of the 100% value of the mind we can apply
                // (in other words the buffstrength according to the time we have been listening)
                float	percentage  = ((*buffIt).second->buffValuePercent/100);
                uint32	time	    = (*buffIt).second->buffLengthSeconds;

                // apply it
				int32	mind		= audience->GetCreature()->GetStatBase(HamBar_Mind);
                mind				= static_cast<uint32>(mind*percentage);
                mind				= static_cast<uint32>(mind*buffPercentageDance);

                //yay!!! we got ourselves a buff!!!
                BuffAttribute* mindAttribute = new BuffAttribute(mind, +mind,0,-(int)mind);
                Buff* mindBuff = Buff::SimpleBuff(audience->GetCreature(), audience->GetCreature(), time*1000, opBACRC_PerformanceMind, gWorldManager->GetCurrentGlobalTick());
                mindBuff->AddAttribute(mindAttribute);
                audience->GetCreature()->AddBuff(mindBuff,true);

                if(buffIt  != entertainer->getEntertainerBuffMap()->end())
                    entertainer->getEntertainerBuffMap()->erase(buffIt++);

                buffIt++;
            }

        }


    }
    else
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_fail"), audience);
    }

}

//=======================================================================================================================
//makes a player object stop listening
//=======================================================================================================================
void EntertainerManager::stopListening(PlayerObject* audience,bool ooRange)
{

    PlayerObject* entertainer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(audience->GetCreature()->getEntertainerListenToId()));

    if(audience->GetCreature()->getEntertainerListenToId()== 0 )
    {   //make sure that we are listening to an entertainer
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_fail"), audience);

        return;
    }

    if(entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Music)
    {   //who is making music

        //stops music to be heard
        audience->GetCreature()->setEntertainerListenToId(0);
        gMessageLib->sendListenToId(audience);

        //the caller is now removed from the audienceList
        removeAudience(entertainer,audience->getId());

        if(ooRange)
            gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_listen_out_of_range", 0, entertainer->getId(), 0), audience);
        else
            gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_listen_stop_self"), audience);

        audience->GetCreature()->setTarget(0);
        gMessageLib->sendTargetUpdateDeltasCreo6(audience->GetCreature());

        if (audience->GetCreature()->getEntertainerWatchToId()== 0 )
        {   //make sure we are not watching somebody before stopping being entertained
            //caller will stop clap
            gMessageLib->sendMoodString(audience->GetCreature(),BString(""));
            gMessageLib->sendWatchEntertainer(audience);
        }

        //now see whether we are grouped and in the buff map
        if((entertainer->GetCreature()->getGroupId() != 0) &&(entertainer->GetCreature()->getGroupId() == audience->GetCreature()->getGroupId()))
        {
            //check whether we are at a valid location
            EMLocationType loc = entertainer->getPlayerLocation();
            if(loc != EMLocation_PlayerStructure || loc != EMLocation_Camp || loc != EMLocation_Cantina)
            {
                BuffMap::iterator buffIt = entertainer->getEntertainerBuffMap()->find(audience->getId());

                if(buffIt  != entertainer->getEntertainerBuffMap()->end())
                    entertainer->getEntertainerBuffMap()->erase(buffIt++);
                return;
            }

            // make sure we are buffing
            // finding the record means that we are applying the buff!!
            BuffMap::iterator buffIt = entertainer->getEntertainerBuffMap()->find(audience->getId());
            if(buffIt != entertainer->getEntertainerBuffMap()->end())
            {
                // get the relevant modificators
                // and make sure we apply them

                // SMod_healing_dance_mind determines how big the mindbuff is
                // 100% is the mind the customer has
                float buffPercentageDance = static_cast<float>(entertainer->GetCreature()->getSkillModValue(SMod_healing_music_mind)/100);


                // buffvaluepercent tells us how much of the 100% value of the mind we can apply
                // (in other words the buffstrength according to the time we have been listening)
                float	percentage  = ((*buffIt).second->buffValuePercent/100);
                uint32	time	    = (*buffIt).second->buffLengthSeconds;

                // apply it
				int32	focus		= audience->GetCreature()->GetStatBase(HamBar_Focus);// >g etHam()->getPropertyValue(HamBar_Focus,HamProperty_BaseHitpoints);
                focus				= static_cast<uint8>((focus*percentage));
                focus				= (int)(focus*buffPercentageDance);

                int32	will		= audience->GetCreature()->GetStatBase(HamBar_Willpower);//audience->getHam()->getPropertyValue(HamBar_Willpower,HamProperty_BaseHitpoints);
                will				= static_cast<uint32>(will*percentage);
                will				= static_cast<uint32>(will*buffPercentageDance);

                //yay!!! we got ourselves a buff!!!
                BuffAttribute* focusAttribute = new BuffAttribute(attr_focus, +focus,0,-(int)focus);
                Buff* focusBuff = Buff::SimpleBuff(audience->GetCreature(), audience->GetCreature(), time*1000, opBACRC_PerformanceFocus, gWorldManager->GetCurrentGlobalTick());
                focusBuff->AddAttribute(focusAttribute);
                audience->GetCreature()->AddBuff(focusBuff,true);

                BuffAttribute* willAttribute = new BuffAttribute(attr_willpower, +will,0,-(int)will);
                Buff* willBuff = Buff::SimpleBuff(audience->GetCreature(), audience->GetCreature(), time*1000, opBACRC_PerformanceWill, gWorldManager->GetCurrentGlobalTick());
                willBuff->AddAttribute(willAttribute);
                audience->GetCreature()->AddBuff(willBuff,true);

                if(buffIt  != entertainer->getEntertainerBuffMap()->end())
                    entertainer->getEntertainerBuffMap()->erase(buffIt++);
            }

        }

    }
}

//=======================================================================================================================
//makes a player object start listening
//=======================================================================================================================
void EntertainerManager::startListening(PlayerObject* audience, PlayerObject* entertainer)
{

    //are we already watching/listening to this entertainer??
    if(checkAudience(entertainer,audience->GetCreature()))
    {
        //we shouldnt be able to watch him several times or listen / watch at the same time
        gMessageLib->SendSystemMessage(L"You are already being entertained by this player.", audience);
        return;
    }

    if(audience->GetCreature()->getEntertainerListenToId()== entertainer->getId())
    {
        gMessageLib->SendSystemMessage(L"You are already being entertained by this player.", audience);
        return;
    }

    //is the audience on our denyservice list?
    if (checkDenyServiceList(audience,entertainer))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "deny_service_add_other"), audience);
        return;
    }

    // is the entertainer valid???
    if(entertainer == NULL || entertainer == audience)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_fail"), audience);
        return;
    }

    //is the entertainer near enough???
	if(glm::distance(entertainer->GetCreature()->mPosition, audience->GetCreature()->mPosition) > 60)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_fail"), audience);
        return;
    }

    if(entertainer->GetCreature()->getPerformingState() != PlayerPerformance_Music)
    {
        //we only can watch entertainers who are dancing!!!!
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_fail"), audience);
        return;
    }

    if(audience->GetCreature()->getPerformingState() != PlayerPerformance_None)
    {
        //we only can watch when we are not performing ourselves
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_fail"), audience);
        return;
    }


    //start by adding the id of the dancer we are watching to our Object
    audience->GetCreature()->setEntertainerListenToId(entertainer->getId());
    gMessageLib->sendListenToId(audience);

    audience->GetCreature()->setTarget(entertainer->getId());
    gMessageLib->sendTargetUpdateDeltasCreo6(audience->GetCreature());

    //makes the caller clap
    if (audience->GetCreature()->getEntertainerWatchToId()== 0 )//make sure we are not watching somebody before sending the packet
    {
        gMessageLib->sendMoodString(audience->GetCreature(),BString("entertained"));
        gMessageLib->sendWatchEntertainer(audience);
    }

    //add the caller to our audience List
    gEntertainerManager->addAudience(entertainer,audience->GetCreature());

    //string name = entertainer->getFirstName().getAnsi();
    gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_listen_self", 0, entertainer->getId(), 0), audience);

    //are we in one group???
    if((entertainer->GetCreature()->getGroupId() != 0) &&(entertainer->GetCreature()->getGroupId() == audience->GetCreature()->getGroupId()))
    {
        //make sure we have not already an entry going ?
        //BuffMap::iterator buffIt = entertainer->getEntertainerBuffMap()->begin();
        BuffMap::iterator buffIt = entertainer->getEntertainerBuffMap()->find(audience->getId());
        if(buffIt != entertainer->getEntertainerBuffMap()->end())
        {
            // the map is per entertainer so we can very well be on more thn one entertainer list
            // however when we start watching the one we stop watching the one before
            // so it is never more than one buff of one kind at once
            entertainer->getEntertainerBuffMap()->erase(buffIt++);
        }

        //yes! lets start the (passive) buff - dancers buff mind
        BuffStruct* buff = new(BuffStruct);

        buff->buffLengthSeconds = 0;
        buff->buffStart			= gWorldManager->GetCurrentGlobalTick();
        buff->buffType			= EMBuff_Secondary;
        buff->buffValuePercent	= 0;
        buff->customer			= audience->getId();
        buff->passive			= true;

        entertainer->getEntertainerBuffMap()->insert(std::make_pair(audience->getId(),buff));

    }

}

//=======================================================================================================================
//makes a player object start watching
//=======================================================================================================================
void EntertainerManager::startWatching(PlayerObject* audience, PlayerObject* entertainer)
{

    if(checkAudience(entertainer,audience->GetCreature()))
    {
        //we shouldnt be able to watch him several times or listen / watch at the same time
        return;
    }

    // is the entertainer valid???
    if(entertainer == NULL || entertainer == audience)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_fail"), audience);
        return;
    }

    //is the audience on our denyservice list?
    if (checkDenyServiceList(audience,entertainer))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "deny_service_add_other"), audience);
        return;
    }


    //is the entertainer near enough???
    //TODO range configureable ??
	if(glm::distance(entertainer->GetCreature()->mPosition, audience->GetCreature()->mPosition) > 60)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_fail"), audience);
        return;
    }

    if(entertainer->GetCreature()->getPerformingState() != PlayerPerformance_Dance)
    {
        //we only can watch entertainers who are dancing!!!!
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_fail"), audience);
        return;
    }

    if(audience->GetCreature()->getPerformingState() != PlayerPerformance_None)
    {
        //we only can watch when we are not performing ourselves
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_fail"), audience);
        return;
    }

    //atMacroString* aMS = new atMacroString();

    audience->GetCreature()->setTarget(entertainer->getId());
    gMessageLib->sendTargetUpdateDeltasCreo6(audience->GetCreature());

    if (audience->GetCreature()->getEntertainerWatchToId()== 0 )//make sure we are not watching somebody before sending the packet
    {
        //start by adding the id of the dancer we are watching to our Object
        audience->GetCreature()->setEntertainerWatchToId(entertainer->getId());

        //makes the caller clap
        gMessageLib->sendMoodString(audience->GetCreature(),BString("entertained"));
        gMessageLib->sendWatchEntertainer(audience);
    }


    //add the caller to our audience List
    gEntertainerManager->addAudience(entertainer,audience->GetCreature());
    gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_watch_self", 0, entertainer->getId(), 0), audience);

    //are we in one group???
    if((entertainer->GetCreature()->getGroupId() != 0) &&(entertainer->GetCreature()->getGroupId() == audience->GetCreature()->getGroupId()))
    {
        //make sure we have not already an entry going ?
        //BuffMap::iterator buffIt = entertainer->getEntertainerBuffMap()->begin();
        BuffMap::iterator buffIt = entertainer->getEntertainerBuffMap()->find(audience->getId());
        if(buffIt != entertainer->getEntertainerBuffMap()->end())
        {
            entertainer->getEntertainerBuffMap()->erase(buffIt++);
        }

        //yes! lets start the (passive) buff - dancers buff mind
        BuffStruct* buff = new(BuffStruct);

        buff->buffLengthSeconds = 0;
        buff->buffStart			= gWorldManager->GetCurrentGlobalTick();
        buff->buffType			= EMBuff_Primary;
        buff->buffValuePercent	= 0;
        buff->customer			= audience->getId();
        buff->passive			= true;

        entertainer->getEntertainerBuffMap()->insert(std::make_pair(audience->getId(),buff));

    }
}


//=======================================================================================================================
//stops the pause and resumes with the performance
//=======================================================================================================================
void EntertainerManager::handlePerformancePause(CreatureObject* mObject)
{
    PlayerObject*	entertainer	= dynamic_cast<PlayerObject*>(mObject);
    int8 text[32];
    if(entertainer->GetCreature()->getPerformance() == NULL) {
        return;
    }

    //see if we have to start a Pause
    if(entertainer->GetCreature()->getPerformancePaused() == Pause_Start)
    {
        int8  animation[32];
        sprintf(animation,"skill_action_0");

        gMessageLib->sendCreatureAnimation(entertainer->GetCreature(), BString(animation));
        gMessageLib->sendperformFlourish(entertainer, 0);

        gStateManager.setCurrentPostureState(entertainer->GetCreature(), CreaturePosture_Upright);
        gMessageLib->sendPostureUpdate(entertainer->GetCreature());
        //gMessageLib->sendSelfPostureUpdate(entertainer);

        entertainer->GetCreature()->setCurrentAnimation("");
        gMessageLib->sendAnimationString(entertainer->GetCreature());

        //entertainer->setEntertainerPauseId(gWorldManager->addEntertainerPause(entertainer,((PerformanceStruct*)entertainer->getPerformance())->loopDuration*1000));
        entertainer->GetCreature()->setPerformancePaused(Pause_Paused);
    }
    else if(entertainer->GetCreature()->getPerformancePaused() == Pause_Paused)
    {
        entertainer->GetCreature()->setPerformancePaused(Pause_None);
        sprintf(text,"dance_%u",((PerformanceStruct*)entertainer->GetCreature()->getPerformance())->danceVisualId);
        entertainer->GetCreature()->setCurrentAnimation(BString(text));
        gMessageLib->sendAnimationString(entertainer->GetCreature());

        gStateManager.setCurrentPostureState(entertainer->GetCreature(),CreaturePosture_SkillAnimating);
    }
}

//=======================================================================================================================
//checks if we still are skillanimating
//invokes the healing and xp subroutines
//checks if we have enough action in our pool
//=======================================================================================================================
bool EntertainerManager::handlePerformanceTick(CreatureObject* mObject)
{
	auto ham = kernel_->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");
    //check if we are still performing otherwise delete the tick and
    //stop performing if our state != 9 (dancing)
    PlayerObject*	entertainer	= dynamic_cast<PlayerObject*>(mObject);
    if(!entertainer)
        return false;

    //check if we need to stop the performance or if it already has been stopped
    //Mind the pausing dancer though
    handlePerformancePause(entertainer->GetCreature());
    if((entertainer->GetCreature()->GetPosture() != CreaturePosture_SkillAnimating)&&(entertainer->GetCreature()->getPerformancePaused() == Pause_None))
    {
        //stop our performance for ourselves and all watchers
        stopEntertaining(entertainer);
        return (false);
    }

    //check distance and remove offending audience
    CheckDistances(entertainer);

    //heal BF and Mindwounds
    //put it before grantXp so the flourishcount is unaffected by the XPs resetting it
    heal(entertainer);


    buff(entertainer);

    //grant xp
    grantXP(entertainer);

    // check ham costs
    

    if(ham->checkMainPools(entertainer->GetCreature(), 0,((PerformanceStruct*)entertainer->GetCreature()->getPerformance())->actionPointPerLoop,0))
    {
        PerformanceStruct* pStruct = (PerformanceStruct*)entertainer->GetCreature()->getPerformance();
        int32 actionCost = pStruct->actionPointPerLoop;
		ham->ApplyHamCost(entertainer->GetCreature(), HamBar_Action, -(actionCost));
		return (true);
    }
    else
    {
        //not enough action stop dancing



        ::common::ProsePackage prose;

        if(entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Dance) {
            prose.base_stf_file = "performance";
            prose.base_stf_label = "dance_too_tired";
        } else {
            prose.base_stf_file = "performance";
            prose.base_stf_label = "music_too_tired";
        }

        //do the sys message before that so we get it right
        stopEntertaining(entertainer);

        gMessageLib->SendSystemMessage(::common::OutOfBand(prose), entertainer);

        return (false);

    }

}


//=======================================================================================================================
//placed Instruments like omnibox/nalarbon/bandfill use this
//warp the instrument to the user display a list of known songs and then start playing
//=======================================================================================================================

void EntertainerManager::playInstrument(PlayerObject* entertainer, Item* instrument)
{
    if(!entertainer || !instrument)
    {
        return;
    }

    uint64	instrumentId	= 0;

	auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto item	= dynamic_cast<Item*>(equip_service->GetEquippedObject(entertainer, "hold_r"));

    // check if the weapon slot is in use by something else than the unarmed default weapon
    if(item && item->getId() != entertainer->getId()+WEAPON_OFFSET)        {
        // TODO: put another message ?
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_must_unequip"), entertainer);
        return;
    }

    if ((instrument->getItemType() == ItemType_Nalargon) || (instrument->getItemType() == ItemType_nalargon_max_reebo) || (instrument->getItemType() == ItemType_omni_box))
    {
        if (gWorldManager->objectsInRange(entertainer->getId(), instrumentId, 6.0))
        {
            if(entertainer->getParentId() != instrument->getParentId())
            {
                gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_must_unequip"), entertainer);
                return;
            }

            // We are in range.
            //move player to instrument vs move instrument to player
            entertainer->mDirection = instrument->mDirection;
            entertainer->GetCreature()->updatePosition(instrument->getParentId(),instrument->mPosition);


        }
    }

    //start the selection list
    handlestartmusic(entertainer);
}

//=======================================================================================================================
//
// places an instrument when selected and used in the inventory
//

void EntertainerManager::useInstrument(PlayerObject* entertainer, Item* usedInstrument)
{
    //we dont want to do this inmidst a performance
    if(entertainer->GetCreature()->getPerformingState() != PlayerPerformance_None)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_fail"), entertainer);
        return;

    }

    // we still need to check whether we have the appropriate skill
    if(!checkInstrumentSkillbyType(entertainer,usedInstrument->getItemType()))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_lack_skill_instrument"), entertainer);
        stopEntertaining(entertainer);
        return;
    }

    // is the instrument already placed
    if (usedInstrument->getPlaced())
    {
        //if yes, is it ours ?
        if(entertainer->getId() != usedInstrument->getOwner())
        {
            // Nope.
            gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_fail"), entertainer);
            return;
        }

        playInstrument(entertainer,usedInstrument);

        return;
    }

    // we have an other instrument placed already
    if(entertainer->getPlacedInstrumentId())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_must_unequip"), entertainer);
        return;
    }

    // place it

    // find out what instrument we are dealing with
    //we know at this point already that its an instrument, however we do not know whether we
    // are allowed to place it and whether we have the necessary skill

    usedInstrument->setPlaced(true);
    usedInstrument->setNonPersistantCopy(0);

    //mark the object we are placing right now its the permant one(!)
    entertainer->setPlacedInstrumentId(usedInstrument->getId());

    //TODO
    //check if we are somewhere where we may place our instrument ?
    gNonPersistantObjectFactory->createTangible(this,ItemFamily_Instrument,usedInstrument->getItemType(),entertainer->getId(),entertainer->mPosition,"",entertainer->getClient());


}



//=======================================================================================================================
//handles callbacks of db creation of items
//=======================================================================================================================

void EntertainerManager::handleObjectReady(Object* object,DispatchClient* client)
{
    if (Item* placedInstrument = dynamic_cast<Item*>(object))
    {
        // Due to ugly hack, the newly created instrumment have the entertainer as parent.
        // Parent should be Inventory, cell or in world, as long as it's not equipped.

        // This kind of hack does not make the code eaiser to handle and maintain.
        // Why not use the client, already provided to this method, to get the actual player handling this object?
        PlayerObject* player = gWorldManager->getPlayerByAccId(client->getAccountId());
        if (player)
        {
            //link the inventory object to the placed temporary object and vice versa
            Item* permanentinstrument = dynamic_cast<Item*>(gWorldManager->getObjectById(player->getPlacedInstrumentId()));

            if(!permanentinstrument)
            {
                return;
            }

            placedInstrument->setPersistantCopy(permanentinstrument->getId());
            permanentinstrument->setNonPersistantCopy(placedInstrument->getId());
            placedInstrument->setPlaced(true);
            placedInstrument->setStatic(false);

            //now set the nonpersistant Instrument in the playerObject
            player->setPlacedInstrumentId(placedInstrument->getId());

            //place it in the gameworld
            placedInstrument->setParentId(player->getParentId());
            placedInstrument->setOwner(player->getId());

            placedInstrument->mPosition  = player->mPosition;
            placedInstrument->mDirection = player->mDirection;

            //add it to MainObjectMap and SI it will be created for all players nearby
            gWorldManager->addObject(object);
        }
        else
        {
            // I wan't to know when this happens
            assert(false && "EntertainerManager::handleObjectReady WorldManager unable to find player");
        }
    }
    else
    {
        // I wan't to know when this happens
        assert(false && "EntertainerManager::handleObjectReady WorldManager unable to find instrument");
    }
}

//=======================================================================================================================
//handles the start of the individual performances on starting a band
//=======================================================================================================================
bool EntertainerManager::handleStartBandIndividual(PlayerObject* performer, BString performance)
{

    //we cant start performing when were about to log out!
    if(performer->getConnectionState() != PlayerConnState_Connected)
    {
        return false;
    }

    SkillCommandList*	entertainerSkillCommands = performer->getSkillCommands();
    SkillCommandList::iterator entertainerIt = entertainerSkillCommands->begin();

    int8 musicStr[32];
    sprintf(musicStr,"startmusic+%s",performance.getAnsi());

    //check if w
    bool found = false;
    if(!performance.getLength())
    {
        //nothing selected
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_fail"), performer);
        return false;
    }

    //check if we are able to perform this piece of music
    while(entertainerIt != entertainerSkillCommands->end())
    {
        BString mEntertainerString = gSkillManager->getSkillCmdById((*entertainerIt));
        //look for our selected dance
        if(BString(musicStr).getCrc() == mEntertainerString.getCrc() )
        {
            //yay we are able to perform this dance :)
            performer->GetCreature()->setPerformingState(PlayerPerformance_Music);
            gEntertainerManager->startMusicPerformance(performer,performance);
            return true;
        }
        entertainerIt++;
    }

    if (found == false)
    {
        //no we perform the piece of music
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_fail"), performer);
        return false;
    }

    performer->setAcceptBandFlourishes(true);

    return(true);
}

//=======================================================================================================================
//handles the start of the individual performances for dancers on starting a band
//=======================================================================================================================
bool EntertainerManager::handleStartBandDanceIndividual(PlayerObject* performer, BString performance)
{
    SkillCommandList*	entertainerSkillCommands = performer->getSkillCommands();
    SkillCommandList::iterator entertainerIt = entertainerSkillCommands->begin();

    int8 musicStr[32];
    sprintf(musicStr,"startdance+%s",performance.getAnsi());

    //check if we can perform the dance
    bool found = false;
    if(performance.getLength() >0 )
    {
        //check if we are able to perform this dance
        while(entertainerIt != entertainerSkillCommands->end())
        {
            BString mEntertainerString = gSkillManager->getSkillCmdById((*entertainerIt));
            //look for our selected dance
            if(BString(musicStr).getCrc() == mEntertainerString.getCrc() )
            {
                //yay we are able to perform this dance :)
                performer->GetCreature()->setPerformingState(PlayerPerformance_Dance);
                gEntertainerManager->startDancePerformance(performer,performance);
                found = true;
                return true;
            }
            entertainerIt++;
        }
    }

    if (found == false)
    {
        //no we cannot perform the dance
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "dance_fail"), performer);
        return false;
    }

    performer->setAcceptBandFlourishes(true);

    return true;
}

//======================================================================================================
//returns the Id of the targeted Instrument
//======================================================================================================
uint64 EntertainerManager::gettargetedInstrument(PlayerObject* entertainer)
{
    //check if we have something targeted

    Item* instrument = dynamic_cast<Item*>(gWorldManager->getObjectById(entertainer->GetCreature()->getTargetId()));
    if(!instrument)
        return 0;

    if(instrument->getItemFamily() == ItemFamily_Instrument)
    {
        return instrument->getId();
    }
    return 0;

}

bool EntertainerManager::checkInstrumentSkill(PlayerObject* entertainer,uint64 instrumentId)
{
    Item* instrument = dynamic_cast<Item*> (gWorldManager->getObjectById(instrumentId));
    return(checkInstrumentSkillbyType(entertainer, instrument->getItemType()));
}

//======================================================================================================
//checks whether we are able to play the given Instrument
//======================================================================================================
bool EntertainerManager::checkInstrumentSkillbyType(PlayerObject* entertainer,uint32 instrumentType)
{
    //bool check = true;

    switch(instrumentType)
    {

    case ItemType_bandfill: //chidinkalu horn
    {
        return(entertainer->GetCreature()->checkSkill(SMSkill_Musician_Knowledge_2));
    }
    break;

    case ItemType_flute_droopy: //chidinkalu horn
    {
        return(entertainer->GetCreature()->checkSkill(SMSkill_Musician_Knowledge_3));
    }
    break;

    case ItemType_omni_box:
    {
        return(entertainer->GetCreature()->checkSkill(SMSkill_Musician_Knowledge_4));
    }
    break;

    case ItemType_nalargon_max_reebo:
    {
        return(entertainer->GetCreature()->checkSkill(SMSkill_MasterMusician));
    }
    break;

    case ItemType_Nalargon:
    {
        return(entertainer->GetCreature()->checkSkill(SMSkill_MasterMusician));
    }
    break;

    case ItemType_Slitherhorn:
    {
        return(entertainer->GetCreature()->checkSkill(SMSkill_NoviceEntertainer));
    }
    break;

    case ItemType_fizzz:
    {
        return(entertainer->GetCreature()->checkSkill(SMSkill_EntertainerMusic1));
    }
    break;

    case ItemType_fanfar:
    {
        return(entertainer->GetCreature()->checkSkill(SMSkill_EntertainerMusic3));
    }
    break;

    case ItemType_Kloo_Horn:
    {
        return(entertainer->GetCreature()->checkSkill(SMSkill_EntertainerMusic4));
    }
    break;

    case ItemType_mandoviol:
    {
        return(entertainer->GetCreature()->checkSkill(SMSkill_MasterEntertainer));
    }
    break;

    case ItemType_traz:
    {
        return(entertainer->GetCreature()->checkSkill(SMSkill_MusicianNovice));
    }
    break;


    default:
        return(true);
    }



    return true;

}

//======================================================================================================
//reports the id of our equipped instrument
//======================================================================================================
uint64 EntertainerManager::getInstrument(PlayerObject* entertainer)
{
    //bool found = false;

    // handles the instrumntselection

    //need to get the equipped or targeted instrument
    uint64	instrumentId = 0;

	auto equip_service	= gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto item			= dynamic_cast<Item*>(equip_service->GetEquippedObject(entertainer, "hold_r"));

    if(item && item->getItemFamily() == ItemFamily_Instrument)
    {
        instrumentId = item->getId();
    }

    // alternatively if no instrument equipped - do we have an instrument targeted?

    if (instrumentId == 0)
    {
        instrumentId = gettargetedInstrument(entertainer);

        // make sure the instrument is placed properly.
        if (instrumentId != 0)
        {
            Instrument* instrument = dynamic_cast<Instrument*>(gWorldManager->getObjectById(instrumentId));
            if (instrument)
            {
                // We have a instrument targeted, but it can be ANY instrument I have or anyone elses laying at the floor.
                uint32 instrumentType = instrument->getItemType();
                if ((instrumentType == ItemType_Nalargon) || (instrumentType == ItemType_omni_box) || (instrumentType == ItemType_nalargon_max_reebo))
                {
                    // Now we know it's placeble instrument, or it's original.
                    // Is it my placed instrument?
                    if (entertainer->getPlacedInstrumentId() != instrument->getId())
                    {
                        // I just allowed the original item to have a owner tag, just to simulate the missing cell-building access system.

                        if (instrument->getOwner() != entertainer->getId())
                        {
                            // It's NOT an item, owned by me.
                            // For debug
                            return 0;
                        }
                        // Whats left?
                        // It's a permanet placed instrument, owned by me.

                        // We do have to make a building access rights functionality.
                    }
                }
            }
        }
    }
    return instrumentId;
}

//======================================================================================================
//
//	Moves the player to the selected instrument, if in range. Returns true if moved succeeded (instrument was in range).
//
//======================================================================================================
bool EntertainerManager::approachInstrument(PlayerObject* entertainer, uint64 instrumentId)
{
    bool moveSucceeded = false;
	
	auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto item	= dynamic_cast<Item*>(equip_service->GetEquippedObject(entertainer, "hold_r"));

    // make sure the instrument is placed properly.
    if (instrumentId != 0)
    {
        Instrument* instrument = dynamic_cast<Instrument*>(gWorldManager->getObjectById(instrumentId));
        if (instrument)
        {
            // We have a selected instrument.
            uint32 instrumentType = instrument->getItemType();
            if ((instrumentType == ItemType_Nalargon) || (instrumentType == ItemType_omni_box) || (instrumentType == ItemType_nalargon_max_reebo))
            {
                // Now we know it's placeble instrument, or it's original.

                if (gWorldManager->objectsInRange(entertainer->getId(), instrumentId, 6.0))
                {
                    // We are in range.
                    moveSucceeded = true;

                    entertainer->mPosition = instrument->mPosition;
                    entertainer->mDirection = instrument->mDirection;

                    entertainer->GetCreature()->updatePosition(instrument->getParentId(),instrument->mPosition);

                }
            }
            else
            {
                // We approach the instrument to the entertainer, if we have it equipped
                if (item && item->getItemFamily() == ItemFamily_Instrument)
                {
                    if (item->getId() == instrumentId)
                    {
                        moveSucceeded = true;
                        instrument->mPosition  = entertainer->mPosition;
                        instrument->mDirection = entertainer->mDirection;

                        gMessageLib->sendDataTransform053(instrument);
                    }
                }
            }
        }
    }
    return moveSucceeded;
}

//======================================================================================================
//
// handles startmusic when selected from a placed instruments radial
//

void EntertainerManager::handlestartmusic(PlayerObject* entertainer)
{

    if(entertainer->GetCreature()->getPerformingState() != PlayerPerformance_None)    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "already_performing_self"), entertainer);
        return;
    }

	auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto inventory	= dynamic_cast<Inventory*>(equip_service->GetEquippedObject(entertainer->GetCreature(), "inventory"));
	auto item		= dynamic_cast<Item*>(equip_service->GetEquippedObject(entertainer->GetCreature(), "hold_r"));

    //do we have an instrument?
    uint64 instrumentId = getInstrument(entertainer);
    if(instrumentId == 0)    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_no_instrument"), entertainer);
        return;
    }

    // check if the weapon slot is in use by something else than the unarmed default weapon
	if(item && item->getId() != entertainer->getId() + WEAPON_OFFSET)    {
        // TODO: put another message ?
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_must_unequip"), entertainer);
        return;
    }    

    //check if we are skilled enough to use the instrument
    if(!checkInstrumentSkill(entertainer,instrumentId))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_lack_skill_instrument"), entertainer);
        return;
    }

    //show list box with all available songs
    SkillCommandList*	entertainerSkillCommands = entertainer->getSkillCommands();
    SkillCommandList::iterator entertainerIt = entertainerSkillCommands->begin();


    StringVector availableCommands;
    uint32 nr = 0;
    entertainerIt = entertainerSkillCommands->begin();
    while(entertainerIt != entertainerSkillCommands->end())
    {
        //look for all the songs
        if(strstr(gSkillManager->getSkillCmdById((*entertainerIt)).getAnsi(),"startmusic+"))
        {
            // add it to our dancelist
            int8 str[64];
            sprintf(str,"@%s:%s","cmd_n",gSkillManager->getSkillCmdById(*entertainerIt).getAnsi());
            availableCommands.push_back(str);
            nr++;
        }

        entertainerIt++;
    }

    if(nr > 0)
    {
        gUIManager->createNewListBox(entertainer,"handleselectmusic","select music","@performance:music_no_music_param",availableCommands,entertainer,SUI_Window_SelectMusic_Listbox,SUI_LB_OK);
    }
    else
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_fail"), entertainer);
    }
}

//=============================================================================
//
// manages flourishes
//

void EntertainerManager::flourish(PlayerObject* entertainer, uint32 mFlourishId)
{
    //check ham
    int32 pActionPoints;
    
	auto ham = kernel_->GetServiceManager()->GetService<swganh::ham::HamService>("HamService");

    if(entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Dance)    {
        pActionPoints = (((PerformanceStruct*)entertainer->GetCreature()->getPerformance())->actionPointPerLoop)*2;
    }
    else
        pActionPoints = (((PerformanceStruct*)entertainer->GetCreature()->getPerformance())->actionPointPerLoop)/2;


    if(!ham->checkMainPools(entertainer->GetCreature(), 0, pActionPoints, 0))
    {
        //not enough action -> no flourish
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "flourish_too_tired"), entertainer);
        return;
    }

    //make sure we cant spam flourishes for fast xp
    entertainer->setFlourishCount(entertainer->getFlourishCount()+1);
    if (entertainer->getFlourishCount()>5)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "flourish_wait_self"), entertainer);
        return;
    }

    //being here means that we are performing and have enough ham

    //update ham
    if (entertainer->getFlourishCount()<2)
    {
		ham->ApplyHamCost(entertainer->GetCreature(), HamBar_Action, -pActionPoints);
    }

    if(entertainer->GetCreature()->getPerformingState() == PlayerPerformance_Dance)
    {
        //send NPCAnimate
        int8  mAnimation[32];
        sprintf(mAnimation,"skill_action_%u",mFlourishId);

        gMessageLib->sendCreatureAnimation(entertainer->GetCreature(), BString(mAnimation));
    }
    else
    {
        gMessageLib->sendperformFlourish(entertainer, mFlourishId);
        //int8  mAnimation[32];
        //sprintf(mAnimation,"skill_action_%u",mFlourishId);
        //gMessageLib->sendCreatureAnimation(entertainer, BString(mAnimation));
    }

}


//=============================================================================
//
// checks for npcs in reach to entertain them
//

void EntertainerManager::entertainInRangeNPCs(PlayerObject* entertainer) {
    ObjectSet in_range_objects;

    gSpatialIndexManager->getObjectsInRange(entertainer, &in_range_objects, ObjType_Creature, 30.0, true);

    std::for_each(in_range_objects.begin(), in_range_objects.end(), [=] (Object* object) {
        if (object->getType() != ObjType_NPC) {
            return;
        }

        CreatureObject* npc = dynamic_cast<CreatureObject*>(object);
        if (!npc) {
            return;
        }

        //does our npc get entertained yet?
        if (npc->getEntertainerListenToId() == 0) {
            //no
            npc->setEntertainerListenToId(entertainer->getId());
            addAudience(entertainer,npc);
            npc->setCurrentAnimation("entertained");
            gMessageLib->sendAnimationString(npc);
        }
    });
}


// NOTE: THIS CODE AS ADDED BECAUSE DID NOT WANT TO BREAK CODE FOR OTHER INSTRUMENTS (by Eruptor)
//=======================================================================================================================
//
// Uses a Nalargon.
//

void EntertainerManager::usePlacedInstrument(PlayerObject* entertainer, Item* usedInstrument)
{

    if (entertainer->GetCreature()->getPerformingState() != PlayerPerformance_None)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "already_performing_self"), entertainer);
        return;
    }

    // If we don't have the skills for playing the instrument,
    // we don't have to bother if it's placed or in use or whatever....
    if (!checkInstrumentSkillbyType(entertainer,usedInstrument->getItemType()))    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_lack_skill_instrument"), entertainer);
        stopEntertaining(entertainer);
        return;
    }

    if ((usedInstrument->getItemType() == ItemType_Nalargon) || (usedInstrument->getItemType() == ItemType_nalargon_max_reebo) || (usedInstrument->getItemType() == ItemType_omni_box))
    {
        // check if another instrument is equipped
        playPlacedInstrument(entertainer);
    }
}

// NOTE: THIS CODE AS ADDED BECAUSE DID NOT WANT TO BREAK CODE FOR OTHER INSTRUMENTS (by Eruptor)
//======================================================================================================
//
// handles startmusic with the Nalargon when selected from a placed instruments radial
//

void EntertainerManager::playPlacedInstrument(PlayerObject* entertainer)
{

    if (entertainer->GetCreature()->getPerformingState() != PlayerPerformance_None)    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "already_performing_self"), entertainer);
        return;
    }

    //check if we are skilled enough to use the instrument
    /*
    if (!checkInstrumentSkill(entertainer,instrumentId))
    {
    	gMessageLib->sendSystemMessage(entertainer,L"","performance","music_lack_skill_instrument");
    	return;
    }
    */

    // check if the weapon slot is in use by something else than the unarmed default weapon
	auto equip_service	= gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto object			= dynamic_cast<Object*>(equip_service->GetEquippedObject(entertainer, "hold_r"));

    if(object && object->getId() != entertainer->getId()+WEAPON_OFFSET)        {
        // TODO: put another message ?
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_must_unequip"), entertainer);
        return;
    }

    //show list box with all available songs
    SkillCommandList*	entertainerSkillCommands = entertainer->getSkillCommands();
    SkillCommandList::iterator entertainerIt = entertainerSkillCommands->begin();


    StringVector availableCommands;
    uint32 nr = 0;
    entertainerIt = entertainerSkillCommands->begin();
    while(entertainerIt != entertainerSkillCommands->end())
    {
        //look for all the songs
        if(strstr(gSkillManager->getSkillCmdById((*entertainerIt)).getAnsi(),"startmusic+"))
        {
            // add it to our dancelist
            int8 str[64];
            sprintf(str,"@%s:%s","cmd_n",gSkillManager->getSkillCmdById(*entertainerIt).getAnsi());
            availableCommands.push_back(str);
            nr++;
        }

        entertainerIt++;
    }

    if(nr > 0)
    {
        gUIManager->createNewListBox(entertainer,"handleselectmusic","select music","@performance:music_no_music_param",availableCommands,entertainer,SUI_Window_SelectMusic_Listbox,SUI_LB_OK);
    }
    else
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_fail"), entertainer);
    }
}

void EntertainerManager::StartBand(PlayerObject* player, BString songName)
{
    gGroupManager->getGroupLeader(player, player->GetCreature()->getGroupId(), GROUPMANAGERCALLBACK_STARTBAND, this, songName);
}

void EntertainerManager::StopBand(PlayerObject* player)
{
    gGroupManager->getGroupLeader(player, player->GetCreature()->getGroupId(), GROUPMANAGERCALLBACK_STOPBAND, this);
}

void EntertainerManager::BandFlourish(PlayerObject* player, uint32 flourishId)
{
    gGroupManager->getGroupLeader(player, player->GetCreature()->getGroupId(), GROUPMANAGERCALLBACK_BANDFLOURISH, this, flourishId);
}

void EntertainerManager::_handleCompleteStartBand(PlayerObject* performer, BString dataStr)
{

    auto members = gGroupManager->getInRangeGroupMembers(performer->GetCreature(), true);
    bool music = true;

    //check if this is a valid song we - as bandleader - can perform.
    //then pass it to the band
    //otherwise open listbox to check

    SkillCommandList*	entertainerSkillCommands = performer->getSkillCommands();
    SkillCommandList::iterator entertainerIt = entertainerSkillCommands->begin();

    int8 musicStr[32];
    sprintf(musicStr,"startmusic+%s",dataStr.getAnsi());

    //check if we can perform the song
    bool found = false;
    if(dataStr.getLength() >0 )
    {
        //check if we are able to perform this piece of music
        while(entertainerIt != entertainerSkillCommands->end())
        {
            BString mEntertainerString = gSkillManager->getSkillCmdById((*entertainerIt));
            //look for our selected dance
            if(BString(musicStr).getCrc() == mEntertainerString.getCrc() )
            {
                //yay we are able to perform this dance :)
                found = true;
            }
            entertainerIt++;
        }
    }

    if (found == false)
    {
        //gMessageLib->sendSystemMessage(performer,L"","performance","music_invalid_song");
        //however we might be able to squeeze the dancers in
        music = false;
        SkillCommandList::iterator entertainerIt = entertainerSkillCommands->begin();
        sprintf(musicStr,"startdance+%s",dataStr.getAnsi());

        //check if we can perform the dance
        found = false;
        if(dataStr.getLength() >0 )
        {
            //check if we are able to perform this piece of dance
            while(entertainerIt != entertainerSkillCommands->end())
            {
                BString mEntertainerString = gSkillManager->getSkillCmdById((*entertainerIt));
                //look for our selected dance
                if(BString(musicStr).getCrc() == mEntertainerString.getCrc() )
                {
                    //yay we are able to perform this dance :)
                    found = true;
                }
                entertainerIt++;
            }
        }


    }

    if (found == false)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_invalid_song"), performer);
        return;
    }

    //iterate through every groupmember and invoke starband on the EntertainerManager who will check if the song
    //is playable by the bandmember

    //check if anybody is still playing
    bool playCheck = true;

    auto memberIt = members.begin();
    while(memberIt != members.end())    {
		CreatureObject* creature = dynamic_cast<CreatureObject*>(*memberIt);
        //check if we are performing
        if(creature->getPerformingState() == PlayerPerformance_Music)
        {
            playCheck = false;
        }
        memberIt++;
    }

    if(!playCheck)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_still_playing"), performer);
        return;
    }

    bool skillCheck = true;
    memberIt = members.begin();
    while(memberIt != members.end())    {
		
		CreatureObject* creature = dynamic_cast<CreatureObject*>(*memberIt);

		auto equip_service	= gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
		auto ghost			= dynamic_cast<PlayerObject*>(equip_service->GetEquippedObject(creature, "ghost"));

        //check if we are performing
        if((creature->getPerformingState() == PlayerPerformance_None)&&(ghost->getConnectionState() == PlayerConnState_Connected))
        {
            if(creature->checkSkill(SMSkill_NoviceEntertainer))//Novice Entertainer
            {
                if(music)
                {
                    if(!handleStartBandIndividual(ghost, dataStr))
                        skillCheck = false;
                }
                else
                {
                    if(!handleStartBandDanceIndividual(ghost, dataStr))
                        skillCheck = false;
                }
            }
        }

        memberIt++;
    }

    if (!skillCheck)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_lack_skill_band_member"), performer);
    }
}



void EntertainerManager::_handleCompleteStopBand(PlayerObject* performer)
{
    bool music = false;

    
    auto members = gGroupManager->getInRangeGroupMembers(performer->GetCreature(), true);
    auto memberIt = members.begin();
    while(memberIt != members.end())    {
		
		CreatureObject* creature = dynamic_cast<CreatureObject*>(*memberIt);
        
		auto equip_service	= gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
		auto ghost		= dynamic_cast<PlayerObject*>(equip_service->GetEquippedObject(creature, "ghost"));

		//check if we are performing
        if(creature->getPerformingState() != PlayerPerformance_None)
        {
            music = true;
            gEntertainerManager->stopEntertaining(ghost);

            if(ghost != performer)
                gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_stop_band_members", performer->getId(), 0, 0), ghost);

        }
        memberIt++;
    }
    if(music)
        gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "music_stop_band_self"), performer);
}

void EntertainerManager::_handleCompleteBandFlourish(PlayerObject* entertainer, uint32 FlourishId)
{
    //give notice
    gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "flourish_perform_band_self"), entertainer);

    ObjectList members;
    members = gGroupManager->getInRangeGroupMembers(entertainer->GetCreature(), true);
    auto memberIt = members.begin();
    while(memberIt != members.end())    {
		CreatureObject* creature = dynamic_cast<CreatureObject*>(*memberIt);
		
		auto equip_service	= gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
		auto ghost		= dynamic_cast<PlayerObject*>(equip_service->GetEquippedObject(creature, "ghost"));
        
		//check if we are performing
        if(creature->getPerformingState() != PlayerPerformance_None)
        {
            //give notice
            gMessageLib->SendSystemMessage(::common::OutOfBand("performance", "flourish_perform_band_member", 0, entertainer->getId(), 0), entertainer);

            if(ghost->getAcceptBandFlourishes())
                gEntertainerManager->flourish(ghost,FlourishId);

        }
        memberIt++;
    }
}
