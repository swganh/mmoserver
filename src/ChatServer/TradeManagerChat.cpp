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

#include "ChatOpcodes.h"
#include "Mail.h"
#include "Player.h"
#include "TradeManagerChat.h"
#include "TradeManagerHelp.h"




#include "Utils/logger.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"

#include "Common/atMacroString.h"
#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"

#include "Utils/utils.h"
#include "Utils/Timer.h"

#include <boost/lexical_cast.hpp>

#include <cstring>
#include <ctime>

bool						TradeManagerChatHandler::mInsFlag    = false;
TradeManagerChatHandler*		TradeManagerChatHandler::mSingleton  = NULL;

uint32 TradeManagerChatHandler::getBazaarRegion(uint64 ID)
{
    //uint32 Region = 0;

    BazaarList::iterator it = mBazaars.begin();
    while(it != mBazaars.end())
    {
        if ((*it)->id == ID )
            return (*it)->regionid;
        it++;
    }
    return(0);
}

//======================================================================================================================

BString TradeManagerChatHandler::getBazaarString(uint64 ID)
{
    //uint32 Region = 0;

    BazaarList::iterator it = mBazaars.begin();
    while(it != mBazaars.end())
    {
        if ((*it)->id == ID )
            return (*it)->string;
        it++;
    }
    return("");
}

//======================================================================================================================

Bazaar* TradeManagerChatHandler::getBazaarInfo(uint64 ID)
{
    //uint32 Region = 0;

    BazaarList::iterator it = mBazaars.begin();
    while(it != mBazaars.end())
    {
        if ((*it)->id == ID )
            return (*it);
        it++;
    }
    return(NULL);
}

//======================================================================================================================

