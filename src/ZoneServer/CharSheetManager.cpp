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
#include "CharSheetManager.h"


#include "Utils/logger.h"

#include "Badge.h"
#include "Bank.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "Inventory.h"
#include "ZoneOpcodes.h"



#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"


//=========================================================================================

bool				CharSheetManager::mInsFlag = false;
CharSheetManager*	CharSheetManager::mSingleton = NULL;

//=========================================================================================

CharSheetManager::CharSheetManager(Database* database,MessageDispatch* dispatch) :
    mDatabase(database),
    mMessageDispatch(dispatch),
    mDBAsyncPool(sizeof(CSAsyncContainer))
{
    _registerCallbacks();

    //gLogger->log(LogManager::DEBUG,"Started Loading Factions.");
    mDatabase->executeSqlAsync(this, new(mDBAsyncPool.malloc()) CSAsyncContainer(CharSheetQuery_Factions), "SELECT * FROM %s.faction ORDER BY id;",mDatabase->galaxy());
    
}

//=========================================================================================

CharSheetManager* CharSheetManager::Init(Database* database,MessageDispatch* dispatch)
{
    if(mInsFlag == false)
    {
        mSingleton = new CharSheetManager(database,dispatch);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=========================================================================================

void CharSheetManager::_registerCallbacks()
{
    mMessageDispatch->RegisterMessageCallback(opFactionRequestMessage,std::bind(&CharSheetManager::_processFactionRequest, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opPlayerMoneyRequest,std::bind(&CharSheetManager::_processPlayerMoneyRequest, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opStomachRequestMessage,std::bind(&CharSheetManager::_processStomachRequest, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opGuildRequestMessage,std::bind(&CharSheetManager::_processGuildRequest, this, std::placeholders::_1, std::placeholders::_2));
}

//=========================================================================================

void CharSheetManager::_unregisterCallbacks()
{
    mMessageDispatch->UnregisterMessageCallback(opFactionRequestMessage);
    mMessageDispatch->UnregisterMessageCallback(opPlayerMoneyRequest);
    mMessageDispatch->UnregisterMessageCallback(opStomachRequestMessage);
    mMessageDispatch->UnregisterMessageCallback(opGuildRequestMessage);
}

//=========================================================================================

CharSheetManager::~CharSheetManager()
{
    _unregisterCallbacks();

    mInsFlag = false;
    delete(mSingleton);
}

//=========================================================================================

void CharSheetManager::handleDatabaseJobComplete(void* ref, DatabaseResult* result)
{
    CSAsyncContainer* asyncContainer = reinterpret_cast<CSAsyncContainer*>(ref);

    switch(asyncContainer->mQuery)
    {
    case CharSheetQuery_Factions:
    {

        BString name;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_bstring,0,255,1);

        uint64 count = result->getRowCount();
        mvFactions.reserve((uint32)count);
        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&name);
            mvFactions.push_back(BString(name.getAnsi()));
        }

		if (count)
		{
			LOG(INFO) << "Loaded " << count << " factions";
		}

        mDatabase->destroyDataBinding(binding);

        // load badge categories
        mDatabase->executeSqlAsync(this,new(mDBAsyncPool.malloc()) CSAsyncContainer(CharSheetQuery_BadgeCategories),"SELECT * FROM %s.badge_categories ORDER BY id",mDatabase->galaxy());
        
    }
    break;

    case CharSheetQuery_BadgeCategories:
    {
        BString name;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_bstring,0,255,1);

        uint64 count = result->getRowCount();
        mvBadgeCategories.reserve((uint32)count);
        for(uint64 i = 0; i < count; i++)
        {
            result->getNextRow(binding,&name);
            mvBadgeCategories.push_back(BString(name.getAnsi()));
        }

		if (count)
		{
			LOG(INFO) << "Loaded " << count << " badge categories";
		}

        mDatabase->destroyDataBinding(binding);

        //gLogger->log(LogManager::DEBUG,"Finished Loading Badge Categories.");
        //gLogger->log(LogManager::NOTICE,"Loading Badges.");
        mDatabase->executeSqlAsync(this,new(mDBAsyncPool.malloc()) CSAsyncContainer(CharSheetQuery_Badges),"SELECT * FROM %s.badges ORDER BY id",mDatabase->galaxy());
        
    }
    break;

    case CharSheetQuery_Badges:
    {
        Badge* badge;

        DataBinding* binding = mDatabase->createDataBinding(4);
        binding->addField(DFT_uint32,offsetof(Badge,mId),4,0);
        binding->addField(DFT_bstring,offsetof(Badge,mName),255,1);
        binding->addField(DFT_uint32,offsetof(Badge,mSoundId),4,2);
        binding->addField(DFT_uint8,offsetof(Badge,mCategory),1,3);

        uint64 count = result->getRowCount();
        mvBadges.reserve((uint32)count);
        for(uint64 i = 0; i < count; i++)
        {
            badge = new Badge();
            result->getNextRow(binding,badge);
            mvBadges.push_back(badge);
        }

		if (count)
		{
			LOG(INFO) << "Loaded " << count << " badges";
		}

        mDatabase->destroyDataBinding(binding);
        //gLogger->log(LogManager::DEBUG,"Finished Loading Badges.");
    }
    break;

    default:
        break;
    }

    mDBAsyncPool.free(asyncContainer);
}

//=========================================================================================

void CharSheetManager::_processFactionRequest(Message* message,DispatchClient* client)
{
    PlayerObject* player = gWorldManager->getPlayerByAccId(client->getAccountId());

    if(player == NULL)
    {
        DLOG(INFO) << "CharSheetManager::_processFactionRequest: could not find player " << client->getAccountId();
        return;
    }

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opFactionResponseMessage);
    gMessageFactory->addString(player->getFaction());
    gMessageFactory->addUint32(player->getFactionPointsByFactionId(2));
    gMessageFactory->addUint32(player->getFactionPointsByFactionId(3));
    gMessageFactory->addUint32(0);

    FactionList* factions = player->getFactionList();

    gMessageFactory->addUint32(factions->size());

    FactionList::iterator it = factions->begin();

    while(it != factions->end())
    {
        gMessageFactory->addString(mvFactions[(*it).first - 1]);
        ++it;
    }

    gMessageFactory->addUint32(factions->size());

    it = factions->begin();

    while(it != factions->end())
    {
        gMessageFactory->addFloat((float)((*it).second));
        ++it;
    }

    Message* newMessage = gMessageFactory->EndMessage();

    client->SendChannelA(newMessage,client->getAccountId(),CR_Client,6);
}

//=========================================================================================

void CharSheetManager::_processPlayerMoneyRequest(Message* message,DispatchClient* client)
{
    PlayerObject* player = gWorldManager->getPlayerByAccId(client->getAccountId());

    if(player == NULL)
    {
        DLOG(INFO) << "CharSheetManager::_processPlayerMoneyRequest: could not find player " << client->getAccountId();
        return;
    }

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opPlayerMoneyResponse);
    gMessageFactory->addUint32(dynamic_cast<Bank*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->credits());
    gMessageFactory->addUint32(dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits());

    Message* newMessage = gMessageFactory->EndMessage();

    client->SendChannelA(newMessage,client->getAccountId(),CR_Client,3);
}

//=========================================================================================

void CharSheetManager::_processStomachRequest(Message* message,DispatchClient* client)
{

}

//=========================================================================================

void CharSheetManager::_processGuildRequest(Message* message,DispatchClient* client)
{

}

//=========================================================================================

