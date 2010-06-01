/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "CharSheetManager.h"

#include "Badge.h"
#include "Bank.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "Inventory.h"
#include "ZoneOpcodes.h"

#include "LogManager/LogManager.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"


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
	_loadCommandMap();

	mDatabase->ExecuteSqlAsync(this,new(mDBAsyncPool.malloc()) CSAsyncContainer(CharSheetQuery_Factions),"SELECT * FROM faction ORDER BY id");
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
	mMessageDispatch->RegisterMessageCallback(opFactionRequestMessage,this);
	mMessageDispatch->RegisterMessageCallback(opPlayerMoneyRequest,this);
	mMessageDispatch->RegisterMessageCallback(opStomachRequestMessage,this);
	mMessageDispatch->RegisterMessageCallback(opGuildRequestMessage,this);
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

void CharSheetManager::_loadCommandMap()
{
	mCommandMap.insert(std::make_pair(opFactionRequestMessage,&CharSheetManager::_processFactionRequest));
	mCommandMap.insert(std::make_pair(opPlayerMoneyRequest,&CharSheetManager::_processPlayerMoneyRequest));
	mCommandMap.insert(std::make_pair(opStomachRequestMessage,&CharSheetManager::_processStomachRequest));
	mCommandMap.insert(std::make_pair(opGuildRequestMessage,&CharSheetManager::_processGuildRequest));
}

//=========================================================================================

CharSheetManager::~CharSheetManager()
{
	_unregisterCallbacks();
	mCommandMap.clear();

	mInsFlag = false;
	delete(mSingleton);
}

//======================================================================================================================

void CharSheetManager::handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client)
{
	CSCommandMap::iterator it = mCommandMap.find(opcode);

	if(it != mCommandMap.end())
		(this->*((*it).second))(message,client);
	else
		gLogger->log(LogManager::DEBUG,"CharSheetManager: Unhandled DispatchMsg %u",opcode);
}

//=========================================================================================

void CharSheetManager::handleDatabaseJobComplete(void* ref, DatabaseResult* result)
{
	CSAsyncContainer* asyncContainer = reinterpret_cast<CSAsyncContainer*>(ref);

	switch(asyncContainer->mQuery)
	{
		case CharSheetQuery_Factions:
		{

			string name;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,0,255,1);

			uint64 count = result->getRowCount();
			mvFactions.reserve((uint32)count);
			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&name);
				mvFactions.push_back(BString(name.getAnsi()));
			}

			if(result->getRowCount())
				gLogger->log(LogManager::NOTICE,"Loaded factions.");

			mDatabase->DestroyDataBinding(binding);

			// load badge categories
			gLogger->log(LogManager::NOTICE,"Loading Badge Categories.");
			mDatabase->ExecuteSqlAsync(this,new(mDBAsyncPool.malloc()) CSAsyncContainer(CharSheetQuery_BadgeCategories),"SELECT * FROM badge_categories ORDER BY id");
		}
		break;

		case CharSheetQuery_BadgeCategories:
		{
			string name;
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_bstring,0,255,1);

			uint64 count = result->getRowCount();
			mvBadgeCategories.reserve((uint32)count);
			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&name);
				mvBadgeCategories.push_back(BString(name.getAnsi()));
			}

			mDatabase->DestroyDataBinding(binding);

			gLogger->log(LogManager::NOTICE,"Loading Badges.");
			mDatabase->ExecuteSqlAsync(this,new(mDBAsyncPool.malloc()) CSAsyncContainer(CharSheetQuery_Badges),"SELECT * FROM badges ORDER BY id");
		}
		break;

		case CharSheetQuery_Badges:
		{
			Badge* badge;

			DataBinding* binding = mDatabase->CreateDataBinding(4);
			binding->addField(DFT_uint32,offsetof(Badge,mId),4,0);
			binding->addField(DFT_bstring,offsetof(Badge,mName),255,1);
			binding->addField(DFT_uint32,offsetof(Badge,mSoundId),4,2);
			binding->addField(DFT_uint8,offsetof(Badge,mCategory),1,3);

			uint64 count = result->getRowCount();
			mvBadges.reserve((uint32)count);
			for(uint64 i = 0;i < count;i++)
			{
				badge = new Badge();
				result->GetNextRow(binding,badge);
				mvBadges.push_back(badge);
			}

			mDatabase->DestroyDataBinding(binding);

			gLogger->log(LogManager::INFORMATION,"Loading weapon groups...");				
		}
		break;

		default:break;
	}

	mDBAsyncPool.free(asyncContainer);
}

//=========================================================================================

void CharSheetManager::_processFactionRequest(Message* message,DispatchClient* client)
{
	PlayerObject* player = gWorldManager->getPlayerByAccId(client->getAccountId());

	if(player == NULL)
	{
		gLogger->log(LogManager::DEBUG,"CharSheetManager::_processFactionRequest: could not find player %u",client->getAccountId());
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
		gLogger->log(LogManager::DEBUG,"CharSheetManager::_processPlayerMoneyRequest: could not find player %u",client->getAccountId());
		return;
	}

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opPlayerMoneyResponse);
	gMessageFactory->addUint32(dynamic_cast<Bank*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->getCredits());
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