TradeManagerChatHandler::TradeManagerChatHandler(Database* database, MessageDispatch* dispatch, ChatManager* chatManager)
{
    mBazaarsLoaded = false;
    mBazaarCount = 0;
    mBazaarMaxBid = 20000;

    mDatabase = database;
    mChatManager = chatManager;
    mMessageDispatch = dispatch;
    mPlayerAccountMap = mChatManager->getPlayerAccountMap();
    TradeManagerAsyncContainer* asyncContainer;

    mMessageDispatch->RegisterMessageCallback(opIsVendorMessage,std::bind(&TradeManagerChatHandler::processHandleIsVendorMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opAuctionQueryHeadersMessage,std::bind(&TradeManagerChatHandler::processHandleopAuctionQueryHeadersMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opGetAuctionDetails,std::bind(&TradeManagerChatHandler::processGetAuctionDetails, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opCancelLiveAuctionMessage,std::bind(&TradeManagerChatHandler::processCancelLiveAuctionMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opRetrieveAuctionItemMessage,std::bind(&TradeManagerChatHandler::processRetrieveAuctionItemMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opProcessCreateAuction,std::bind(&TradeManagerChatHandler::ProcessCreateAuction, this, std::placeholders::_1, std::placeholders::_2));
    //mMessageDispatch->RegisterMessageCallback(opGetCommoditiesTypeList,std::bind(&TradeManagerChatHandler::_ProcessRequestTypeList, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opBidAuctionMessage,std::bind(&TradeManagerChatHandler::processBidAuctionMessage, this, std::placeholders::_1, std::placeholders::_2));
    mMessageDispatch->RegisterMessageCallback(opBankTipDustOff,std::bind(&TradeManagerChatHandler::ProcessBankTip, this, std::placeholders::_1, std::placeholders::_2));

    // load our bazaar terminals
    asyncContainer = new TradeManagerAsyncContainer(TRMQuery_LoadBazaar, 0);
    mDatabase->executeProcedureAsync(this, asyncContainer, "CALL %s.sp_BazaarTerminalsGet();",mDatabase->galaxy());
    

    // load our global tick
    asyncContainer = new TradeManagerAsyncContainer(TRMQuery_LoadGlobalTick, 0);

    // @todo Hardcoded galaxy at this time, to be changed.
    mDatabase->executeProcedureAsync(this, asyncContainer, "CALL %s.sp_ServerGlobalTickGet (%u);", mDatabase->galaxy(),  2);
    

    //load the TypeList to properly sort our items in Bazzarcategories
    //asyncContainer = new TradeManagerAsyncContainer(TRMQuery_TypeList,0);
    //mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT category_mask, directory, name FROM swganh.commerce_item_types");


    mTimerQueueProcessTimeLimit = 10;

    //move to Handle dispatch message at some time
    uint32 ServerTimeInterval = 1;

    std::tr1::shared_ptr<Timer> global_tick_timer(new Timer(CMTimer_GlobalTick,this,ServerTimeInterval*1000,NULL));
    std::tr1::shared_ptr<Timer> tick_preserve_timer(new Timer(CMTimer_TickPreserve,this,ServerTimeInterval*10000,NULL));
    std::tr1::shared_ptr<Timer> check_auctions_timer(new Timer(CMTimer_CheckAuctions,this,ServerTimeInterval*10000,NULL));

    mTimers.push_back(global_tick_timer);
    mTimers.push_back(tick_preserve_timer);
    mTimers.push_back(check_auctions_timer);
    /*
    TimerList::iterator timerIt = mTimers.begin();
    while(timerIt != mTimers.end())
    {
        (*timerIt)->Start();
        ++timerIt;
    }
    */
}


//======================================================================================================================
TradeManagerChatHandler::~TradeManagerChatHandler()
{
    mInsFlag = false;
    // NO, nooooo delete(mSingleton);
    // Not used, cant delete.... delete(Commodities);

//	delete(AuctionHandler);
}
//======================================================================================================================
TradeManagerChatHandler*	TradeManagerChatHandler::Init(Database* database, MessageDispatch* dispatch, ChatManager* chatManager)
{
    if(!mInsFlag)
    {
        mSingleton = new TradeManagerChatHandler(database,dispatch,chatManager);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;

}

//======================================================================================================================
void TradeManagerChatHandler::Shutdown()
{
    // TradeManagerAsyncContainer* asynContainer = new (TradeManagerAsyncContainer);
    // save our global tick

    // @todo Hardcoded galaxy at this time, to be changed.
    mDatabase->executeProcedureAsync(this, NULL, "CALL %s.sp_ServerGlobalTickUpdate(%u, '%" PRIu64 "');", mDatabase->galaxy(),  2, getGlobalTickCount());
    

    mMessageDispatch->UnregisterMessageCallback(opIsVendorMessage);
    mMessageDispatch->UnregisterMessageCallback(opAuctionQueryHeadersMessage);
    mMessageDispatch->UnregisterMessageCallback(opGetAuctionDetails);
    mMessageDispatch->UnregisterMessageCallback(opCancelLiveAuctionMessage);
    mMessageDispatch->UnregisterMessageCallback(opBidAuctionAcceptedMessage);
    mMessageDispatch->UnregisterMessageCallback(opRetrieveAuctionItemMessage);
    mMessageDispatch->UnregisterMessageCallback(opProcessCreateAuction);
    mMessageDispatch->UnregisterMessageCallback(opGetCommoditiesTypeList);
    mMessageDispatch->UnregisterMessageCallback(opBidAuctionMessage);
    mMessageDispatch->UnregisterMessageCallback(opBankTipDustOff);

    mBazaars.clear();
}

//=======================================================================================================================

uint32	TradeManagerChatHandler::TerminalRegionbyID(uint64 id)
{
    BazaarList::iterator it = mBazaars.begin();
    while(it != mBazaars.end())
    {
        if((*it)->id == id) {
            return((*it)->regionid);
            break;
        }
        it++;
    }
    return(-1);
}

//=======================================================================================================================

void TradeManagerChatHandler::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    TradeManagerAsyncContainer* asynContainer = (TradeManagerAsyncContainer*)ref;
    Player* player(0);
    if (asynContainer->mClient) {

        PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(asynContainer->mClient->getAccountId());

        if(accIt != mPlayerAccountMap.end())
            player = (*accIt).second;
        else
        {
            LOG(WARNING) << "Error getting player from account map " << asynContainer->mClient->getAccountId();
            return;
        }
    }

    switch(asynContainer->mQueryType)
    {

    case TRMQuery_RetrieveAuction:
    {
        DataBinding* binding = mDatabase->createDataBinding(3);
        binding->addField(DFT_uint64,offsetof(AuctionItem,ItemID),8,0);
        binding->addField(DFT_uint64,offsetof(AuctionItem,BazaarID),8,1);
        binding->addField(DFT_uint32,offsetof(AuctionItem,ItemTyp),4,2);
        AuctionItem AuctionTemp;
        result->getNextRow(binding,&AuctionTemp);
        //can we retrieve the item from our terminal???
        //uint32 mZoneId = player->getPlanetId();
        uint32 error = 0;
        if (TerminalRegionbyID(asynContainer->BazaarID) == TerminalRegionbyID(AuctionTemp.BazaarID)) {
            TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_DeleteAuction,asynContainer->mClient);

            // Delete from commerce_auction
            mDatabase->executeProcedureAsync(this, asyncContainer, "CALL %s.sp_BazaarAuctionDelete('%" PRIu64 "');", mDatabase->galaxy(),  AuctionTemp.ItemID);

            //send relevant info to Zoneserver for Itemcreation
            gChatMessageLib->processSendCreateItem(asynContainer->mClient, player->getCharId(),AuctionTemp.ItemID, AuctionTemp.ItemTyp, player->getPlanetId());

        } else {
            //send error
            error = 1;
        }
        gChatMessageLib->SendRetrieveAuctionItemResponseMessage(asynContainer->mClient,AuctionTemp.ItemID, error);

        mDatabase->destroyDataBinding(binding);

    }
    break;

    case TRMQuery_CreateAuction:
    {

    }

    break;
    case TRMQuery_ACKRetrieval:
    {
        //probably we should ponder letting the sf respond with 0 in case of everything ok and !0 in case of error
        gChatMessageLib->sendBidAuctionResponse(asynContainer->mClient, 0, asynContainer->AuctionID);
    }
    break;
    case TRMQuery_GetDetails:
    {
        //TODO handle resources!!!!!!!!!!!

        DataBinding* binding = mDatabase->createDataBinding(3);

        binding->addField(DFT_uint64,offsetof(DescriptionItem, ItemID), 8, 0);
        binding->addField(DFT_string,offsetof(DescriptionItem, Description), 1024, 1);
        binding->addField(DFT_string,offsetof(DescriptionItem, tang), 128, 2);

        DescriptionItem* mItemDescription = new DescriptionItem;

        result->getNextRow(binding, mItemDescription);

        //might be a resource
        if(strstr(mItemDescription->tang, "resource_container"))
        {
            //build our query to get the attributes
            int8 sql[1024];
            //we'll need the quantity and all the attributes
            sprintf(sql,"SELECT rc.resource_id, rc.amount, r.name, r.er, r.cr,r.cd,r.dr,r.fl,r.hr,r.ma, r.oq, r.sr, r.ut, r.pe FROM %s.resource_containers rc INNER JOIN %s.resources r ON r.id = rc.resource_id WHERE rc.id =%" PRIu64 "",mDatabase->galaxy(),mDatabase->galaxy(),mItemDescription->ItemID);
            TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_GetResAttributeDetails, asynContainer->mClient);
            asyncContainer->mItemDescription = mItemDescription;

            mDatabase->executeSqlAsync(this,asyncContainer,sql);

            mDatabase->destroyDataBinding(binding);

            return;
        }

        //build our query to get the attributes
        int8 sql[1024];
        //we'll need all the attributes which are marked as external
        sprintf(sql,"SELECT name, value FROM %s.item_attributes ia  INNER JOIN %s.attributes a ON ia.attribute_id = a.id WHERE ia.item_id =%" PRIu64 " and a.internal = 0 ORDER BY ia.order",mDatabase->galaxy(),mDatabase->galaxy(),mItemDescription->ItemID);

        TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_GetAttributeDetails, asynContainer->mClient);
        asyncContainer->mItemDescription = mItemDescription;

        mDatabase->executeSqlAsync(this,asyncContainer,sql);

        mDatabase->destroyDataBinding(binding);
    }
    break;

    case TRMQuery_GetResAttributeDetails:
    {
        DataBinding* binding = mDatabase->createDataBinding(14);

        binding->addField(DFT_uint64,offsetof(ResItemDescriptionAttributes,id),8,0);
        binding->addField(DFT_uint32,offsetof(ResItemDescriptionAttributes,amount),4,1);
        binding->addField(DFT_string,offsetof(ResItemDescriptionAttributes,name),32,2);
        binding->addField(DFT_string,offsetof(ResItemDescriptionAttributes,er),32,3);
        binding->addField(DFT_string,offsetof(ResItemDescriptionAttributes,cr),32,4);
        binding->addField(DFT_string,offsetof(ResItemDescriptionAttributes,cd),32,5);
        binding->addField(DFT_string,offsetof(ResItemDescriptionAttributes,dr),32,6);
        binding->addField(DFT_string,offsetof(ResItemDescriptionAttributes,fl),32,7);
        binding->addField(DFT_string,offsetof(ResItemDescriptionAttributes,hr),32,8);
        binding->addField(DFT_string,offsetof(ResItemDescriptionAttributes,ma),32,9);
        binding->addField(DFT_string,offsetof(ResItemDescriptionAttributes,oq),32,10);
        binding->addField(DFT_string,offsetof(ResItemDescriptionAttributes,sr),32,11);
        binding->addField(DFT_string,offsetof(ResItemDescriptionAttributes,ut),32,12);
        binding->addField(DFT_string,offsetof(ResItemDescriptionAttributes,pe),32, 13);


        uint64 count = result->getRowCount();
        if (count == 1)
        {
            ResItemDescriptionAttributes* mItemDescription = new ResItemDescriptionAttributes;
            result->getNextRow(binding,mItemDescription);


            AttributesList::iterator it = mAtrributesList.begin();

            ItemDescriptionAttributes* resItemDescription = new ItemDescriptionAttributes;
            strcpy(resItemDescription->name,"volume");
            strcpy(resItemDescription->value,"1");
            mAtrributesList.push_back(resItemDescription);

            resItemDescription = new ItemDescriptionAttributes;
            strcpy(resItemDescription->name,"amount");
            sprintf(resItemDescription->value,"%"PRIu32"/100000",mItemDescription->amount);
            mAtrributesList.push_back(resItemDescription);

            resItemDescription = new ItemDescriptionAttributes;
            strcpy(resItemDescription->name,"resource_name");
            sprintf(resItemDescription->value,"%s",mItemDescription->name);
            mAtrributesList.push_back(resItemDescription);

            if(atoi(mItemDescription->er) !=0)
            {
                resItemDescription = new ItemDescriptionAttributes;
                strcpy(resItemDescription->name,"entangle_resistance");
                sprintf(resItemDescription->value,"%s",mItemDescription->er);
                mAtrributesList.push_back(resItemDescription);
            }
            if(atoi(mItemDescription->cr) >0)
            {
                resItemDescription = new ItemDescriptionAttributes;
                strcpy(resItemDescription->name,"res_cold_resist");
                sprintf(resItemDescription->value,"%s",mItemDescription->cr);
                mAtrributesList.push_back(resItemDescription);
            }
            if(atoi(mItemDescription->cd) >0)
            {
                resItemDescription = new ItemDescriptionAttributes;
                strcpy(resItemDescription->name,"res_conductivity");
                sprintf(resItemDescription->value,"%s",mItemDescription->cd);
                mAtrributesList.push_back(resItemDescription);
            }
            if(atoi(mItemDescription->dr) >0)
            {
                resItemDescription = new ItemDescriptionAttributes;
                strcpy(resItemDescription->name,"res_decay_resist");
                sprintf(resItemDescription->value,"%s",mItemDescription->dr);
                mAtrributesList.push_back(resItemDescription);
            }

            if(atoi(mItemDescription->fl) >0)
            {
                resItemDescription = new ItemDescriptionAttributes;
                strcpy(resItemDescription->name,"res_flavor");
                sprintf(resItemDescription->value,"%s",mItemDescription->fl);
                mAtrributesList.push_back(resItemDescription);
            }
            if(atoi(mItemDescription->hr) >0)
            {
                resItemDescription = new ItemDescriptionAttributes;
                strcpy(resItemDescription->name,"res_heat_resist");
                sprintf(resItemDescription->value,"%s",mItemDescription->hr);
                mAtrributesList.push_back(resItemDescription);
            }
            if(atoi(mItemDescription->ma) >0)
            {
                resItemDescription = new ItemDescriptionAttributes;
                strcpy(resItemDescription->name,"res_malleability");
                sprintf(resItemDescription->value,"%s",mItemDescription->ma);
                mAtrributesList.push_back(resItemDescription);
            }
            if(atoi(mItemDescription->oq) >0)
            {
                resItemDescription = new ItemDescriptionAttributes;
                strcpy(resItemDescription->name,"res_quality");
                sprintf(resItemDescription->value,"%s",mItemDescription->oq);
                mAtrributesList.push_back(resItemDescription);
            }
            if(atoi(mItemDescription->sr) >0)
            {
                resItemDescription = new ItemDescriptionAttributes;
                strcpy(resItemDescription->name,"res_shock_resistance");
                sprintf(resItemDescription->value,"%s",mItemDescription->sr);
                mAtrributesList.push_back(resItemDescription);
            }
            if(atoi(mItemDescription->ut) >0)
            {
                resItemDescription = new ItemDescriptionAttributes;
                strcpy(resItemDescription->name,"res_toughness");
                sprintf(resItemDescription->value,"%s",mItemDescription->ut);
                mAtrributesList.push_back(resItemDescription);
            }
            if(atoi(mItemDescription->pe) >0)
            {
                resItemDescription = new ItemDescriptionAttributes;
                strcpy(resItemDescription->name,"res_potential_energy");
                sprintf(resItemDescription->value,"%s",mItemDescription->pe);
                mAtrributesList.push_back(resItemDescription);
            }

            gChatMessageLib->SendGetAuctionDetailsResponse(asynContainer, &mAtrributesList);

            it = mAtrributesList.begin();

            while(it != mAtrributesList.end())
            {
                SAFE_DELETE((*it));
                //it = AtrributesList.erase(it);
                it++;
            }

            mAtrributesList.clear();

            SAFE_DELETE(asynContainer->mItemDescription);
        }
        mDatabase->destroyDataBinding(binding);


        return;
    }

    case TRMQuery_GetAttributeDetails:
    {
        //now that we have the auctions main data we need to query for all the atrributes and their values
        DataBinding* binding = mDatabase->createDataBinding(2);

        binding->addField(DFT_string,offsetof(ItemDescriptionAttributes,name),32,0);
        binding->addField(DFT_string,offsetof(ItemDescriptionAttributes,value),32,1);

        AttributesList::iterator it = mAtrributesList.begin();

        uint64 count = result->getRowCount();

        for(uint16 i=0; i <count; i++)
        {
            ItemDescriptionAttributes* mItemDescription = new ItemDescriptionAttributes;
            result->getNextRow(binding,mItemDescription);
            mAtrributesList.push_back(mItemDescription);
        }

        gChatMessageLib->SendGetAuctionDetailsResponse(asynContainer, &mAtrributesList);

        it = mAtrributesList.begin();

        while(it != mAtrributesList.end())
        {
            ItemDescriptionAttributes* mItemDescription = (*it);
            it = mAtrributesList.erase(it);
            SAFE_DELETE(mItemDescription);
        }

        SAFE_DELETE(asynContainer->mItemDescription);
        mDatabase->destroyDataBinding(binding);
    }
    break;

    case TRMQuery_CancelAuction_BidderMail:
    {
        AuctionItem Item;

        DataBinding* binding = mDatabase->createDataBinding(3);
        binding->addField(DFT_uint64,offsetof(AuctionItem, BidderID), 8, 0);
        binding->addField(DFT_string,offsetof(AuctionItem, Name), 128, 1);
        binding->addField(DFT_uint32,offsetof(AuctionItem, HighProxy), 4, 2);

        uint64 count = result->getRowCount();
        if (count >= 1)	{
            //the Query actually only yields a result when we are dealing with an auction
            for(uint16 i=0; i <count; i++) {
                result->getNextRow(binding,&Item);
                gChatMessageLib->sendBidderCancelAuctionMail(asynContainer->mClient, player->getCharId(), Item.BidderID, Item.Name);

                //TODO
                //refund the bidders
            }
        }
        int8 sql[100];
        sprintf(sql, "SELECT %s.sf_CancelLiveAuction ('%" PRIu64 "')", mDatabase->galaxy(),  asynContainer->AuctionID);

        TradeManagerAsyncContainer* asyncContainer;
        asyncContainer = new TradeManagerAsyncContainer(TRMQuery_CancelAuction, asynContainer->mClient);

        asyncContainer->AuctionID = asynContainer->AuctionID;
        mDatabase->executeSqlAsync(this, asyncContainer, sql);
        
        mDatabase->destroyDataBinding(binding);
    }
    break;

    case TRMQuery_CancelAuction:
    {
        int8 ItemName[128];

        DataBinding* binding = mDatabase->createDataBinding(1);

        binding->addField(DFT_string, 0, 128);
        uint64 count = result->getRowCount();

        if (count == 1) {
            result->getNextRow(binding, &ItemName);
            gChatMessageLib->sendCanceLiveAuctionResponseMessage(asynContainer->mClient, 0, asynContainer->AuctionID);

            //send the relevant EMail
            gChatMessageLib->sendCancelAuctionMail(asynContainer->mClient, player->getCharId(), player->getCharId(), ItemName);
            mDatabase->destroyDataBinding(binding);
        }
        else
        {
            gChatMessageLib->sendCanceLiveAuctionResponseMessage(asynContainer->mClient, 1, asynContainer->AuctionID);
            LOG(INFO) << "TradeManager::TRMQuery_CancelAuction::Aucction not found : " << asynContainer->AuctionID;
        }
    }
    break;

    case TRMQuery_LoadBazaar:
    {
        DataBinding* binding = mDatabase->createDataBinding(4);
        binding->addField(DFT_uint64,offsetof(Bazaar, id), 8, 0);
        binding->addField(DFT_string,offsetof(Bazaar, string), 128, 1);
        binding->addField(DFT_uint32,offsetof(Bazaar, regionid), 4, 2);
        binding->addField(DFT_uint32,offsetof(Bazaar, planetid), 4, 3);

        uint64 count = result->getRowCount();

        for(uint64 i = 0; i < count; i++)
        {
            Bazaar* bazaar = new Bazaar();
            result->getNextRow(binding,bazaar);

            mBazaars.push_back(bazaar);
        }

        mBazaarCount += static_cast<uint32>(count);
        mBazaarsLoaded = true;
        mDatabase->destroyDataBinding(binding);
    }
    break;

    case TRMQuery_AuctionQuery:
    {
        //better check if theres a player???
        //all auctions that fit the search criteria are loaded into memory - the query
        //selects them based on the page count
        //Lists are assembled of every single seller, bazaar and of the auctions
        //every seller and bazaar name is only send once, regardlass how much
        //auctions they have

        DataBinding* binding = mDatabase->createDataBinding(18);
        binding->addField(DFT_uint64,offsetof(AuctionItem,ItemID),8,0);
        binding->addField(DFT_uint64,offsetof(AuctionItem,OwnerID),8,1);
        binding->addField(DFT_uint64,offsetof(AuctionItem,BazaarID),8,2);
        binding->addField(DFT_uint32,offsetof(AuctionItem,AuctionTyp),4,3);
        binding->addField(DFT_uint64,offsetof(AuctionItem,EndTime),8,4);
        binding->addField(DFT_uint32,offsetof(AuctionItem,Premium),4,5);
        binding->addField(DFT_uint32,offsetof(AuctionItem,Category),4,6);
        binding->addField(DFT_uint32,offsetof(AuctionItem,ItemTyp),4,7);
        binding->addField(DFT_uint32,offsetof(AuctionItem,Price),4,8);
        binding->addField(DFT_string,offsetof(AuctionItem,Name),128,9);
        binding->addField(DFT_string,offsetof(AuctionItem,Description), 1024, 10);
        binding->addField(DFT_uint16,offsetof(AuctionItem,RegionID), 2, 11);
        binding->addField(DFT_string,offsetof(AuctionItem,bidder_name), 32, 12);
        binding->addField(DFT_uint16,offsetof(AuctionItem,PlanetID), 2, 13);
        binding->addField(DFT_string,offsetof(AuctionItem,SellerName), 32 ,14);
        binding->addField(DFT_string,offsetof(AuctionItem,BazaarName), 128, 15);
        binding->addField(DFT_string,offsetof(AuctionItem,HighProxyRaw), 8, 16);
        binding->addField(DFT_string,offsetof(AuctionItem,HighBidRaw) ,8, 17);

        uint64 count = result->getRowCount();

        auction = new AuctionClass();

        AuctionItem AuctionTemp;
        //Not sure how to properly read this in. We perform a left join, because
        //the data is not existing when nobody has bid on the item yet

        strcpy(AuctionTemp.HighBidRaw,"0");
        strcpy(AuctionTemp.HighProxyRaw,"0");

        AuctionTemp.HighBid = 0;
        AuctionTemp.HighProxy = 0;
        for(uint64 i = 0; i < count; i++)
        {
            //assemble our string list
            result->getNextRow(binding,&AuctionTemp);
            AuctionTemp.HighBid = atoi(AuctionTemp.HighBidRaw);
            AuctionTemp.HighProxy = atoi(AuctionTemp.HighProxyRaw);
            auction->AddAuction(AuctionTemp);
        }
        //now that the lists are done we need to send the packet

        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opAuctionQueryHeadersResponseMessage);

        gMessageFactory->addUint32(asynContainer->BazaarPage);//
        gMessageFactory->addUint32(asynContainer->BazaarWindow );
        //total of unique Terminals and unique sellers per terminal
        //so here goes the total nr of strings
        gMessageFactory->addUint32(auction->getStringCount());
        ListStringList::iterator itL = auction->mListStringList.begin();
        //that are all bazaars, sellers and bidders
        while(itL != auction->mListStringList.end())
        {
            gMessageFactory->addString((*itL)->GetString());
            itL++;
        }

        //Nr of unique Auction Names (no auction name more than once)
        gMessageFactory->addUint32(auction->NameStringCount);

        BString s;
        NameStringList::iterator itD = auction->mNameStringList.begin();
        while(itD != auction->mNameStringList.end())
        {
            s = (*itD)->GetName();
            s.convert(BSTRType_Unicode16);
            gMessageFactory->addString(s);
            itD++;
        }

        //finally here the total Nr of auctions
        gMessageFactory->addUint32(auction->AuctionStringCount);
        AuctionStringList::iterator itA = auction->mAuctionStringList.begin();
        while(itA != auction->mAuctionStringList.end())
        {

            //Item/AuctionID
            gMessageFactory->addUint64((*itA)->GetAuctionID() );
            //ListID of the Auctions name
            gMessageFactory->addUint8(static_cast<uint8>((*itA)->GetNameListID()-1));

            //the Items Price
            gMessageFactory->addUint32((*itA)->GetPrice());

            //remaining time in seconds
            uint32 time = static_cast<uint32>((*itA)->GetTime()- (getGlobalTickCount()/1000));
            gMessageFactory->addUint32(time);

            //auction or instant??
            gMessageFactory->addUint8((*itA)->GetType());

            //List Id of the auctions bazaar string
            gMessageFactory->addUint16(static_cast<uint16>((*itA)->GetBazaarListID()-1));

            //Auction Owner ID
            gMessageFactory->addUint64((*itA)->GetOwnerID());

            //Auction Owner Namestring ID - first name is nr 1
            gMessageFactory->addUint16(static_cast<uint16>((*itA)->GetSellerListID()-1));

            //Category
            gMessageFactory->addUint32((*itA)->GetCategory());

            //listplace of the highbidder
            gMessageFactory->addUint16(static_cast<uint16>((*itA)->GetBidderListID()));


            gMessageFactory->addUint32((*itA)->GetBid());	// high bid My High Bid!!!!
            gMessageFactory->addUint32((*itA)->GetProxy());	// my Proxy
            gMessageFactory->addUint32((*itA)->GetBid());	// high bid My High Bid!!!!

            gMessageFactory->addUint32((*itA)->GetCategory());// item type for proper text reference


            gMessageFactory->addUint8(0);
            //Ok now heres our bitmask
            //1
            //2
            //4 = Premium
            //8 = shows Accept bid AND Withdraw sale on own auctions
            uint8 bitmap = 0;
            bitmap = (bitmap | 8);//set bit
            if (player->getCharId() == (*itA)->GetOwnerID()) {
                //bitmap = (bitmap | 8);//set bit 4
                if ((*itA)->GetType() == 2) {
                    bitmap = (bitmap ^ 8);//unset bit 4 when not for sale anymore
                }
            }
            if ((*itA)->GetPremium() == 1)
                bitmap = (bitmap | 4);//set bit 2;
            //bitmap = (bitmap | 2);//set bit


            //	bitmap = (bitmap | 1);//set bit


            gMessageFactory->addUint8(bitmap);//bitmask);
            gMessageFactory->addUint8(0);
            gMessageFactory->addUint8(0);
            gMessageFactory->addUint32(0);

            itA++;
        }

        gMessageFactory->addUint16(static_cast<uint16>(asynContainer->Itemsstart));

        uint32 pages = asynContainer->Itemsstart + static_cast<uint32>(count);
        if ((pages-asynContainer->Itemsstart) < 100) {
            pages = 0;
        }
        gMessageFactory->addUint16(static_cast<uint16>(pages));

        gMessageFactory->addUint32(0);
        gMessageFactory->addUint32(0);
        gMessageFactory->addUint32(0);
        gMessageFactory->addUint32(0);

        gMessageFactory->addUint32(0);
        Message* newMessage = gMessageFactory->EndMessage();
        asynContainer->mClient->SendChannelA(newMessage, asynContainer->mClient->getAccountId(),  CR_Client, 6);

        delete(auction);
        mDatabase->destroyDataBinding(binding);

    }
    break;

    case TRMQuery_ProcessBidAuction:
    {

        DataBinding* binding = mDatabase->createDataBinding(3);
        binding->addField(DFT_uint32,offsetof(AuctionItem,HighProxy),4,0);
        binding->addField(DFT_uint32,offsetof(AuctionItem,HighBid),4,1);
        binding->addField(DFT_uint64,offsetof(AuctionItem,BidderID),8,2);

        AuctionItem AuctionTemp;

        uint64 count = result->getRowCount();
        asynContainer->AuctionTemp->HighBid = 0 ;
        asynContainer->AuctionTemp->HighProxy = 0;
        asynContainer->AuctionTemp->BidderID = 0;
        if(count == 1) {
            //there is a highbidder ... - we have the highbid and proxy
            result->getNextRow(binding,&AuctionTemp);
            //proxy bid and id of current high bidder
            asynContainer->AuctionTemp->HighBid = AuctionTemp.HighBid;
            asynContainer->AuctionTemp->HighProxy = AuctionTemp.HighProxy;
            asynContainer->AuctionTemp->BidderID = AuctionTemp.BidderID;
        }
        processAuctionBid(asynContainer,player);

    }
    break;

    case TRMQuery_ExpiredListing:
    {
        int8 sql[390];

        //here we get EVERY auction and every instant which has run out of time
        //we need to set the new Owners for the sold auctions
        //send the respective EMails
        //refund money where necessary

        DataBinding* binding = mDatabase->createDataBinding(9);
        binding->addField(DFT_uint64,offsetof(AuctionItem,ItemID),8,0);
        binding->addField(DFT_uint64,offsetof(AuctionItem,OwnerID),8,1);
        binding->addField(DFT_uint32,offsetof(AuctionItem,AuctionTyp),4,2);
        binding->addField(DFT_uint32,offsetof(AuctionItem,Price),4,3);
        binding->addField(DFT_string,offsetof(AuctionItem,Name),128,4);
        binding->addField(DFT_string,offsetof(AuctionItem,bidder_name),32,5);
        binding->addField(DFT_uint64,offsetof(AuctionItem,BazaarID),8,6);
        binding->addField(DFT_string,offsetof(AuctionItem,Owner),32,7);
        binding->addField(DFT_raw,offsetof(AuctionItem,BidderIDRaw),32,8);


        AuctionItem* auctionTemp;
        uint32 count = static_cast<uint32>(result->getRowCount());

        for(uint32 i = 0; i < count; i++)
        {
            auctionTemp = new(AuctionItem);
            //an auction with no bidder will have the bidder_id as NULL and the bidder name empty
            strcpy(auctionTemp->bidder_name,"");
            strcpy(auctionTemp->BidderIDRaw,"");
            auctionTemp->BidderID=0;

            result->getNextRow(binding,auctionTemp);

            using boost::lexical_cast;
            using boost::bad_lexical_cast;

            try
            {
                auctionTemp->BidderID=boost::lexical_cast<uint64>(auctionTemp->BidderIDRaw);
            }
            catch(bad_lexical_cast &)
            {
                auctionTemp->BidderID	= 0;
            }

            processAuctionEMails(auctionTemp);
            mAuction.push_back(auctionTemp);
        }

        AuctionList::iterator it = mAuction.begin();

        while(it != mAuction.end())
        {
            auctionTemp = (*it);

            //do we need to refund money???
            if(auctionTemp->AuctionTyp == TRMVendor_Auction)
            {
                //read in the bidhistory and iterate through the bidders of the auction
                //refund money to everybody but the winning bidder

                TradeManagerAsyncContainer* asyncContainer;
                sprintf(sql,"SELECT cbh.proxy_bid, c.id  FROM %s.characters AS c INNER JOIN %s.commerce_bidhistory AS cbh ON (c.firstname = cbh.bidder_name) WHERE cbh.auction_id = %" PRIu64 "",mDatabase->galaxy(),mDatabase->galaxy(),auctionTemp->ItemID);
                asyncContainer = new TradeManagerAsyncContainer(TRMQuery_ProcessAuctionRefund,NULL);
                asyncContainer->AuctionTemp = auctionTemp;
                //mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
                //gLogger->log(LogManager::DEBUG, "SQL :: %s", sql); // SQL Debug Log

                //WE NEED THIS WHILE WE HAVE THE ABOVE LINE COMMENTED OUT!!!

				// why the heck is it commented out ?
                delete asyncContainer;
            }

            //Update the new owner in case our auction got sold
            if((auctionTemp->AuctionTyp == TRMVendor_Auction) && (auctionTemp->BidderID != 0))
            {
                //set the new Owner
                TradeManagerAsyncContainer* asyncContainer;

                sprintf(sql,"UPDATE %s.commerce_auction SET owner_id = %" PRIu64 ", bidder_name = '' WHERE auction_id = %" PRIu64 " ",mDatabase->galaxy(),auctionTemp->BidderID,auctionTemp->ItemID);
                asyncContainer = new TradeManagerAsyncContainer(TRMQuery_NULL,NULL);
                mDatabase->executeSqlAsync(this,asyncContainer,sql);
                
            }

            it++;
        }

        //now move the auctions in their proper holding areas or delete the ones which have expired their holding date
        TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_NULL,NULL);

        mDatabase->executeProcedureAsync(this, asyncContainer, "CALL %s.sp_CommerceFindExpiredListing();",mDatabase->galaxy());
        

    }
    break;

    case TRMQuery_ProcessAuctionRefund:
    {
        DataBinding* binding = mDatabase->createDataBinding(2);
        binding->addField(DFT_uint32,offsetof(AuctionItem,MyProxy),4,0);
        binding->addField(DFT_uint64,offsetof(AuctionItem,BidderID),8,1);

        AuctionItem* AuctionTemp;
        uint64 count = result->getRowCount();

        //iterate through all the bids and refund them
        //Mind the winning bidder though
        int8 sql[390];
        //printf("\n %u bidders to refund minus one buyer\n", count);
        for(uint64 i = 0; i < count; i++)
        {
            AuctionTemp = new(AuctionItem);
            result->getNextRow(binding,AuctionTemp);
            // printf("\not not the winner - so refund\n");
            // let zone handle the refund problem for logged ins in the Object
            // sometime in the future

            // just update db here - only add credits so this should be rather relaxed
            if(AuctionTemp->BidderID != asynContainer->AuctionTemp->OwnerID) {
                TradeManagerAsyncContainer* asyncContainer;

                sprintf(sql,"UPDATE %s.banks SET credits=credits+%"PRId32" WHERE id=%" PRIu64 "",mDatabase->galaxy(),asynContainer->MyProxy, AuctionTemp->BidderID+4);
                asyncContainer = new TradeManagerAsyncContainer(TRMQuery_NULL,NULL);
                mDatabase->executeSqlAsync(this,asyncContainer,sql);
                
            }
            SAFE_DELETE(AuctionTemp);

        }

        //now delete the Auctions Bidhistory
        TradeManagerAsyncContainer* asyncContainer;

        sprintf(sql,"DELETE FROM %s.commerce_bidhistory WHERE auction_id = '%" PRIu64 "' ",mDatabase->galaxy(),asynContainer->AuctionTemp->ItemID);
        asyncContainer = new TradeManagerAsyncContainer(TRMQuery_NULL,NULL);
        mDatabase->executeSqlAsync(this,asyncContainer,sql);
        

        SAFE_DELETE(asyncContainer->AuctionTemp);
    }
    break;

    case TRMQuery_BidAuction:
    {
        DataBinding* binding = mDatabase->createDataBinding(10);
        binding->addField(DFT_uint64,offsetof(AuctionItem,ItemID),8,0);
        binding->addField(DFT_uint64,offsetof(AuctionItem,OwnerID),8,1);
        binding->addField(DFT_uint32,offsetof(AuctionItem,AuctionTyp),4,2);
        binding->addField(DFT_uint32,offsetof(AuctionItem,Category),4,3);
        binding->addField(DFT_uint32,offsetof(AuctionItem,Price),4,4);
        binding->addField(DFT_uint32,offsetof(AuctionItem,ItemTyp),4,5);
        binding->addField(DFT_string,offsetof(AuctionItem,Name),128,6);
        binding->addField(DFT_uint64,offsetof(AuctionItem,BazaarID),8,7);
        binding->addField(DFT_string,offsetof(AuctionItem,bidder_name),32,8);
        binding->addField(DFT_string,offsetof(AuctionItem,Owner),32,9);


        AuctionItem* AuctionTemp = new(AuctionItem);
        result->getNextRow(binding,AuctionTemp);

        mDatabase->destroyDataBinding(binding);

        Player* player = gChatManager->getPlayerbyId(asynContainer->BuyerID);

        Bazaar* bazaarInfo = getBazaarInfo(AuctionTemp->BazaarID);

        //is it an auction or an instant
        if (AuctionTemp->AuctionTyp == 0)
        {
            //now get additional Information - do we have an actual high bidder? - Who is it? and
            //what is the highbid and the highproxy
            //_processAuctionBid(AuctionTemp,asynContainer,player);
            TradeManagerAsyncContainer* asyncContainer;

            int8 name[40],*sqlPointer;
            sqlPointer = name;
            sqlPointer += mDatabase->escapeString(name,AuctionTemp->bidder_name,strlen(AuctionTemp->bidder_name));
            *sqlPointer++ = '\0';

            int8 sql[390];
            sprintf(sql,"SELECT cbh.proxy_bid, cbh.max_bid, c.id FROM %s.characters AS c INNER JOIN %s.commerce_bidhistory AS cbh ON c.firstname = '%s' WHERE cbh.auction_id = %" PRIu64 " AND cbh.bidder_name = '",mDatabase->galaxy(),mDatabase->galaxy(),name,AuctionTemp->ItemID);

            sqlPointer = sql + strlen(sql);
            sqlPointer += mDatabase->escapeString(sqlPointer,AuctionTemp->bidder_name, strlen(AuctionTemp->bidder_name));
            *sqlPointer++ = '\'';
            *sqlPointer++ = '\0';

            asyncContainer = new TradeManagerAsyncContainer(TRMQuery_ProcessBidAuction,asynContainer->mClient);
            asyncContainer->MyBid = asynContainer->MyBid;
            asyncContainer->MyProxy = asynContainer->MyProxy;
            asyncContainer->AuctionID = asynContainer->AuctionID;

            asyncContainer->BuyerID = player->getCharId();
            asyncContainer->BazaarID = 0;//
            asyncContainer->AuctionTemp = AuctionTemp;
            mDatabase->executeSqlAsync(this,asyncContainer,sql);
            
            break;
        }

        if (AuctionTemp->AuctionTyp == 1)
        {
            //instant
            //the client checks for the money so we will check for cheating later in the zoneserver
            //the client sends a retrieve message hereafter so this is only to buy the item
            //flag it as bought, change the owner and the remaining Time
            //retrieve is send by client after that

            uint32 time = (3600*24*30)+(static_cast<uint32>(getGlobalTickCount())/1000);

            //let the zoneserver deal with the transaction and send the relevant Emails
            gChatMessageLib->sendBazaarTransactionMessage(asynContainer->mClient, *AuctionTemp, player->getCharId(), time, player, bazaarInfo);

            SAFE_DELETE(AuctionTemp);

        }
        break;
    }

    break;
    case TRMQuery_NULL:
    {
        break;
    }

    case TRMQuery_LoadGlobalTick:
    {
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint64,offsetof(TradeManagerChatHandler,mGlobalTickCount),8,0);
        result->getNextRow(binding,this);
        mDatabase->destroyDataBinding(binding);
    }
    break;

    default:
        break;
    }
    SAFE_DELETE(asynContainer);

    if(mBazaarsLoaded)
    {
        mBazaarsLoaded = false;
        LOG(WARNING) << "Loaded bazaars";
    }
}

