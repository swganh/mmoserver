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

#include "TradeManager.h"


#include "Utils/logger.h"

#include "Bank.h"
#include "CharacterBuilderTerminal.h"
#include "Datapad.h"
#include "Inventory.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "TreasuryManager.h"
#include "WaypointObject.h"
#include "Wearable.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "ContainerManager.h"
#include "ZoneOpcodes.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/Transaction.h"
#include "DatabaseManager/DataBinding.h"
#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"
#include "Utils/utils.h"

bool				TradeManager::mInsFlag    = false;
TradeManager*		TradeManager::mSingleton  = NULL;


//======================================================================================================================

TradeManager::TradeManager(Database* database, MessageDispatch* dispatch)
: mDatabase(database)
, mMessageDispatch(dispatch)
{
    mMessageDispatch->RegisterMessageCallback(opCreateAuctionMessage,std::bind(&TradeManager::_processHandleAuctionCreateMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opCreateImmediateAuctionMessage,std::bind(&TradeManager::_processHandleImmediateAuctionCreateMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opProcessSendCreateItem,std::bind(&TradeManager::_processCreateItemMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opAbortTradeMessage,std::bind(&TradeManager::_processAbortTradeMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opTradeCompleteMessage,std::bind(&TradeManager::_processTradeCompleteMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opAddItemMessage,std::bind(&TradeManager::_processAddItemMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opRemoveItemMessage,std::bind(&TradeManager::_processRemoveItemMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opAcceptTransactionMessage,std::bind(&TradeManager::_processAcceptTransactionMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opBeginVerificationMessage,std::bind(&TradeManager::_processBeginVerificationMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opVerifyTradeMessage,std::bind(&TradeManager::_processVerificationMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opUnacceptTransactionMessage,std::bind(&TradeManager::_processUnacceptTransactionMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opGiveMoneyMessage,std::bind(&TradeManager::_processGiveMoneyMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opDeductMoneyMessage,std::bind(&TradeManager::_processDeductMoneyMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opFindFriendRequestPosition,std::bind(&TradeManager::_processFindFriendRequestPositionMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opFindFriendCreateWaypoint,std::bind(&TradeManager::_processFindFriendCreateWaypointMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opBankTipDeduct,std::bind(&TradeManager::_processBanktipUpdate, this, std::placeholders::_1, std::placeholders::_2));

    mErrorCount = 1;
    mZoneId = gWorldManager->getZoneId();

    // load our bazaar terminals
    TradeManagerAsyncContainer* load_bazaar_container = new TradeManagerAsyncContainer(TRMQuery_LoadBazaar, 0);
    mDatabase->executeSqlAsync(this, load_bazaar_container, "SELECT * FROM %s.commerce_bazaar",mDatabase->galaxy());
   
    //AuctionHandler = new AuctionClass;


    //load the item table for the character builder terminal
    TradeManagerAsyncContainer* frog_query_container = new TradeManagerAsyncContainer(TRMQuery_ItemTableFrogQuery, 0);
    mDatabase->executeSqlAsync(this, frog_query_container, "SELECT * FROM %s.frog_items fi INNER JOIN %s.item_families i_f on fi.family = i_f.id",mDatabase->galaxy(),mDatabase->galaxy());
}


//======================================================================================================================
TradeManager::~TradeManager()
{
    mInsFlag = false;
    delete(mSingleton);
}
//======================================================================================================================
TradeManager*	TradeManager::Init(Database* database, MessageDispatch* dispatch)
{
    if(!mInsFlag)
    {
        mSingleton = new TradeManager(database,dispatch);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;

}

//======================================================================================================================
void TradeManager::Shutdown()
{
    mMessageDispatch->UnregisterMessageCallback(opCreateAuctionMessage);
    mMessageDispatch->UnregisterMessageCallback(opCreateImmediateAuctionMessage);
    mMessageDispatch->UnregisterMessageCallback(opProcessSendCreateItem);
    mMessageDispatch->UnregisterMessageCallback(opAbortTradeMessage);
    mMessageDispatch->UnregisterMessageCallback(opTradeCompleteMessage);
    mMessageDispatch->UnregisterMessageCallback(opAddItemMessage);
    mMessageDispatch->UnregisterMessageCallback(opRemoveItemMessage);
    mMessageDispatch->UnregisterMessageCallback(opAcceptTransactionMessage);
    mMessageDispatch->UnregisterMessageCallback(opBeginVerificationMessage);
    mMessageDispatch->UnregisterMessageCallback(opVerifyTradeMessage);
    mMessageDispatch->UnregisterMessageCallback(opUnacceptTransactionMessage);
    mMessageDispatch->UnregisterMessageCallback(opGiveMoneyMessage);
    mMessageDispatch->UnregisterMessageCallback(opDeductMoneyMessage);
    mMessageDispatch->UnregisterMessageCallback(opFindFriendRequestPosition);
    mMessageDispatch->UnregisterMessageCallback(opFindFriendCreateWaypoint);

    mMessageDispatch->UnregisterMessageCallback(opBankTipDeduct);

}

//=======================================================================================================================
//handles all the db callbacks
//=======================================================================================================================
void TradeManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    TradeManagerAsyncContainer* asynContainer = (TradeManagerAsyncContainer*)ref;


    switch(asynContainer->mQueryType)
    {

    case TRMQuery_MoneyTransaction:
    {
        uint32 error;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);
        result->getNextRow(binding,&error);


        if((!asynContainer->player1)||(!asynContainer->player2))
        {
            DLOG(INFO) << "TradeManager TRMQuery_MoneyTransaction : one of the tradepartners doesnt exist";
            error = 1;
        }

        if (error )
        {
            DLOG(INFO) << "TradeManager Trade transaction failed";
            // oh woe we need to rollback :(
            // (ie do nothing)
            //oh and send an error to the client!
            asynContainer->player1->getTrade()->cancelTradeSession();
            asynContainer->player2->getTrade()->cancelTradeSession();
            return;
        }

        //proceed as normal our transaction has been a success
        asynContainer->player1->getTrade()->processTradeListPostTransaction();
        asynContainer->player2->getTrade()->processTradeListPostTransaction();
        asynContainer->player1->getTrade()->updateCash(-asynContainer->amount1+asynContainer->amount2);
        asynContainer->player2->getTrade()->updateCash(-asynContainer->amount2+asynContainer->amount1);
        asynContainer->player1->getTrade()->endTradeSession();
        asynContainer->player2->getTrade()->endTradeSession();


    }
    break;

    case TRMQuery_ACKRetrieval:
    {
        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opBidAuctionResponseMessage);//opRetrieveAuctionItemResponseMessage);
        gMessageFactory->addUint64(asynContainer->AuctionID);
        gMessageFactory->addUint32(0);
        Message* newMessage = gMessageFactory->EndMessage();
        asynContainer->mClient->SendChannelA(newMessage, asynContainer->mClient->getAccountId(),  CR_Client, 6);
    }
    break;

    case TRMQuery_BazaarMoneyTransaction:
    {
        uint32 error;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);
        result->getNextRow(binding,&error);
        if (error == 0)
        {
            // no errors : lets move the money
            if(asynContainer->player1 != NULL && asynContainer->player1->getConnectionState() == PlayerConnState_Connected)
            {
                asynContainer->player1->giveInventoryCredits(-asynContainer->amountcash);
                asynContainer->player1->giveBankCredits(-asynContainer->amountbank);
            }

            //CAVE player2 does NOT exist if the seller is NOT online
            if(asynContainer->player2 != NULL && asynContainer->player2->getConnectionState() == PlayerConnState_Connected)
            {
                asynContainer->player2->giveBankCredits(asynContainer->amount1);
            }

            gMessageLib->sendBidAuctionResponseMessage(asynContainer->player1, asynContainer->AuctionID,0);

            //now send the relevant Emails
            //player1 is new owner; player2 is old owner
            //tell the old owner that the instant has been sold
            //we cannot really give this back to chat because we cant guarantee that the auctiondb entry still exists at this point
            gMessageLib->sendSoldInstantMail(asynContainer->sellerID, asynContainer->player1, asynContainer->name, asynContainer->amount1, asynContainer->mPlanet, asynContainer->mRegion);

            //now inform the new owner that the auction has been won
            gMessageLib->sendBoughtInstantMail(asynContainer->player1, asynContainer->name, asynContainer->mOwner, asynContainer->amount1, asynContainer->mPlanet, asynContainer->mRegion,asynContainer->mX,asynContainer->mY);



        }
        else
        {
            //hmmm reroll so dont change the money item is still with the old owner in  the db
            gMessageLib->sendBidAuctionResponseMessage(asynContainer->player1, asynContainer->AuctionID,1);
        }
    }
    break;

    case TRMQuery_CheckListingsBazaar:
    {

        uint64 count = result->getRowCount();

        PlayerObject* playerObject = asynContainer->player1;

        //to many listings ??  default is 25
        //max price on public bazaar

        if(count >= static_cast<uint64>(gWorldConfig->getConfiguration<uint32>("Server_Bazaar_MaxListing",(uint32)25)))	{
            gMessageLib->sendCreateAuctionItemResponseMessage(playerObject,asynContainer->tangible->getId(),13);
            return;
        }

        TradeManagerAsyncContainer*	asyncContainer = new TradeManagerAsyncContainer(TRMQuery_CreateAuctionTransaction,asynContainer->player1->getClient());

        asyncContainer->tangible	= asynContainer->tangible;
        asyncContainer->player1		= asynContainer->player1;
        asyncContainer->BazaarID	= asynContainer->BazaarID;
        asyncContainer->crc			= asynContainer->crc;
        asyncContainer->time		= asynContainer->time;
        asyncContainer->mClient		= asynContainer->mClient;
        asyncContainer->amountcash	= asynContainer->amountcash;
        asyncContainer->amountbank	= asynContainer->amountbank;

        Transaction* mTransaction = mDatabase->startTransaction(this,asyncContainer);
        int8 sql[200];

        sprintf(sql,"UPDATE %s.inventories SET credits=credits-%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),asyncContainer->amountcash, playerObject->getId()+INVENTORY_OFFSET);
        mTransaction->addQuery(sql);
        sprintf(sql,"UPDATE %s.banks SET credits=credits-%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),asyncContainer->amountbank, playerObject->getId()+BANK_OFFSET);
        mTransaction->addQuery(sql);
        int8 query[2000];
        sprintf(query,"INSERT INTO %s.commerce_auction SET auction_id = %" PRIu64 ", owner_id = %" PRIu64 ", bazaar_id = %" PRIu64 ", type = %u, start = %u ,premium = %u, category = 0, itemtype = %u, price = %u, name = '%s', description = '%s', region_id = 0, planet_id = 0, bidder_name = '', object_string = '%s'",mDatabase->galaxy(), asynContainer->tangible->getId(),playerObject->getId(),asynContainer->BazaarID,asynContainer->auctionType,asynContainer->time,asynContainer->premium,asynContainer->itemType,asynContainer->price, asynContainer->name.getAnsi(),asynContainer->description.getAnsi(),asynContainer->tang.getAnsi());
        mTransaction->addQuery(query);
        mTransaction->execute();
       

    }
    break;

    case TRMQuery_CreateAuctionTransaction:
    {
        //creates the Auction in the bazaar
        //takes the bazaar fees

        uint32 error;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);
        result->getNextRow(binding,&error);
        uint64 itemId = asynContainer->tangible->getId();

        if (error)	{
            gMessageLib->sendCreateAuctionItemResponseMessage(asynContainer->player1,itemId,2);
            return;
        }

        //sort the money
        asynContainer->player1->giveInventoryCredits(-asynContainer->amountcash);
        asynContainer->player1->giveBankCredits(-asynContainer->amountbank);
        gMessageLib->sendCreateAuctionItemResponseMessage(asynContainer->player1,itemId,0);

        //assign the Bazaar as the new owner to the item
        gObjectFactory->GiveNewOwnerInDB(asynContainer->tangible,asynContainer->BazaarID);

        TangibleObject* container = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(asynContainer->tangible->getParentId()));
        
		//now remove it from the zone
		if(container)        {
            gContainerManager->deleteObject(asynContainer->tangible,container);
        }

        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opProcessCreateAuction);
        gMessageFactory->addUint64(asynContainer->player1->getId());
        gMessageFactory->addUint64(asynContainer->BazaarID);
        gMessageFactory->addUint64(itemId);
        gMessageFactory->addUint32(asynContainer->crc);
        gMessageFactory->addUint32(asynContainer->time);

        Message* newMessage = gMessageFactory->EndMessage();
        asynContainer->mClient->SendChannelA(newMessage, asynContainer->mClient->getAccountId(), CR_Chat, 6);

    }
    break;

    case TRMQuery_ItemTableFrogQuery:
    {

        DataBinding* binding = mDatabase->createDataBinding(6);
        binding->addField(DFT_uint64,offsetof(ItemFrogItemClass,id),8,0);
        binding->addField(DFT_uint32,offsetof(ItemFrogItemClass,type),4,1);
        binding->addField(DFT_uint32,offsetof(ItemFrogItemClass,family),4,2);
        binding->addField(DFT_bstring,offsetof(ItemFrogItemClass,name),32,3);
        binding->addField(DFT_uint8,offsetof(ItemFrogItemClass,active),1,4);
        binding->addField(DFT_bstring,offsetof(ItemFrogItemClass,f_name),32,6);


        uint64 count = result->getRowCount();

        for(uint64 i=0; i <count; i++)
        {
            ItemFrogItemClass*	item = new(ItemFrogItemClass);
            result->getNextRow(binding,item);

            ItemFrogTypeClass* type = mItemFrogClass.LookUpFamily(item->family);
            if(!type)
            {
                type = new(ItemFrogTypeClass);
                type->family = item->family;
                type->f_name = item->f_name;
                mItemFrogClass.AddFamily(type);
            }
            type->InsertItem(item);

        }

		if (count)
		{
			LOG(INFO) << "Loaded " << count << " frog items";
		}

    }
    break;

    case TRMQuery_NULL:
    {
        break;
    }

    default:
        break;
    }
    delete(asynContainer);
}

//=======================================================================================================================
//Deduct amount from the bank only
//=======================================================================================================================

void TradeManager::_processBankTipDeduct(Message* message,DispatchClient* client)
{
    uint64			playerId		= message->getUint64();//is on this zone
    uint32			amount			= message->getUint32();
    PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));
    Bank*			bank			= dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

    bank->credits(bank->credits() + amount);
    gMessageLib->sendBankCreditsUpdate(playerObject);
}

//=======================================================================================================================
//update the bankcredits and finish the banktip
//=======================================================================================================================

void TradeManager::_processBanktipUpdate(Message* message,DispatchClient* client)
{
    uint64	playerId	= message->getUint64();//is on this zone
    uint32	amount		= message->getUint32();

    if(PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId)))
    {
        Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

        bank->credits(bank->credits() + amount);

        gTreasuryManager->saveAndUpdateBankCredits(playerObject);
    }
}

//=======================================================================================================================
//we have the position and planet now create the WP
//=======================================================================================================================

void TradeManager::_processFindFriendCreateWaypointMessage(Message* message,DispatchClient* client)
{
    uint64	playerId	= message->getUint64();//is on this zone

    BString	playerFriendName;
    message->getStringAnsi(playerFriendName);

    uint32 planet = message->getUint32();
    float x = message->getFloat();
    float z = message->getFloat();

    PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

    Datapad* datapad			= playerObject->getDataPad();

    glm::vec3 position;
    position.x = x;
    position.z = z;

    WaypointObject* wp = datapad->getWaypointByName(playerFriendName);
    if(wp)
    {
        //update instead of deleting and re-creating...

        datapad->updateWaypoint(wp->getId(), playerFriendName.getAnsi(), position, static_cast<uint16>(planet), playerObject->getId(), WAYPOINT_ACTIVE);
        gMessageLib->sendUpdateWaypoint(wp,ObjectUpdateChange,playerObject);
    }
    else
    {
        if(datapad->getCapacity())
        {
            datapad->requestNewWaypoint(playerFriendName.getAnsi(),position,static_cast<uint16>(planet),Waypoint_blue);
        }
    }
}

//=======================================================================================================================
//provides the chatserver with current positioning
//=======================================================================================================================

void TradeManager::_processFindFriendRequestPositionMessage(Message* message,DispatchClient* client)
{
    uint64	playerFriend		= message->getUint64();//is on this zone
    uint64	player				= message->getUint64();//is on another zone

    PlayerObject* friendObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerFriend));

    if(!friendObject)
    {
        return;
    }

    gMessageLib->sendFindFriendLocation(friendObject,playerFriend,player,friendObject->mPosition.x,friendObject->mPosition.z);
}

//=======================================================================================================================
//handles a purchase for the chatserver.
//=======================================================================================================================
void TradeManager::_processDeductMoneyMessage(Message* message,DispatchClient* client)
{
    uint64	buyerID		= message->getUint64();//new owner
    uint64	sellerID	= message->getUint64();//old owner - receives the money
    uint64	itemID		= message->getUint64();
    int32	amount		= message->getUint32();
    uint32	time		= message->getUint32();
    BString	name;
    message->getStringAnsi(name);
    BString	planet;
    message->getStringAnsi(planet);
    BString	region;
    message->getStringAnsi(region);
    BString	owner;
    message->getStringAnsi(owner);
    BString	x;
    message->getStringAnsi(x);
    BString	y;
    message->getStringAnsi(y);

    TradeManagerAsyncContainer* asyncContainer;
    asyncContainer = new TradeManagerAsyncContainer(TRMQuery_BazaarMoneyTransaction,client);
    asyncContainer->player1 = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(buyerID));
    asyncContainer->player2 = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(sellerID));
    asyncContainer->sellerID = sellerID;
    asyncContainer->amount1 = amount;
    asyncContainer->AuctionID = itemID;
    asyncContainer->amountbank = 0;
    asyncContainer->amountcash = amount;
    asyncContainer->name = name;
    asyncContainer->mRegion = region;
    asyncContainer->mPlanet = planet;
    asyncContainer->mOwner = owner;
    asyncContainer->mX = atoi(x.getAnsi());
    asyncContainer->mY = atoi(y.getAnsi());

    Inventory* inventory = dynamic_cast<Inventory*>(asyncContainer->player1->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

    if(inventory->getCredits() < amount)
    {
        //cash alone isnt sufficient
        asyncContainer->amountcash = inventory->getCredits();
        asyncContainer->amountbank = (amount - asyncContainer->amountcash);
    }

    //ok use transactions and see to the object in memory in the postransaction
    Transaction* mTransaction = mDatabase->startTransaction(this,asyncContainer);
    int8 sql[200];
    sprintf(sql,"UPDATE %s.inventories SET credits=credits-%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),asyncContainer->amountcash, buyerID+INVENTORY_OFFSET);
    mTransaction->addQuery(sql);
    sprintf(sql,"UPDATE %s.banks SET credits=credits-%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),asyncContainer->amountbank, buyerID+BANK_OFFSET);
    mTransaction->addQuery(sql);
    sprintf(sql,"UPDATE %s.banks SET credits=credits+%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),amount, sellerID+BANK_OFFSET);
    mTransaction->addQuery(sql);
    //set owner id to new owner. the item will be taken out of the bazaar in the next step IF the buyer is near
    sprintf(sql,"UPDATE %s.commerce_auction SET owner_id = %" PRIu64 ", type = %u,start = %u WHERE auction_id = %" PRIu64 "",mDatabase->galaxy(),buyerID,TRMVendor_Cancelled,time,itemID);
    mTransaction->addQuery(sql);
   

    mTransaction->execute();
}

//=======================================================================================================================
// send by the chatserver to have us create the item we just received from the vendor
// ?????
//
void TradeManager::_processCreateItemMessage(Message* message,DispatchClient* client)
{
    uint64			PlayerID		= message->getUint64();
    uint64			ItemID			= message->getUint64();
    uint32			Group			= message->getUint32();
    PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(PlayerID));
    Inventory*		inventory		= dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));


    if(playerObject && playerObject->isConnected())
    {
        gObjectFactory->requestTanoNewParent(inventory,ItemID,inventory->getId(),(TangibleGroup)Group);
    }
}

//=======================================================================================================================

void TradeManager::_processHandleAuctionCreateMessage(Message* message,DispatchClient* client)
{
    _HandleAuctionCreateMessage(message,client, TRMVendor_Auction);
}

void TradeManager::_processHandleImmediateAuctionCreateMessage(Message* message,DispatchClient* client)
{
    _HandleAuctionCreateMessage(message, client, TRMVendor_Instant);
}

void TradeManager::_HandleAuctionCreateMessage(Message* message,DispatchClient* client, TRMAuctionType auction)
{
    TradeManagerAsyncContainer* asyncContainer;

    PlayerObject*	playerObject	= gWorldManager->getPlayerByAccId(client->getAccountId());
    //Inventory*		inventory		= dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

    //the items description we provide
    BString			Description;

    //squeeze our Packet for all usefull information
    uint64	ItemID		= message->getUint64();
    uint64	VendorID	= message->getUint64();
    uint32	price		= message->getUint32();
    uint32	Time		= message->getUint32();

    message->getStringUnicode16(Description);

    //premium Flag
    uint8	premium	= message->getUint8();

    if(!(playerObject && playerObject->isConnected()))
    {
        //some error
        gMessageLib->sendCreateAuctionItemResponseMessage(playerObject,ItemID,1);
        return;
    }

    //auction description
    Description.convert(BSTRType_ANSI);

    if(Description.getLength() >= 1024)
        Description.setLength(1024);

    int8 theDescription[1028];
    mDatabase->escapeString(theDescription,Description.getAnsi(),Description.getLength());


    TangibleObject*		requestedObject		= dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(ItemID));
    
	if(!requestedObject)    {
        //cave we might sell datapad schematics, too
        return;
    }
	
	Item*				item				= dynamic_cast<Item*>(requestedObject);

    //it might be a resourcecontaine - so its a tangibleObject
    //however only items can be equipped
    if (item)    {
        //unequips the item in case it is equipped
        playerObject->getEquipManager()->unEquipItem(item);

        //will destroy any temporary placed instruments
        checkPlacedInstrument(item,client);
    }

    if(price > (uint32)gWorldConfig->getConfiguration<uint32>("Server_Bazaar_MaxPrice",20000))
    {
        //to expensive!!!
        gMessageLib->sendCreateAuctionItemResponseMessage(playerObject,ItemID,4);
        return;
    }

    //just initialize some things in case we get something which is not a tano
    uint32 category = requestedObject->getCategoryBazaar();

    BString name, customName, tang;

    uint32 itemType = 0;

    //get the proper name
    name		= requestedObject->getBazaarName();
    customName	= requestedObject->getCustomName();
    if(customName.getLength()>0)
    {
        customName.convert(BSTRType_ANSI);
        name = customName ;
    }

    int8 theName[128];
    mDatabase->escapeString(theName,name.getAnsi(),name.getLength());

    itemType	= requestedObject->getTangibleGroup();
    tang		= requestedObject->getBazaarTang();

    // check for funds
    // and calculate for fees dependend on skill

    int32 fee = 20;

    if(premium)    {
        fee = 100;
    }

    if(playerObject->checkSkill(SMSkill_Merchant_Sales_1))    {
        //efficiency 1
        fee = 16;
        if(premium)	{
            fee = 80;
        }
    }
    else if(playerObject->checkSkill(SMSkill_Merchant_Sales_3))    {
        //efficiency 3
        fee = 12;
        if(premium)	{
            fee = 80;
        }
    }


    if(!playerObject->checkDeductCredits(fee))	{
        //we cannot afford the listing
        gMessageLib->sendCreateAuctionItemResponseMessage(playerObject,ItemID,9);
        return;

    }

    //=============================================================================
    //prepare our transaction
    asyncContainer = new TradeManagerAsyncContainer(TRMQuery_CheckListingsBazaar,client);

    Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

    //we give the amounts of money to be taken from bank and inventory directly to the
    //transaction in the next async call

    if(bank->credits() < fee)	{
        asyncContainer->amountbank = bank->credits();
        asyncContainer->amountcash = fee - bank->credits();
    }
    else    {
        asyncContainer->amountbank = fee;
        asyncContainer->amountcash = 0;
    }

    asyncContainer->tangible	= requestedObject;
    asyncContainer->player1		= playerObject;
    asyncContainer->BazaarID	= VendorID;
    asyncContainer->crc			= category;
    asyncContainer->time		= Time;
    asyncContainer->mClient		= client;
    asyncContainer->auctionType = auction;
    asyncContainer->premium		= premium;
    asyncContainer->itemType	= itemType;
    asyncContainer->price		= price;
    asyncContainer->name		= theName;
    asyncContainer->description	= theDescription;
    asyncContainer->tang		= tang;


    int8 sql[200];
    sprintf(sql,"Select * FROM commerce_auction WHERE owner_id = '%" PRIu64 "'", playerObject->getId());
    mDatabase->executeSqlAsync(this,asyncContainer,sql);


}

//=======================================================================================================================

void TradeManager::_processAbortTradeMessage(Message* message,DispatchClient* client)
{
    PlayerObject* playerObject = gWorldManager->getPlayerByAccId(client->getAccountId());

    if(playerObject && playerObject->isConnected())
    {
        if(playerObject->getTradeStatus())
        {
            PlayerObject* tradePartner = (PlayerObject*) gWorldManager->getObjectById(playerObject->getTradePartner());

            if(tradePartner)
                tradePartner->getTrade()->cancelTradeSession();

            playerObject->getTrade()->cancelTradeSession();

        }
    }
}

//=======================================================================================================================

void TradeManager::_processTradeCompleteMessage(Message* message,DispatchClient* client)
{
    PlayerObject* playerObject = gWorldManager->getPlayerByAccId(client->getAccountId());
    PlayerObject* tradePartner = (PlayerObject*) gWorldManager->getObjectById(playerObject->getTradePartner());

    if(playerObject && playerObject->isConnected())
    {
        if(!tradePartner)
        {
            playerObject->setTradeStatus(false);
            playerObject->getTrade()->cancelTradeSession();

        }

        if(playerObject->getTrade()->getAcceptStatus() && tradePartner->getTrade()->getAcceptStatus())
        {
            playerObject->getTrade()->setAcceptStatus(false);
            tradePartner->getTrade()->setAcceptStatus(false);

            if(playerObject->getTradeStatus() == true)
            {
                playerObject->setTradeStatus(false);
                tradePartner->setTradeStatus(false);
                //check if enough places in the inventory
                if(playerObject->getTrade()->checkTradeListtoInventory() && tradePartner->getTrade()->checkTradeListtoInventory())
                {
                    //Objects must get the proper Owner Id
                    //Delete them in the partners inventory
                    if(playerObject->checkDeductCredits(playerObject->getTrade()->getTradeCredits()) && tradePartner->checkDeductCredits(tradePartner->getTrade()->getTradeCredits()))
                    {
                        TradeTransaction(client,playerObject,tradePartner);
                    }
                    else
                    {
                        playerObject->getTrade()->cancelTradeSession();
                        tradePartner->getTrade()->cancelTradeSession();
                    }
                }
                else
                {
                    //this message gets send to both tradepartners!
                    playerObject->getTrade()->cancelTradeSession();
                    tradePartner->getTrade()->cancelTradeSession();
                }
            }
        }
        else
        {
            DLOG(INFO) << "TradeManager Trade finished without Accept !!!!!";
        }
    }
}

//=======================================================================================================================

void TradeManager::TradeTransaction(DispatchClient* client,PlayerObject* player1,PlayerObject* player2)
{
    TradeManagerAsyncContainer* asyncContainer;
    asyncContainer = new TradeManagerAsyncContainer(TRMQuery_MoneyTransaction,client);

    //lets start with the cash

    asyncContainer->player1 = player1;
    asyncContainer->player2 = player2;

    int8 sql[128];
    //trade uses always cash (thats what the client checks)
    asyncContainer->amount1 = player1->getTrade()->getTradeCredits();
    asyncContainer->amount2 = player2->getTrade()->getTradeCredits();
    Transaction* mTransaction = mDatabase->startTransaction(this,asyncContainer);

    if (player1->testCash(asyncContainer->amount1) && player2->testCash(asyncContainer->amount2))
    {
        sprintf(sql,"UPDATE %s.inventories SET credits=credits+%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),-asyncContainer->amount1+asyncContainer->amount2, player1->getId()+1);
        mTransaction->addQuery(sql);
        sprintf(sql,"UPDATE %s.inventories SET credits=credits+%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),-asyncContainer->amount2+asyncContainer->amount1, player2->getId()+1);
        mTransaction->addQuery(sql);

        //now we need to add the items

        player1->getTrade()->processTradeListPreTransaction(mTransaction);
        player2->getTrade()->processTradeListPreTransaction(mTransaction);
        //playerObject->getTradePartner()->getTrade()->processTradeList();

        mTransaction->execute();
      
    }
    else
    {
        //running in this spot basically means we deal with a cheater since the client checks the amounts
        player1->getTrade()->cancelTradeSession();
        player2->getTrade()->cancelTradeSession();

        DLOG(INFO) << "TradeManager Trade likely to have been tampered with";

        if (!player1->testCash(asyncContainer->amount1) )
        {
			DLOG(INFO) <<"Player : "<<player1->getFirstName().getAnsi()<< "id "<<player1->getId()<< "wanted to trade "<<asyncContainer->amount1<<" credits but had only " << dynamic_cast<Inventory*>(player1->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits();
        }
        if (!player2->testCash(asyncContainer->amount2) )
        {
            DLOG(INFO) <<"Player : "<<player2->getFirstName().getAnsi()<< "id "<<player2->getId()<< "wanted to trade "<<asyncContainer->amount2<<" credits but had only " << dynamic_cast<Inventory*>(player2->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits();
        }
    }
}

//=======================================================================================================================

void TradeManager::_processVerificationMessage(Message* message,DispatchClient* client)
{
    PlayerObject* playerObject = gWorldManager->getPlayerByAccId(client->getAccountId());
    PlayerObject* tradePartner = (PlayerObject*) gWorldManager->getObjectById(playerObject->getTradePartner());

    if((!tradePartner)||(!tradePartner->isConnected()))
    {
        playerObject->getTrade()->cancelTradeSession();
        return;
    }

    if((playerObject && playerObject->isConnected()) && (tradePartner  && tradePartner->isConnected()))
    {
        playerObject->getTrade()->setTradeFinishedStatus(true);

        //make sure 'partner' is still set to be trading...not sure how this would fail but it CAN: http://paste.swganh.org/viewp.php?id=20100627222833-5bae6fb86eda66de328ca73fb1aae6eb
        if(tradePartner->getTradeStatus()) {
            if(tradePartner->getTrade()->getTradeFinishedStatus())
            {
                playerObject->getTrade()->setTradeFinishedStatus(false);
                tradePartner->getTrade()->setTradeFinishedStatus(false);
                _processTradeCompleteMessage(message,client);
            }
        }
    }
}

//=======================================================================================================================

void TradeManager::_processBeginVerificationMessage(Message* message,DispatchClient* client)
{
    PlayerObject* playerObject = gWorldManager->getPlayerByAccId(client->getAccountId());
    PlayerObject* tradePartner = (PlayerObject*) gWorldManager->getObjectById(playerObject->getTradePartner());

    if(playerObject != NULL && playerObject->getConnectionState() == PlayerConnState_Connected)
    {
        playerObject->getTrade()->setAcceptStatus(true);
        gMessageLib->sendBeginVerificationMessage(tradePartner);
    }
}

//=======================================================================================================================

void TradeManager::_processAcceptTransactionMessage(Message* message,DispatchClient* client)
{
    PlayerObject* playerObject = gWorldManager->getPlayerByAccId(client->getAccountId());
    PlayerObject* tradePartner = (PlayerObject*) gWorldManager->getObjectById(playerObject->getTradePartner());

    if(playerObject != NULL && playerObject->getConnectionState() == PlayerConnState_Connected)
    {
        playerObject->getTrade()->setAcceptStatus(true);
        gMessageLib->sendAcceptTradeMessage(tradePartner);
    }
}

//=======================================================================================================================

void TradeManager::_processGiveMoneyMessage(Message* message,DispatchClient* client)
{
    PlayerObject* playerObject = gWorldManager->getPlayerByAccId(client->getAccountId());
    PlayerObject* tradePartner = (PlayerObject*) gWorldManager->getObjectById(playerObject->getTradePartner());

    if(playerObject && playerObject->isConnected())
    {
        uint32	Money	= message->getUint32();
        playerObject->getTrade()->setTradeCredits(Money);
        gMessageLib->sendGiveMoneyMessage(tradePartner,Money);
    }
}


//=======================================================================================================================

void TradeManager::_processUnacceptTransactionMessage(Message* message,DispatchClient* client)
{
    PlayerObject* playerObject = gWorldManager->getPlayerByAccId(client->getAccountId());
    PlayerObject* tradePartner = (PlayerObject*) gWorldManager->getObjectById(playerObject->getTradePartner());

    if(playerObject && playerObject->isConnected())
    {
        playerObject->getTrade()->setAcceptStatus(false);
        gMessageLib->sendUnacceptTradeMessage(tradePartner);
    }
}
//=======================================================================================================================

void TradeManager::_processRemoveItemMessage(Message* message,DispatchClient* client)
{
    //not possible with our client :(
    PlayerObject* playerObject = gWorldManager->getPlayerByAccId(client->getAccountId());
    PlayerObject* tradePartner = (PlayerObject*) gWorldManager->getObjectById(playerObject->getTradePartner());

    if(playerObject && playerObject->isConnected())
    {
        uint64	ItemId	= message->getUint64();
        //Update the tradelist
        //playerObject->getTrade()->TradeListDeleteItem(ItemId);
        //Update the tradewindow
        gMessageLib->sendDeleteItemMessage(playerObject,ItemId);
        gMessageLib->sendDeleteItemMessage(tradePartner,ItemId);
    }
}

//=======================================================================================================================

void TradeManager::_processAddItemMessage(Message* message,DispatchClient* client)
{
    PlayerObject*	playerObject	= gWorldManager->getPlayerByAccId(client->getAccountId());
    PlayerObject* tradePartner = (PlayerObject*) gWorldManager->getObjectById(playerObject->getTradePartner());

    //Inventory*		inventory		= dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

    if(!(playerObject && playerObject->isConnected()))
    {
        return;
    }

    if (!playerObject->getTradeStatus())
    {
        return;
    }

    uint64				ItemId		= message->getUint64();
    TangibleObject*		addedItem	= dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(ItemId));

    if (!addedItem)
    {
        return;
    }

    //check if the giver really owns the item
    //still TODO
    //CAVE it may be in a container that he is carrying!!!!


    //inventory items
    Item* theitem = dynamic_cast<Item*>(gWorldManager->getObjectById(ItemId));
    if (theitem && playerObject->getController()->checkContainingContainer(addedItem->getParentId(),playerObject->getId()))
    {

        //make sure the item is not equipped
        playerObject->getEquipManager()->unEquipItem(theitem);
    }
    //make sure that the item is only once on the list
    if (playerObject->getTrade()->ItemTradeCheck(ItemId) == false)
    {
        //item must be added to our trade list
        //hark! always add to the itemOwners list!
        playerObject->getTrade()->ItemTradeAdd(addedItem,tradePartner,playerObject);
        //inform the receivers client about that
        informTradePartner(addedItem,tradePartner);

    }
}

//=======================================================================================================================

void TradeManager::informTradePartner(TangibleObject* item,PlayerObject* tradePartner)
{
	gMessageLib->sendAddItemMessage(tradePartner,item);

	switch (item->getTangibleGroup())
	{
		case TanGroup_ResourceContainer:
		{
			gMessageLib->sendCreateResourceContainer((ResourceContainer*)item,tradePartner);
		}
		break;

		default:
		{
			gMessageLib->sendCreateTano(item,tradePartner);
		}
		break;
	}
}

//=============================================================================
//checks whether there is a placed copy and deletes it
//=============================================================================

void TradeManager::checkPlacedInstrument(Item* addedItem,DispatchClient* client)
{
    if(PlayerObject* playerObject = gWorldManager->getPlayerByAccId(client->getAccountId()))
    {
        if(addedItem && addedItem->getItemFamily() == ItemFamily_Instrument && addedItem->getPlaced())
        {
            if(Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById(addedItem->getNonPersistantCopy())))
            {
                // yes it has a placed copy
                playerObject->getController()->destroyObject(item->getId());
            }
        }
    }
}

//=============================================================================
//add a tradeinvitation to the targeted player
//

void TradeManager::addTradeInvitation(PlayerObject* invitedPlayer, PlayerObject* invitingPlayer)
{
    //We (this) are the inviter
    //make it known to the partner

    gMessageLib->sendSecureTrade(invitedPlayer,invitingPlayer,1);
    //check if we are already invited by the player
    if(invitingPlayer->getTrade()->verifyInvitation(invitedPlayer) == false)
    {
        invitedPlayer->getTrade()->tradeInvitationAdded(invitingPlayer);
    }
    else
    {
        // we are invited by the player lets start trading
        //no need to put us on the list but remove us from the other players list
        invitingPlayer->setTradePartner(invitedPlayer->getId());
        invitedPlayer->setTradePartner(invitingPlayer->getId());

        invitingPlayer->getTrade()->deleteTradeInvitation(invitedPlayer);

        invitingPlayer->setTradeStatus(true);
        invitedPlayer->setTradeStatus(true);
        gMessageLib->sendBeginTradeMessage(invitingPlayer,invitedPlayer);
        gMessageLib->sendBeginTradeMessage(invitedPlayer,invitingPlayer);
    }
}



//=============================================================================
ItemFrogClass::~ItemFrogClass()
{
    //mCommoditiesHashList.clear();

    ItemFrogTypeList::iterator it = mItemFrogTypeList.begin();
    while(it != mItemFrogTypeList.end())
    {
        SAFE_DELETE(*it);
        it = mItemFrogTypeList.begin();
    }

}

ItemFrogTypeClass::~ItemFrogTypeClass()
{
    //mCommoditiesHashList.clear();

    ItemFrogItemList::iterator it = mItemFrogItemList.begin();
    while(it != mItemFrogItemList.end())
    {
        SAFE_DELETE(*it);
        it = mItemFrogItemList.begin();
    }

}

ItemFrogTypeClass*ItemFrogClass::LookUpFamily(uint32 family)
{
    ItemFrogTypeList::iterator it = mItemFrogTypeList.begin();
    while(it != mItemFrogTypeList.end())
    {
        if((*it)->family == family)
            return (*it);

        it++;
    }
    return NULL;
}

ItemFrogItemClass*	ItemFrogClass::LookUpType(uint32 type)
{
    ItemFrogTypeList::iterator it = mItemFrogTypeList.begin();
    while(it != mItemFrogTypeList.end())
    {
        ItemFrogItemClass* item = (*it)->getType(type);
        if(item != NULL)
            return item;

        it++;
    }
    return NULL;
}

ItemFrogItemClass*	ItemFrogTypeClass::getType(uint32 type)
{
    ItemFrogItemList::iterator it = mItemFrogItemList.begin();
    while(it != mItemFrogItemList.end())
    {
        if((*it)->GetType() == type)
            return (*it);
        it++;
    }
    return NULL;

}