//=======================================================================================================================

void TradeManagerChatHandler::processAuctionEMails(AuctionItem* auctionTemp)
{
    Bazaar* mBazaarInfo = getBazaarInfo(auctionTemp->BazaarID);

    char *Token;
    char separation[] = ".#,";
    BString mString;
    mString = mBazaarInfo->string;

    Token = strtok(mString.getRawData(),separation); //
    BString planet = BString(Token);
    Token = strtok( NULL,separation);
    BString region = BString(Token);

    Token = strtok( NULL,separation);
    //thats now the terminal id part
    Token = strtok( NULL,separation);
    //x/y
    Token = strtok( NULL,separation);
    float x = float(atoi(Token));

    Token = strtok( NULL,separation);
    float y = float(atoi(Token));



    if(auctionTemp->AuctionTyp == TRMVendor_Instant)
    {
        //any instant getting here has not been purchased
        atMacroString* aMS = new atMacroString();

        aMS->addMBstf("auction","seller_fail");
        aMS->addTO(BString(auctionTemp->Name));
        aMS->addTextModule();

        Mail* mail = new Mail();
        mail->setSender(BString("auctioner"));
        mail->setSubject(BString("@auction:subject_auction_unsuccessful"));
        mail->setText(BString(""));
        mail->setStatus(MailStatus_New);
        mail->setTime(static_cast<uint32>(time(NULL)));
        mail->setAttachments(aMS->assemble());
        //attachment is already initialized with an empty string
        //mail->setAttachments(attachment);

        gChatManager->sendSystemMailMessage(mail,auctionTemp->OwnerID);
        
    }

    if(auctionTemp->AuctionTyp == TRMVendor_Auction)
    {
        //has it been sold??
        if(auctionTemp->BidderID == 0)
        {
            // no it has not been bought

            atMacroString* aMS = new atMacroString();

            aMS->addMBstf("auction","seller_fail");
            aMS->addTO(BString(auctionTemp->Name));
            aMS->addTextModule();

            Mail* mail = new Mail();
            mail->setSender(BString("auctioner"));
            mail->setSubject(BString("@auction:subject_auction_unsuccessful"));
            mail->setText(BString(""));
            mail->setStatus(MailStatus_New);
            mail->setTime(static_cast<uint32>(time(NULL)));
            mail->setAttachments(aMS->assemble());

            gChatManager->sendSystemMailMessage(mail,auctionTemp->OwnerID);

        }
        else
        {
            //EMail to the seller
            atMacroString* aMS = new atMacroString();

            aMS->addMBstf("auction","seller_success");
            aMS->addTO(BString(auctionTemp->Name));
            aMS->addTT(BString(auctionTemp->bidder_name));
            aMS->addDI(auctionTemp->Price);
            aMS->addTextModule();
            aMS->addMBstf("auction","seller_success_location");
            aMS->addTT(region);
            aMS->addTO(planet);
            aMS->addTextModule();

            Mail* mail = new Mail();
            mail->setSender(BString("auctioner"));
            mail->setSubject(BString("@auction:subject_auction_seller"));
            mail->setText(BString(""));
            mail->setStatus(MailStatus_New);
            mail->setTime(static_cast<uint32>(time(NULL)));
            mail->setAttachments(aMS->assemble());

            gChatManager->sendSystemMailMessage(mail,auctionTemp->OwnerID);

            //EMail to the buyer
            aMS = new atMacroString();

            aMS->addMBstf("auction","buyer_success");
            aMS->addTO(BString(auctionTemp->Name));
            aMS->addTT(BString(auctionTemp->Owner));
            aMS->addDI(auctionTemp->Price);
            aMS->addTextModule();

            aMS->addMBstf("auction","buyer_success_location");
            aMS->addTT(region);
            aMS->addTO(planet);
            aMS->addTextModule();

            aMS->setPlanetString(planet);
            aMS->setWP(x,y,0,auctionTemp->Name);
            aMS->addWaypoint();

            mail= new Mail();
            mail->setSender(BString("auctioner"));
            mail->setSubject(BString("@auction:subject_auction_buyer"));
            mail->setText(BString(""));
            mail->setStatus(MailStatus_New);
            mail->setTime(static_cast<uint32>(time(NULL)));
            mail->setAttachments(aMS->assemble());

            gChatManager->sendSystemMailMessage(mail,auctionTemp->BidderID);
        }
    }

    if(auctionTemp->AuctionTyp == TRMVendor_Ended)
    {
        //auction is ended and stored longer than the time limit so it gets deleted
        atMacroString* aMS = new atMacroString();

        aMS->addMBstf("auction","item_expired");
        aMS->addTO(BString(auctionTemp->Name));
        aMS->addTextModule();

        Mail* mail= new Mail();
        mail->setSender(BString("auctioner"));
        mail->setSubject(BString("@auction:subject_auction_item_expired"));
        mail->setText(BString(""));
        mail->setStatus(MailStatus_New);
        mail->setTime(static_cast<uint32>(time(NULL)));
        mail->setAttachments(aMS->assemble());

        gChatManager->sendSystemMailMessage(mail,auctionTemp->BidderID);
    }
}

void TradeManagerChatHandler::processAuctionBid(TradeManagerAsyncContainer* asynContainer, Player* player)
{
//auction ...
    int8 sql[250], PlayerName[40],*sqlPointer;

    sqlPointer = PlayerName;
    sqlPointer += mDatabase->escapeString(sqlPointer,player->getName().getAnsi(),player->getName().getLength());
    *sqlPointer++ = '\0';

    //Check the current add depending on the price

    uint32 TheBid;
    uint32 TheProxy;
    // check if there is a valid bid
    //a) the bid amount

    if (asynContainer->MyBid > mBazaarMaxBid) {
        asynContainer->MyBid = mBazaarMaxBid;
    }

    //basically the client checks for invalid bids
    //so if there is a cheater, like someone with a dummyclient we should log that at this point
    if (asynContainer->MyProxy > mBazaarMaxBid)
    {
        asynContainer->MyProxy = mBazaarMaxBid;
        //invalid bids will be eaten by the client
        //so if we are here we have a cheater -
        // TODO is the 20000 really clientchecked for AuctionProxies?????
    }
    //are we bidding again while already being high bidder ???
    if (asynContainer->AuctionTemp->BidderID == player->getCharId())
    {
        //yes AND we have a new high proxy
        if (asynContainer->MyProxy > asynContainer->AuctionTemp->HighProxy)
        {

            //just update the high proxy
            sprintf(sql,"UPDATE %s.commerce_bidhistory SET proxy_bid = '%"PRIu32"'WHERE auction_id = '%" PRIu64 "' AND bidder_name = '%s'",mDatabase->galaxy(), asynContainer->MyProxy, asynContainer->AuctionTemp->ItemID, PlayerName);

            TradeManagerAsyncContainer* asyncContainer;
            asyncContainer = new TradeManagerAsyncContainer(TRMQuery_ACKRetrieval,asynContainer->mClient);

            asyncContainer->AuctionID = asynContainer->AuctionTemp->ItemID;
            mDatabase->executeSqlAsync(this,asyncContainer,sql);
            

            return;
        }
        else
        {
            // we are bidding on our own auction but our proxy is the same
            //dont bid ourselfes unnecessarily up
            gChatMessageLib->sendBidAuctionResponse(asynContainer->mClient,0,asynContainer->AuctionID);
            return;
        }
    }


    //are we bidding enough to be the high bidder????
    if (asynContainer->MyProxy > asynContainer->AuctionTemp->HighProxy) {
        //we will be the new high bidder

        // Inform the client
        //gChatMessageLib->sendBidAuctionResponse(asynContainer->mClient,0, asynContainer->AuctionTemp->ItemID);

        //now send the Mail to the outbid bidder
        //unless of course this is the first bid
        if (asynContainer->AuctionTemp->BidderID != 0)
        {
            gChatMessageLib->sendAuctionOutbidMail(asynContainer->mClient, asynContainer->AuctionTemp->BidderID,asynContainer->AuctionTemp->BidderID, asynContainer->AuctionTemp->Name);
        }

        //determine the new high Bid Proxy and bidder
        TheBid = asynContainer->AuctionTemp->HighProxy+1;

        //make sure the Bid resembles the items price should this be the first bid
        if(TheBid < asynContainer->AuctionTemp->Price) {
            TheBid = asynContainer->AuctionTemp->Price;
        }

        TheProxy = asynContainer->MyProxy;

    }
    else
    {
        //nope we didnt bid high enough :(
        //gChatMessageLib->sendBidAuctionResponse(asynContainer->mClient,0, asynContainer->AuctionTemp->ItemID);

        TheBid = asynContainer->MyProxy+1;
        if (TheBid > asynContainer->AuctionTemp->HighProxy)
            TheBid = asynContainer->AuctionTemp->HighProxy;

        TheProxy = asynContainer->AuctionTemp->HighProxy;
        int8 BidderName[40];
        strcpy(BidderName,asynContainer->AuctionTemp->bidder_name);

        // what do we do if this is our first bid and we are NOT the high bidder?
        // sf_BidAuction only updates the bid of the high bidder

        sprintf(sql, "SELECT %s.sf_BidUpdate ('%" PRIu64 "','%"PRIu32"','%"PRIu32"','%s')", mDatabase->galaxy(),  asynContainer->AuctionTemp->ItemID, asynContainer->MyBid, asynContainer->MyProxy, PlayerName);
        TradeManagerAsyncContainer* asyncContainer;
        asyncContainer = new TradeManagerAsyncContainer(TRMQuery_ACKRetrieval, asynContainer->mClient);
        mDatabase->executeSqlAsync(this, asyncContainer, sql);
        

    }

    sprintf(sql, "SELECT %s.sf_BidAuction ('%" PRIu64 "','%"PRIu32"','%"PRIu32"','%s')", mDatabase->galaxy(),  asynContainer->AuctionTemp->ItemID, TheBid, TheProxy, PlayerName);
    TradeManagerAsyncContainer* asyncContainer;
    asyncContainer = new TradeManagerAsyncContainer(TRMQuery_ACKRetrieval,asynContainer->mClient);

    asyncContainer->AuctionID = asynContainer->AuctionTemp->ItemID;
    mDatabase->executeSqlAsync(this,asyncContainer,sql);
    
}

//=======================================================================================================================
void TradeManagerChatHandler::ProcessCreateAuction(Message* message,DispatchClient* client)
{
    //we got a packet from zone concerning a started auction...
    Player* player;
    PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

    if(accIt != mPlayerAccountMap.end())
        player = (*accIt).second;
    else
    {
        return;
    }

    /*uint64	PlayerID	= */message->getUint64();
    uint64	TerminalID	= message->getUint64();
    uint64	ItemID		= message->getUint64();
    uint32	category	= message->getUint32();
    uint32	Time		= message->getUint32();

    uint64 time = (getGlobalTickCount()/1000)+Time;
    uint32 RegionID = TerminalRegionbyID(TerminalID);

    //check if we already have a valid category like we have when dealing with resources
    int8 Query[4096];		//depending on our Auction comments this can become long
    if (category != 0)
    {
        sprintf(Query," UPDATE %s.commerce_auction SET start = '%" PRIu64 "', region_id = '%"PRIu32"', planet_id = '%"PRIu32"',category = '%"PRIu32"' WHERE auction_id = '%" PRIu64 "'",mDatabase->galaxy(), time, RegionID, player->getPlanetId(),category,ItemID);
    }
    else
    {
        sprintf(Query," UPDATE %s.commerce_auction ca inner join %s.items i ON (i.id = ca.auction_id) inner join %s.item_types it ON (it.id = i.item_type) SET ca.start = '%" PRIu64 "' , ca.region_id = '%"PRIu32"', ca.planet_id = '%"PRIu32"', ca.category = it.bazaar_category  WHERE ca.auction_id = '%" PRIu64 "'", mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),time, RegionID, player->getPlanetId(),ItemID);
    }


    TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_CreateAuction,client);
    asyncContainer->AuctionID = ItemID;
    mDatabase->executeSqlAsync(this,asyncContainer,Query);
    
}

//=======================================================================================================================
void TradeManagerChatHandler::processRetrieveAuctionItemMessage(Message* message,DispatchClient* client)
{
    TradeManagerAsyncContainer* asyncContainer;
    PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

    if(accIt != mPlayerAccountMap.end()) {
		return;
    }

    // squeeze our Packet for all usefull information
    // ID of the Auction that gets retrieved

    uint64	ItemID		= message->getUint64();
    uint64	TerminalID	= message->getUint64();

    int8 sql[200];
    sprintf(sql,"SELECT auction_id, bazaar_id, itemtype FROM %s.commerce_auction c  WHERE c.auction_id = %" PRIu64 "",mDatabase->galaxy(),mDatabase->galaxy(), ItemID);
    asyncContainer = new TradeManagerAsyncContainer(TRMQuery_RetrieveAuction, client);
    asyncContainer->BazaarID = TerminalID;
    mDatabase->executeSqlAsync(this, asyncContainer, sql);
    
}

//=======================================================================================================================
void TradeManagerChatHandler::processBidAuctionMessage(Message* message,DispatchClient* client)
{
    TradeManagerAsyncContainer* asyncContainer;

    Player* player;
    PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

    if(accIt != mPlayerAccountMap.end())
        player = (*accIt).second;
    else
    {
		return;
    }

    //ID of the Auction that gets retrieved
    uint64	ItemID	= message->getUint64();
    uint32	MyBid	= message->getUint32();
    uint32	MyProxy	= message->getUint32();

    // 1 We have to Query the auction to find out If its an auction or an instant sale
    //

    int8 sql[490];
    //auction_bidhistory fields will be NULL when we have no bids

    sprintf(sql,"SELECT ca.auction_id, ca.owner_id, ca.type, ca.category, ca.price, ca.itemtype, ca.name, ca.bazaar_id, ca.bidder_name, c.firstname FROM %s.characters AS c INNER JOIN %s.commerce_auction AS ca ON (ca.owner_id = c.id) where ca.auction_id = %" PRIu64 "",mDatabase->galaxy(),mDatabase->galaxy(),ItemID);

    asyncContainer = new TradeManagerAsyncContainer(TRMQuery_BidAuction,client);
    asyncContainer->MyBid = MyBid;
    asyncContainer->MyProxy = MyProxy;
    asyncContainer->AuctionID = ItemID;
    asyncContainer->BuyerID = player->getCharId();

    Bazaar* bazaar = player->getBazaar();
    if(bazaar)
    {
        asyncContainer->BazaarID = bazaar->id;//
    }
    else
        asyncContainer->BazaarID = 0;//

    mDatabase->executeSqlAsync(this,asyncContainer,sql);
    
    // client checks if we have enough money
    // cheaters (bot modified client )will be flagged in the zoneserver
}

//=======================================================================================================================
void TradeManagerChatHandler::processCancelLiveAuctionMessage(Message* message,DispatchClient* client)
{
    TradeManagerAsyncContainer* asyncContainer;

    PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

    if(accIt != mPlayerAccountMap.end()) {
		return;
    }

    //ID of the Auction that gets canceled
    uint64	ItemID		= message->getUint64();

    //get all the bids on the auction and refunf the affected players
    //send the EMails
    int8 sql[300];
    sprintf(sql,"SELECT ch.id, ca.name, cbh.proxy_bid FROM %s.commerce_auction AS ca INNER JOIN %s.commerce_bidhistory AS cbh ON (cbh.bidder_name = ca.bidder_name)  INNER JOIN %s.characters AS ch ON (cbh.bidder_name = ch.firstname)  WHERE ca.auction_id = %" PRIu64 "",mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),ItemID);
    asyncContainer = new TradeManagerAsyncContainer(TRMQuery_CancelAuction_BidderMail, client);
    asyncContainer->AuctionID = ItemID;
    mDatabase->executeSqlAsync(this,asyncContainer,sql);
    
}

//=======================================================================================================================
void TradeManagerChatHandler::processGetAuctionDetails(Message* message,DispatchClient* client)
{
    TradeManagerAsyncContainer* asyncContainer;

    PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

    if(accIt != mPlayerAccountMap.end()) {
		return;
    }

    //the ID of the Auction we want to learn more about
    uint64 AuctionID = message->getUint64();

    //we'll need our item description, the iff data and the rest will be done by the items object
    asyncContainer = new TradeManagerAsyncContainer(TRMQuery_GetDetails,client);

    mDatabase->executeProcedureAsync(this, asyncContainer, "CALL %s.sp_BazaarAuctionDetailsGet(%" PRIu64 ");", mDatabase->galaxy(),  AuctionID);
    
}

//=======================================================================================================================
void TradeManagerChatHandler::processHandleopAuctionQueryHeadersMessage(Message* message,DispatchClient* client)
{
    TradeManagerAsyncContainer* asyncContainer;

    //uint64 time = (getGlobalTickCount()/1000);

    int8 sql[2024];
    sprintf(sql,"SELECT c.auction_id, owner_id, c.bazaar_id, type, start, premium, category, itemtype, price, name, description, c.region_id, c.bidder_name, c.planet_id, firstname, bazaar_string, cbh.proxy_bid, cbh.max_bid FROM %s.commerce_auction c INNER JOIN %s.characters ch on (c.owner_id = ch.id) INNER join %s.commerce_bazaar cb ON (cb.bazaar_id = c.bazaar_id) left join %s.commerce_bidhistory cbh ON (cbh.bidder_name = c.bidder_name AND cbh.auction_id = c.auction_id) AND c.owner_id = ch.id  WHERE",mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy());

    Player* player;
    PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

    if(accIt != mPlayerAccountMap.end())
        player = (*accIt).second;
    else
    {
		return;
    }

    //squeeze our Packet for all usefull information

    Query query;
    query.Region = message->getUint32();
    query.UpdateCounter = message->getUint32();
    query.Windowtype = message->getUint32();
    query.Category = message->getUint32();
    query.ItemTyp = message->getUint32();
    //query.searchstring;
    query.searchstring.setType(BSTRType_Unicode16);
    message->getStringUnicode16(query.searchstring);
    query.unknown = message->getUint32();
    query.minprice = message->getUint32();
    query.maxprice = message->getUint32();
    query.entrancefee = message->getUint8();
    query.vendorID = message->getUint64();
    query.unknown2 = message->getUint8();
    query.start = message->getUint16();//nr of 1st auction to show


    // build our db Query

    // the region to display

    switch(query.Region)
    {
    case TRMVendor:
    {
        sprintf(query.RegionQuery," (c.bazaar_id = %" PRIu64 ")",query.vendorID);

    }
    break;

    case TRMRegion:
    {
        //for now Planet
        sprintf(query.RegionQuery," (c.region_id = %"PRIu32")",TerminalRegionbyID(query.vendorID));
    }
    break;

    case TRMPlanet:
    {
        sprintf(query.RegionQuery," (c.planet_id = %"PRIu32")",player->getPlanetId());

    }
    break;

    case TRMGalaxy:
    {
        sprintf(query.RegionQuery," (c.planet_id >= 0)");

    }
    default:
        sprintf(query.RegionQuery," ");
    }

    //need new table for bidding ??

    // the kind of "hopper" to display auction hopper or stockroom or ownauctions or offers

    switch (query.Windowtype)
    {
    case TRMVendor_AllAuctions: //open up bazaar and look at it without a category
    {
        sprintf(query.WindowQuery," ((c.type = %"PRIu32") or (c.type = %"PRIu32"))", TRMVendor_Auction, TRMVendor_Instant);

    }
    break;

    case TRMVendor_MySales: //what Im selling at the bazaar
    {
        sprintf(query.WindowQuery," ((c.type = %"PRIu32") or (c.type = %"PRIu32"))AND", TRMVendor_Auction, TRMVendor_Instant);
        int8 tmp[128];
        sprintf(tmp," (c.owner_id = %" PRIu64 ")",player->getCharId());
        strcat(query.WindowQuery,tmp);

    }
    break;

    case TRMVendor_MyBids: //auctions Im bidding on currently at the bazaar
    {
        sprintf(query.WindowQuery," ((c.type = %"PRIu32") or (c.type = %"PRIu32")) AND",TRMVendor_Auction,TRMVendor_Instant);
        int8 tmp[128],end[128],*sqlPointer;
        sprintf(tmp," (cbh.bidder_name = '");
        sqlPointer = tmp + strlen(tmp);
        sqlPointer += mDatabase->escapeString(sqlPointer,player->getName().getAnsi(),player->getName().getLength());
        *sqlPointer++ = '\'';
        *sqlPointer++ = '\0';
        strcat(query.WindowQuery,tmp);
        sprintf(end," )");
        strcat(query.WindowQuery,end);
        sprintf(sql,"SELECT c.auction_id, owner_id, c.bazaar_id, type, start, premium, category, itemtype, price, name, description, c.region_id, c.bidder_name, c.planet_id, firstname, bazaar_string, cbh.proxy_bid, cbh.max_bid FROM %s.commerce_auction c INNER JOIN %s.characters ch on (c.owner_id = ch.id) INNER join %s.commerce_bazaar cb ON (cb.bazaar_id = c.bazaar_id) inner join %s.commerce_bidhistory cbh ON (cbh.auction_id = c.auction_id) AND c.owner_id = ch.id WHERE",mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy());

    }
    break;
    case TRMVendor_AvailableItems:
    {
        sprintf(query.WindowQuery," (c.type = %"PRIu32") AND (c.owner_id = %" PRIu64 ") ",TRMVendor_Ended,player->getCharId());


    }
    break;
    case TRMVendor_Offers:
    {
        sprintf(query.WindowQuery," (c.type = %"PRIu32") AND (c.bidder_name = %s) AND (c.bazaar_id = %" PRIu64 ")",TRMVendor_Offer,player->getName().getAnsi(),query.vendorID);


    }
    break;
    case TRMVendor_ForSale:
    {
        sprintf(query.WindowQuery," ((c.type = %"PRIu32") or (c.type = %"PRIu32")) AND",TRMVendor_Auction,TRMVendor_Instant);
        int8 tmp[128],end[128],*sqlPointer;
        sprintf(tmp," (c.bidder_name = '");
        sqlPointer = tmp + strlen(tmp);
        sqlPointer += mDatabase->escapeString(sqlPointer,player->getName().getAnsi(),player->getName().getLength());
        *sqlPointer++ = '\'';
        *sqlPointer++ = '\0';
        strcat(query.WindowQuery,tmp);
        sprintf(end," ) AND");
        strcat(query.WindowQuery,end);

        sprintf(tmp," (c.bazaar_id = %" PRIu64 ")",query.vendorID);
        strcat(query.WindowQuery,tmp);
    }
    break;
    default:
        sprintf(query.WindowQuery," ");
    }

    uint32 category;
    category = query.Category;
    //Were looking for a main category

    if (category != 0)
    {
        if ((category << 24) == 0)
        {
            sprintf(query.CategoryQuery," ((c.category >> 8) = %"PRIu32")",(query.Category >> 8));

        }
        else
        {
            //were looking for a subcategory
            sprintf(query.CategoryQuery," ((c.category) = %"PRIu32")",query.Category );
        }
    }
    else sprintf(query.CategoryQuery," ");

    if (query.ItemTyp != 0) {
        sprintf(query.ItemTypQuery," (c.itemtype = %"PRIu32")",query.ItemTyp);
    }
    else
        sprintf(query.ItemTypQuery," ");

    int8 Limit[64];
    uint32 StopTime;
    StopTime = (static_cast<uint32>(getGlobalTickCount()) / 1000);
    sprintf(Limit," AND (c.start > %"PRIu32") LIMIT %u, %u", StopTime,query.start, query.start+100);

    //region or bazaar id

    strcat(sql,query.RegionQuery); //
    bool add = false;

    if (strcmp(query.RegionQuery," ") != 0)
        add = true;

    if (strcmp(query.WindowQuery, " ") != 0) {
        if (add == true)
            strcat(sql," AND ");
        add = true;
    }
    strcat(sql,query.WindowQuery);

    if (strcmp(query.CategoryQuery, " ") !=0 ) {
        if (add == true)
            strcat(sql," AND ");
        add = true;
    }

    strcat(sql,query.CategoryQuery); //

    if (strcmp(query.ItemTypQuery, " ") !=0 ) {
        if (add == true)
            strcat(sql," AND ");
        add = true;
    }

    strcat(sql,query.ItemTypQuery); //
    strcat(sql,Limit); //

    asyncContainer = new TradeManagerAsyncContainer(TRMQuery_AuctionQuery,client);
    div_t d;
    d = div(query.start,100);
    asyncContainer->BazaarPage = d.quot+1 ;
    asyncContainer->BazaarWindow = query.Windowtype;
    asyncContainer->Itemsstart = query.start;
    mDatabase->executeSqlAsync(this,asyncContainer,sql);
    
}

//=======================================================================================================================
void TradeManagerChatHandler::ProcessRequestTypeList(Message* message,DispatchClient* client)
{
    /*
    TradeManagerAsyncContainer* asyncContainer;
    mPlayerAccountMap = mChatManager->getPlayerAccountMap();

    Player* player;

    PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

    if(accIt != mPlayerAccountMap.end())
        player = (*accIt).second;
    else
    {
        return;
    }

            BString servername;

            uint32 theCounter = 0;

            TypeListItem TypeListTemp;


            servername = "Wookiee.";//.4502";
            char buffer[12];
            itoa(Commodities->CommoditiesTypeCount,buffer,10);
            servername << buffer;

            gMessageFactory->StartMessage();
            gMessageFactory->addUint32(opCommoditiesTypeListResponse);
            gMessageFactory->addString(servername);//
            gMessageFactory->addUint32(Commodities->GetCount());

            CommoditiesTypeList::iterator it = Commodities->mCommoditiesTypeList.begin();

            while(it != Commodities->mCommoditiesTypeList.end())
            {
                gMessageFactory->addUint32((*it)->GetMask());
                gMessageFactory->addUint32((*it)->GetCount());

                CommoditiesItemList::iterator itItem = (*it)->mCommoditiesItemList.begin();
                while(itItem != (*it)->mCommoditiesItemList.end())
                {
                    gMessageFactory->addUint32((*itItem)->GetCRC());
                    gMessageFactory->addString((*itItem)->GetDirectory());
                    gMessageFactory->addUint32((*itItem)->GetUnknown());
                    gMessageFactory->addString((*itItem)->GetName());
                    itItem++;
                }
                it++;
            }
            Message* newMessage = gMessageFactory->EndMessage();
            client->SendChannelA(newMessage, client->getAccountId(),  CR_Client, 6);

    */
}

//=======================================================================================================================
void TradeManagerChatHandler::processHandleIsVendorMessage(Message* message,DispatchClient* client)
{
    mPlayerAccountMap = mChatManager->getPlayerAccountMap();

    Player* player;

    PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

    if(accIt != mPlayerAccountMap.end())
        player = (*accIt).second;
    else
    {
        return;
    }

    //Todo ::
    //Vendor or Bazaar?
    //whos vendor is this??? is it ours???

    uint64 mVendorId = message->getUint64();

    BString mBazaarString = "";
    uint32 error = 0;

    mBazaarString = getBazaarString(mVendorId);

    if (mBazaarString == "") {
        error = 1;
    }

    //permission hardcoded as 2 until player Vendors are in
    uint32 mPermission = 2;
    gChatMessageLib->sendIsVendorOwnerResponseMessage(client, mBazaarString, mPermission, error, mVendorId);

    Bazaar* mBazaarInfo = getBazaarInfo(mVendorId);
    player->setBazaar(mBazaarInfo);
}

//=======================================================================================================================
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//every second we update the Global tickcount to the db
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TradeManagerChatHandler::handleGlobalTickPreserve()
{
    TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_SaveGlobalTick, 0);

    mDatabase->executeProcedureAsync(this, asyncContainer, "CALL %s.sp_ServerGlobalTickUpdate (%u, '%" PRIu64 "');", mDatabase->galaxy(),  2, getGlobalTickCount());
    
}

//=======================================================================================================================

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Will be called every 10 seconds to find all expired auctions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TradeManagerChatHandler::handleCheckAuctions()
{
    TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_ExpiredListing, NULL);
    uint32 time = static_cast<uint32>(getGlobalTickCount());
    mDatabase->executeProcedureAsync(this, asyncContainer, "CALL %s.sp_BazaarAuctionFindExpired(%"PRIu32");", mDatabase->galaxy(),  time/1000);
    
}

//=======================================================================================================================
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//every second the server tick is advanced by one
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TradeManagerChatHandler::handleGlobalTickUpdate()
{
    mGlobalTickCount += 1000; // in our case a 1 second time interval is sufficient
}

//=======================================================================================================================
void TradeManagerChatHandler::handleTimer(uint32 id, void* container)
{
    boost::mutex::scoped_lock lock(mTimerMutex);
    mTimerEventQueue.push(id);
}

//=======================================================================================================================
void TradeManagerChatHandler::processTimerEvents()
{
    uint64	startTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
    uint64	processTime = 0;
    while(mTimerEventQueue.size() && processTime < mTimerQueueProcessTimeLimit)
    {
        boost::mutex::scoped_lock lock(mTimerMutex);
        uint32 id = mTimerEventQueue.front();
        mTimerEventQueue.pop();
        lock.unlock();

        switch(id)
        {
        case CMTimer_GlobalTick:
            handleGlobalTickUpdate();
            break;
        case CMTimer_TickPreserve:
            handleGlobalTickPreserve();
            break;
        case CMTimer_CheckAuctions:
            handleCheckAuctions();
            break;

        default:
            break;
        }

        processTime = Anh_Utils::Clock::getSingleton()->getLocalTime() - startTime;
    }
}


//=======================================================================================================================
void TradeManagerChatHandler::Process()
{
    processTimerEvents();
}



//=======================================================================================================================
void TradeManagerChatHandler::ProcessBankTip(Message* message,DispatchClient* client)
{
    mPlayerAccountMap = mChatManager->getPlayerAccountMap();
    Player* player;
    PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

    if(accIt != mPlayerAccountMap.end())
        player = (*accIt).second;
    else
    {
        return;
    }

    uint64	recipientID	= message->getUint64();
    /*uint64	playerID	= */
    message->getUint64();
    uint32	amount		= message->getUint32();

    BString	receiverName;
    message->getStringAnsi(receiverName);

    //send the respective EMails
    gChatMessageLib->sendBanktipMail(client, player, receiverName,recipientID, amount);

    //update the bankaccount of the recipient if online
    Player* recipient = gChatManager->getPlayerByName(receiverName);

    if(recipient)
    {
        //let the zone update the players bank account db is already updated at this point
        gChatMessageLib->sendBankTipDeductMessage(client,recipient->getCharId(),amount,recipient);
        gChatMessageLib->sendSystemMessageProper(recipient,0,L"","base_player","prose_tip_pass_target","","","",amount,"","","",player->getCharId(),0,0,"","","");

    }
    receiverName.convert(BSTRType_Unicode16);

    gChatMessageLib->sendSystemMessageProper(player,0,L"","base_player","prose_tip_pass_self","","","",amount,"","",receiverName.getUnicode16(),0,0,0,"","","");

    //send the respective sys messages

}

