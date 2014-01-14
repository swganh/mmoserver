 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "ChatOpcodes.h"
#include "Mail.h"
#include "Player.h"
#include "TradeManagerChat.h"
#include "TradeManagerHelp.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"

#include "Common/atMacroString.h"
#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"

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

string TradeManagerChatHandler::getBazaarString(uint64 ID)
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

	mMessageDispatch->RegisterMessageCallback(opIsVendorMessage,this);
	mMessageDispatch->RegisterMessageCallback(opAuctionQueryHeadersMessage,this);
	mMessageDispatch->RegisterMessageCallback(opGetAuctionDetails,this);
	mMessageDispatch->RegisterMessageCallback(opCancelLiveAuctionMessage,this);
	mMessageDispatch->RegisterMessageCallback(opRetrieveAuctionItemMessage,this);
	mMessageDispatch->RegisterMessageCallback(opProcessCreateAuction,this);
	mMessageDispatch->RegisterMessageCallback(opGetCommoditiesTypeList,this);
	mMessageDispatch->RegisterMessageCallback(opBidAuctionMessage,this);
	mMessageDispatch->RegisterMessageCallback(opBankTipDustOff,this);





	// load our bazaar terminals
	asyncContainer = new TradeManagerAsyncContainer(TRMQuery_LoadBazaar, 0);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT * FROM commerce_bazaar");

	// load our global tick
	asyncContainer = new TradeManagerAsyncContainer(TRMQuery_LoadGlobalTick, 0);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT Global_Tick_Count FROM galaxy WHERE galaxy_id = '2'");

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
	//TradeManagerAsyncContainer* asynContainer = new (TradeManagerAsyncContainer);
	// save our global tick
	int8 sql[256];
	sprintf(sql,"UPDATE galaxy SET Global_Tick_Count = '%"PRIu64"' WHERE galaxy_id = '2'",getGlobalTickCount());
	mDatabase->ExecuteSqlAsync(this,NULL,sql);

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
	/*
	// timers
	// clear our eventqueue
	TimerList::iterator timerIt = mTimers.begin();
	while(timerIt != mTimers.end())
	{
		Timer* timer = (*timerIt);

		if(timer->IsRunning())
			timer->Stop();

		// Don't delete object before trying to erase it from list. But I'm not sure if mTimers.erase(..) deletes the object.
		// delete(timer);
		timerIt = mTimers.erase(timerIt);
	}
    */
}

//======================================================================================================================
void TradeManagerChatHandler::handleDispatchMessage(uint32 opcode, Message* message, DispatchClient* client)
{
	switch(opcode)
	{
		case opBankTipDustOff:
		{
			ProcessBankTip(message,client);
		}
		break;

		case opGetCommoditiesTypeList:
		{
			//_ProcessRequestTypeList(message,client);
			//gLogger->logMsgF("opCommoditiesTypeList:: handled opcode %u : sendTypeList",MSG_NORMAL,opcode);
		}
		break;
		case opProcessCreateAuction:
		{
			ProcessCreateAuction(message,client);
			//gLogger->logMsgF("opProcessCreateAuction::handleDispatchMessage: handled opcode %u",MSG_NORMAL,opcode);

		}
		break;
		case opRetrieveAuctionItemMessage:
		{
			processRetrieveAuctionItemMessage(message,client);
			//gLogger->logMsgF("opRetrieveAuctionItemMessage::handleDispatchMessage: handled opcode %u",MSG_NORMAL,opcode);
		}
		break;

		case opBidAuctionMessage:
		{
			processBidAuctionMessage(message,client);
			//gLogger->logMsgF("opBidAuctionAcceptedMessage",MSG_NORMAL);
		}
		break;
		case opCancelLiveAuctionMessage:
		{
			processCancelLiveAuctionMessage(message,client);
			//gLogger->logMsgF("opCanceLiveAuctionMessage::handleDispatchMessage:",MSG_NORMAL);
		}
		break;
		case opIsVendorMessage:
		{
			processHandleIsVendorMessage(message,client);
			//gLogger->logMsgF("opIsVendorMessage::handleDispatchMessage: handled opcode %u",MSG_NORMAL,opcode);
		}
		break;

		case opGetAuctionDetails:
		{
			processGetAuctionDetails(message,client);
			//gLogger->logMsgF("opGetAuctionDetails::handleDispatchMessage: handled opcode %u",MSG_NORMAL,opcode);
		}
		break;

		case opAuctionQueryHeadersMessage:
		{
			processHandleopAuctionQueryHeadersMessage(message,client);
			//gLogger->logMsgF("opIsVendorMessage::handleDispatchMessage: handled opcode %u",MSG_NORMAL,opcode);

		}
		break;

		default:
			gLogger->logMsgF("TradeManagerMessage::handleDispatchMessage: Unhandled opcode %u",MSG_NORMAL,opcode);
		break;
	}
}

//=======================================================================================================================

uint32	TradeManagerChatHandler::TerminalRegionbyID(uint64 id)
{
	BazaarList::iterator it = mBazaars.begin();
	while(it != mBazaars.end())
	{
		if((*it)->id == id){
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
			gLogger->logMsgF("Error getting player from account map %u",MSG_NORMAL,asynContainer->mClient->getAccountId());
			return;
		}
	}

	switch(asynContainer->mQueryType)
	{

		case TRMQuery_RetrieveAuction:
			{
				DataBinding* binding = mDatabase->CreateDataBinding(3);
				binding->addField(DFT_uint64,offsetof(AuctionItem,ItemID),8,0);
				binding->addField(DFT_uint64,offsetof(AuctionItem,BazaarID),8,1);
				binding->addField(DFT_uint32,offsetof(AuctionItem,ItemTyp),4,2);
				AuctionItem AuctionTemp;
				result->GetNextRow(binding,&AuctionTemp);
				//can we retrieve the item from our terminal???
				//uint32 mZoneId = player->getPlanetId();
				uint32 error = 0;
				if (TerminalRegionbyID(asynContainer->BazaarID) == TerminalRegionbyID(AuctionTemp.BazaarID)){
					//ok now delete from commerce_auction
					int8 sql[100];
					sprintf(sql,"DELETE FROM commerce_auction WHERE auction_id = '%"PRIu64"' ",AuctionTemp.ItemID);

					TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_DeleteAuction,asynContainer->mClient);
					mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
					//send relevant info to Zoneserver for Itemcreation
					gChatMessageLib->processSendCreateItem(asynContainer->mClient, player->getCharId(),AuctionTemp.ItemID,AuctionTemp.ItemTyp,player->getPlanetId());

					//gChatMessageLib->SendRetrieveAuctionItemResponseMessage(asynContainer->mClient,AuctionTemp.ItemID, error);
				}else{
					//send error
					error = 1;
				}
				gChatMessageLib->SendRetrieveAuctionItemResponseMessage(asynContainer->mClient,AuctionTemp.ItemID, error);

				mDatabase->DestroyDataBinding(binding);

		}
		break;

		case TRMQuery_CreateAuction:
			{

			}

		break;
		case TRMQuery_ACKRetrieval:
			{
				//probably we should ponder letting the sf respond with 0 in case of everything ok and !0 in case of error
				gChatMessageLib->sendBidAuctionResponse(asynContainer->mClient,0,asynContainer->AuctionID);
			}
		break;
		case TRMQuery_GetDetails:
			{
				//TODO handle resources!!!!!!!!!!!

				DataBinding* binding = mDatabase->CreateDataBinding(3);

				binding->addField(DFT_uint64,offsetof(DescriptionItem,ItemID),8,0);
				binding->addField(DFT_string,offsetof(DescriptionItem,Description),1024,1);
				binding->addField(DFT_string,offsetof(DescriptionItem,tang),128,2);

				DescriptionItem* mItemDescription = new DescriptionItem;

				result->GetNextRow(binding,mItemDescription);

				//might be a resource
				if(strstr(mItemDescription->tang,"resource_container"))
				{
					//build our query to get the attributes
					int8 sql[1024];
					//we'll need the quantity and all the attributes
					sprintf(sql,"SELECT rc.resource_id, rc.amount, r.name, r.er, r.cr,r.cd,r.dr,r.fl,r.hr,r.ma, r.oq, r.sr, r.ut, r.pe FROM swganh.resource_containers rc INNER JOIN swganh.resources r ON r.id = rc.resource_id WHERE rc.id =%"PRIu64"",mItemDescription->ItemID);
					TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_GetResAttributeDetails,asynContainer->mClient);
					asyncContainer->mItemDescription = mItemDescription;

					mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

					mDatabase->DestroyDataBinding(binding);

					return;
				}

				//build our query to get the attributes
				int8 sql[1024];
				//we'll need all the attributes which are marked as external
				sprintf(sql,"SELECT name, value FROM swganh.item_attributes ia  INNER JOIN swganh.attributes a ON ia.attribute_id = a.id WHERE ia.item_id =%"PRIu64" and a.internal = 0 ORDER BY ia.order",mItemDescription->ItemID);

				TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_GetAttributeDetails,asynContainer->mClient);
				asyncContainer->mItemDescription = mItemDescription;

				mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

				mDatabase->DestroyDataBinding(binding);
		}
		break;

		case TRMQuery_GetResAttributeDetails:
			{
				DataBinding* binding = mDatabase->CreateDataBinding(14);

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
					result->GetNextRow(binding,mItemDescription);


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
				mDatabase->DestroyDataBinding(binding);


				return;
			}

		case TRMQuery_GetAttributeDetails:
			{
				//now that we have the auctions main data we need to query for all the atrributes and their values
				DataBinding* binding = mDatabase->CreateDataBinding(2);

				binding->addField(DFT_string,offsetof(ItemDescriptionAttributes,name),32,0);
				binding->addField(DFT_string,offsetof(ItemDescriptionAttributes,value),32,1);

				AttributesList::iterator it = mAtrributesList.begin();

				uint64 count = result->getRowCount();

				for(uint16 i=0;i <count;i++)
				{
					ItemDescriptionAttributes* mItemDescription = new ItemDescriptionAttributes;
					result->GetNextRow(binding,mItemDescription);
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
				mDatabase->DestroyDataBinding(binding);
			}
			break;

		case TRMQuery_CancelAuction_BidderMail:
			{
				AuctionItem Item;

				DataBinding* binding = mDatabase->CreateDataBinding(3);
				binding->addField(DFT_uint64,offsetof(AuctionItem,BidderID),8,0);
				binding->addField(DFT_string,offsetof(AuctionItem,Name),128,1);
				binding->addField(DFT_uint32,offsetof(AuctionItem,HighProxy),4,2);

				uint64 count = result->getRowCount();
				if (count >= 1)	{
					//the Query actually only yields a result when we are dealing with an auction
					for(uint16 i=0;i <count;i++){
						result->GetNextRow(binding,&Item);
						gChatMessageLib->sendBidderCancelAuctionMail(asynContainer->mClient, player->getCharId(),Item.BidderID, Item.Name);

						//TODO
						//refund the bidders
					}
				}
				int8 sql[100];
				sprintf(sql,"SELECT sf_CancelLiveAuction ('%"PRIu64"')",asynContainer->AuctionID);

				TradeManagerAsyncContainer* asyncContainer;
				asyncContainer = new TradeManagerAsyncContainer(TRMQuery_CancelAuction,asynContainer->mClient);

				asyncContainer->AuctionID = asynContainer->AuctionID;
				mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
				mDatabase->DestroyDataBinding(binding);

			}
		break;

		case TRMQuery_CancelAuction:
			{
				int8 ItemName[128];

				DataBinding* binding = mDatabase->CreateDataBinding(1);

				binding->addField(DFT_string,0,128);
				uint64 count = result->getRowCount();

				if (count == 1){
					result->GetNextRow(binding,&ItemName);
					gChatMessageLib->sendCanceLiveAuctionResponseMessage(asynContainer->mClient, 0,asynContainer->AuctionID);

					//send the relevant EMail
					gChatMessageLib->sendCancelAuctionMail(asynContainer->mClient, player->getCharId(),player->getCharId(), ItemName);
					mDatabase->DestroyDataBinding(binding);
				}
				else
				{
					gChatMessageLib->sendCanceLiveAuctionResponseMessage(asynContainer->mClient, 1,asynContainer->AuctionID);
					gLogger->logMsgF("TradeManager::TRMQuery_CancelAuction::Aucction not found : %I64u",MSG_NORMAL,asynContainer->AuctionID);

				}
			}
		break;

		case TRMQuery_LoadBazaar:
		{
			DataBinding* binding = mDatabase->CreateDataBinding(4);
			binding->addField(DFT_uint64,offsetof(Bazaar,id),8,0);
			binding->addField(DFT_string,offsetof(Bazaar,string),128,1);
			binding->addField(DFT_uint32,offsetof(Bazaar,regionid),4,2);
			binding->addField(DFT_uint32,offsetof(Bazaar,planetid),4,3);

			uint64 count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				Bazaar* bazaar = new Bazaar();
				result->GetNextRow(binding,bazaar);

				mBazaars.push_back(bazaar);
			}

			mBazaarCount += static_cast<uint32>(count);
			mBazaarsLoaded = true;
			mDatabase->DestroyDataBinding(binding);
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

			DataBinding* binding = mDatabase->CreateDataBinding(18);
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
			binding->addField(DFT_string,offsetof(AuctionItem,Description),1024,10);
			binding->addField(DFT_uint16,offsetof(AuctionItem,RegionID),2,11);
			binding->addField(DFT_string,offsetof(AuctionItem,bidder_name),32,12);
			binding->addField(DFT_uint16,offsetof(AuctionItem,PlanetID),2,13);
			binding->addField(DFT_string,offsetof(AuctionItem,SellerName),32,14);
			binding->addField(DFT_string,offsetof(AuctionItem,BazaarName),128,15);
			binding->addField(DFT_string,offsetof(AuctionItem,HighProxyRaw),8,16);
			binding->addField(DFT_string,offsetof(AuctionItem,HighBidRaw),8,17);

			uint64 count = result->getRowCount();

			auction = new AuctionClass();

			AuctionItem AuctionTemp;
			//Not sure how to properly read this in. We perform a left join, because
			//the data is not existing when nobody has bid on the item yet

			strcpy(AuctionTemp.HighBidRaw,"0          ");
			strcpy(AuctionTemp.HighProxyRaw,"0          ");

			AuctionTemp.HighBid = 0;
			AuctionTemp.HighProxy = 0;
			for(uint64 i = 0;i < count;i++)
			{
				//assemble our string list
				result->GetNextRow(binding,&AuctionTemp);
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

			string s;
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


				gMessageFactory->addUint32((*itA)->GetBid());//highbid My High Bid!!!!
				gMessageFactory->addUint32((*itA)->GetProxy());// my Proxy
				gMessageFactory->addUint32((*itA)->GetBid());//highbid My High Bid!!!!

				gMessageFactory->addUint32((*itA)->GetCategory());// itemtype for proper text reference


				gMessageFactory->addUint8(0);
				//Ok now heres our bitmask
				//1
				//2
				//4 = Premium
				//8 = shows Accept bid AND Withdraw sale on own auctions
				uint8 bitmap = 0;
				bitmap = (bitmap | 8);//set bit
				if (player->getCharId() == (*itA)->GetOwnerID()){
					//bitmap = (bitmap | 8);//set bit 4
					if ((*itA)->GetType() == 2){
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
			if ((pages-asynContainer->Itemsstart) < 100){
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
			mDatabase->DestroyDataBinding(binding);

		}
		break;

		case TRMQuery_ProcessBidAuction:
			{

				DataBinding* binding = mDatabase->CreateDataBinding(3);
				binding->addField(DFT_uint32,offsetof(AuctionItem,HighProxy),4,0);
				binding->addField(DFT_uint32,offsetof(AuctionItem,HighBid),4,1);
				binding->addField(DFT_uint64,offsetof(AuctionItem,BidderID),8,2);

				AuctionItem AuctionTemp;

				uint64 count = result->getRowCount();
				asynContainer->AuctionTemp->HighBid = 0 ;
				asynContainer->AuctionTemp->HighProxy = 0;
				asynContainer->AuctionTemp->BidderID = 0;
				if(count == 1){
					//there is a highbidder ... - we have the highbid and proxy
					result->GetNextRow(binding,&AuctionTemp);
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

				 DataBinding* binding = mDatabase->CreateDataBinding(9);
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

				for(uint32 i = 0;i < count;i++)
				{
					auctionTemp = new(AuctionItem);
					//an auction with no bidder will have the bidder_id as NULL and the bidder name empty
					strcpy(auctionTemp->bidder_name,"");
					strcpy(auctionTemp->BidderIDRaw,"");
					auctionTemp->BidderID=0;

					result->GetNextRow(binding,auctionTemp);

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
						//gLogger->logMsgF("check refund",MSG_NORMAL);
						//read in the bidhistory and iterate through the bidders of the auction
						//refund money to everybody but the winning bidder

						TradeManagerAsyncContainer* asyncContainer;
						sprintf(sql,"SELECT cbh.proxy_bid, c.id  FROM swganh.characters AS c INNER JOIN swganh.commerce_bidhistory AS cbh ON (c.firstname = cbh.bidder_name) WHERE cbh.auction_id = %"PRIu64"",auctionTemp->ItemID);
						asyncContainer = new TradeManagerAsyncContainer(TRMQuery_ProcessAuctionRefund,NULL);
						asyncContainer->AuctionTemp = auctionTemp;
						//mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

						//WE NEED THIS WHILE WE HAVE THE ABOVE LINE COMMENTED OUT!!!
						delete asyncContainer;
					}

					//Update the new owner in case our auction got sold
					if((auctionTemp->AuctionTyp == TRMVendor_Auction) && (auctionTemp->BidderID != 0))
					{
						//set the new Owner
						TradeManagerAsyncContainer* asyncContainer;

						//sprintf(sql," UPDATE commerce_bidhistory SET proxy_bid = '%u'WHERE auction_id = '%I64u' AND bidder_name = '%s'",asynContainer->MyProxy  ,asynContainer->AuctionTemp->ItemID ,Name);
						sprintf(sql,"UPDATE swganh.commerce_auction SET owner_id = %"PRIu64", bidder_name = '' WHERE auction_id = %"PRIu64" ",auctionTemp->BidderID,auctionTemp->ItemID);
						asyncContainer = new TradeManagerAsyncContainer(TRMQuery_NULL,NULL);
						mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
					}

					it++;
				}

				//now move the auctions in their proper holding areas or delete the ones which have expired their holding date
				TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_NULL,NULL);

				sprintf(sql,"CALL sp_CommerceFindExpiredListing()");
				mDatabase->ExecuteProcedureAsync(this,asyncContainer,sql);

				}
			break;

			case TRMQuery_ProcessAuctionRefund:
			{
				DataBinding* binding = mDatabase->CreateDataBinding(2);
				binding->addField(DFT_uint32,offsetof(AuctionItem,MyProxy),4,0);
				binding->addField(DFT_uint64,offsetof(AuctionItem,BidderID),8,1);

				AuctionItem* AuctionTemp;
				uint64 count = result->getRowCount();

				//iterate through all the bids and refund them
				//Mind the winning bidder though
				int8 sql[390];
				//printf("\n %u bidders to refund minus one buyer\n", count);
				for(uint64 i = 0;i < count;i++)
				{
					AuctionTemp = new(AuctionItem);
					result->GetNextRow(binding,AuctionTemp);
					//printf("\nits not the winner - so refund\n");
					//let zone handle the refund problem for logged ins in the Object
					//sometime in the future

					//just update db here - only add credits so this should be rather relaxed
					if(AuctionTemp->BidderID != asynContainer->AuctionTemp->OwnerID){
						TradeManagerAsyncContainer* asyncContainer;

						sprintf(sql,"UPDATE banks SET credits=credits+%"PRId32" WHERE id=%"PRIu64"",asynContainer->MyProxy, AuctionTemp->BidderID+4);
						asyncContainer = new TradeManagerAsyncContainer(TRMQuery_NULL,NULL);
						mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
					}
					SAFE_DELETE(AuctionTemp);

				}

				//now delete the Auctions Bidhistory
				TradeManagerAsyncContainer* asyncContainer;

				sprintf(sql,"DELETE FROM commerce_bidhistory WHERE auction_id = '%"PRIu64"' ",asynContainer->AuctionTemp->ItemID);
				asyncContainer = new TradeManagerAsyncContainer(TRMQuery_NULL,NULL);
				mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

				SAFE_DELETE(asyncContainer->AuctionTemp);

			}
			break;

			case TRMQuery_BidAuction:
			{
				DataBinding* binding = mDatabase->CreateDataBinding(10);
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
				result->GetNextRow(binding,AuctionTemp);

				mDatabase->DestroyDataBinding(binding);

				Player* player = gChatManager->getPlayerbyId(asynContainer->BuyerID);

				Bazaar* bazaarInfo = getBazaarInfo(AuctionTemp->BazaarID);

				//is it an auction or an instant
				if (AuctionTemp->AuctionTyp == 0)
				{
					//now get additional Information - do we have an actual high bidder? - Who is it? and
					//what is the highbid and the highproxy
					//_processAuctionBid(AuctionTemp,asynContainer,player);
					TradeManagerAsyncContainer* asyncContainer;

					gLogger->logMsgF("TMR query bidauction",MSG_NORMAL);
					int8 name[40],*sqlPointer;
					sqlPointer = name;
					sqlPointer += mDatabase->Escape_String(name,AuctionTemp->bidder_name,strlen(AuctionTemp->bidder_name));
					*sqlPointer++ = '\0';

					int8 sql[390];
					sprintf(sql,"SELECT cbh.proxy_bid, cbh.max_bid, c.id FROM swganh.characters AS c INNER JOIN swganh.commerce_bidhistory AS cbh ON c.firstname = '%s' WHERE cbh.auction_id = %"PRIu64" AND cbh.bidder_name = '",name,AuctionTemp->ItemID);

					sqlPointer = sql + strlen(sql);
					sqlPointer += mDatabase->Escape_String(sqlPointer,AuctionTemp->bidder_name,strlen(AuctionTemp->bidder_name));
					*sqlPointer++ = '\'';
					*sqlPointer++ = '\0';

					asyncContainer = new TradeManagerAsyncContainer(TRMQuery_ProcessBidAuction,asynContainer->mClient);
					asyncContainer->MyBid = asynContainer->MyBid;
					asyncContainer->MyProxy = asynContainer->MyProxy;
					asyncContainer->AuctionID = asynContainer->AuctionID;

					asyncContainer->BuyerID = player->getCharId();
					asyncContainer->BazaarID = 0;//
					asyncContainer->AuctionTemp = AuctionTemp;
					mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);


					break;
				}

				if (AuctionTemp->AuctionTyp == 1)
				{
					//instant
					//the client checks for the money so we will check for cheating later in the zoneserver
					//the client sends a retrieve message hereafter so this is only to buy the item
					//flag it as bought, change the owner and the remaining Time
					//retrieve is send by client after that

					uint32 time = (3600*24*30)+( static_cast<uint32>(getGlobalTickCount())/1000);

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
			DataBinding* binding = mDatabase->CreateDataBinding(1);
			binding->addField(DFT_uint64,offsetof(TradeManagerChatHandler,mGlobalTickCount),8,0);
			result->GetNextRow(binding,this);
			mDatabase->DestroyDataBinding(binding);
		}
		break;

	default:break;
	}
	SAFE_DELETE(asynContainer);

	if(mBazaarsLoaded)
	{
		mBazaarsLoaded = false;
		gLogger->logMsgF("TradeManager::Load complete (%u bazaars )",MSG_NORMAL,mBazaarCount);
	}

}

//=======================================================================================================================

void TradeManagerChatHandler::processAuctionEMails(AuctionItem* auctionTemp)
{
	gLogger->logMsgF("processAuctionEMails",MSG_NORMAL);
	Bazaar* mBazaarInfo = getBazaarInfo(auctionTemp->BazaarID);

	char *Token;
	char separation[] = ".#,";
	string mString;
	mString = mBazaarInfo->string;

	Token = strtok(mString.getRawData(),separation); //
	string planet = BString(Token);
	Token = strtok( NULL,separation);
	string region = BString(Token);

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
		gLogger->logMsgF("trade manager vendor instant",MSG_NORMAL);
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
		gLogger->logMsgF("trade manager vendor instant ended",MSG_NORMAL);

	}

	if(auctionTemp->AuctionTyp == TRMVendor_Auction)
	{
		gLogger->logMsgF("trm vendor auction",MSG_NORMAL);
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
		gLogger->logMsgF("trm vendor auction ended",MSG_NORMAL);
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
	sqlPointer += mDatabase->Escape_String(sqlPointer,player->getName().getAnsi(),player->getName().getLength());
	*sqlPointer++ = '\0';

	//Check the current add depending on the price


	uint32 TheBid;
	uint32 TheProxy;
	// check if there is a valid bid
	//a) the bid amount

	if (asynContainer->MyBid > mBazaarMaxBid){
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
			//strcpy(Name,player->getName().getAnsi());
			//sprintf(sql," UPDATE commerce_bidhistory SET proxy_bid = '%u'WHERE auction_id = '%I64u' AND bidder_name = %s",asynContainer->MyProxy  ,asynContainer->AuctionTemp->ItemID ,Name);
			sprintf(sql," UPDATE commerce_bidhistory SET proxy_bid = '%"PRIu32"'WHERE auction_id = '%"PRIu64"' AND bidder_name = '%s'",asynContainer->MyProxy  ,asynContainer->AuctionTemp->ItemID,PlayerName);

			TradeManagerAsyncContainer* asyncContainer;
			asyncContainer = new TradeManagerAsyncContainer(TRMQuery_ACKRetrieval,asynContainer->mClient);

			asyncContainer->AuctionID = asynContainer->AuctionTemp->ItemID;
			mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

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
	if (asynContainer->MyProxy > asynContainer->AuctionTemp->HighProxy){
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
		if(TheBid < asynContainer->AuctionTemp->Price){
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
		//what do we do if this is our first bid and we are NOT the high bidder?
		//sf_BidAuction only updates the bid of the high bidder

		//solution!!! Invent sf_BidUpdates

		sprintf(sql,"SELECT sf_BidUpdate ('%"PRIu64"','%"PRIu32"','%"PRIu32"','%s')",asynContainer->AuctionTemp->ItemID,asynContainer->MyBid,asynContainer->MyProxy,PlayerName);
		TradeManagerAsyncContainer* asyncContainer;
		asyncContainer = new TradeManagerAsyncContainer(TRMQuery_ACKRetrieval,asynContainer->mClient);
		mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

	}

	sprintf(sql,"SELECT sf_BidAuction ('%"PRIu64"','%"PRIu32"','%"PRIu32"','%s')",asynContainer->AuctionTemp->ItemID,TheBid,TheProxy,PlayerName);
	TradeManagerAsyncContainer* asyncContainer;
	asyncContainer = new TradeManagerAsyncContainer(TRMQuery_ACKRetrieval,asynContainer->mClient);

	asyncContainer->AuctionID = asynContainer->AuctionTemp->ItemID;
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

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
		gLogger->logMsgF("Error getting player from account map %u",MSG_NORMAL,client->getAccountId());
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
		sprintf(Query," UPDATE commerce_auction SET start = '%"PRIu64"', region_id = '%"PRIu32"', planet_id = '%"PRIu32"',category = '%"PRIu32"' WHERE auction_id = '%"PRIu64"'", time, RegionID, player->getPlanetId(),category,ItemID);
	}
	else
	{
		sprintf(Query," UPDATE commerce_auction ca inner join swganh.items i ON (i.id = ca.auction_id) inner join swganh.item_types it ON (it.id = i.item_type) SET ca.start = '%"PRIu64"' , ca.region_id = '%"PRIu32"', ca.planet_id = '%"PRIu32"', ca.category = it.bazaar_category  WHERE ca.auction_id = '%"PRIu64"'", time, RegionID, player->getPlanetId(),ItemID);
	}


	TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_CreateAuction,client);
	asyncContainer->AuctionID = ItemID;
	mDatabase->ExecuteSqlAsync(this,asyncContainer,Query);
}

//=======================================================================================================================
void TradeManagerChatHandler::processRetrieveAuctionItemMessage(Message* message,DispatchClient* client)
{
	TradeManagerAsyncContainer* asyncContainer;
	Player* player;
	PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

	if(accIt != mPlayerAccountMap.end())
		player = (*accIt).second;
	else
	{
		gLogger->logMsgF("Error getting player from account map %u",MSG_NORMAL,client->getAccountId());
		return;
	}

	//squeeze our Packet for all usefull information
	//ID of the Auction that gets retrieved
	uint64	ItemID		= message->getUint64();
	uint64	TerminalID	= message->getUint64();

	int8 sql[200];
	sprintf(sql,"SELECT auction_id, bazaar_id, itemtype FROM swganh.commerce_auction c  WHERE c.auction_id = %"PRIu64"",ItemID);
	asyncContainer = new TradeManagerAsyncContainer(TRMQuery_RetrieveAuction,client);
	asyncContainer->BazaarID = TerminalID;
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
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
		gLogger->logMsgF("processBidAuctionMessage :: Error getting the player from account map %u",MSG_NORMAL,client->getAccountId());
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

	sprintf(sql,"SELECT ca.auction_id, ca.owner_id, ca.type, ca.category, ca.price, ca.itemtype, ca.name, ca.bazaar_id, ca.bidder_name, c.firstname FROM swganh.characters AS c INNER JOIN swganh.commerce_auction AS ca ON (ca.owner_id = c.id) where ca.auction_id = %"PRIu64"",ItemID);

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

	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
	//client checks if we have enough money
	//cheaters (bot/ modified client )will be flagged in the zoneserver

}



//=======================================================================================================================
void TradeManagerChatHandler::processCancelLiveAuctionMessage(Message* message,DispatchClient* client)
{
	TradeManagerAsyncContainer* asyncContainer;

	Player* player;
	PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

	if(accIt != mPlayerAccountMap.end())
		player = (*accIt).second;
	else
	{
		gLogger->logMsgF("Error getting player from account map %u",MSG_NORMAL,client->getAccountId());
		return;
	}

	//ID of the Auction that gets canceled
	uint64	ItemID		= message->getUint64();

	//get all the bids on the auction and refunf the affected players
	//send the EMails
	int8 sql[300];
	sprintf(sql,"SELECT ch.id, ca.name, cbh.proxy_bid FROM swganh.commerce_auction AS ca INNER JOIN swganh.commerce_bidhistory AS cbh ON (cbh.bidder_name = ca.bidder_name)  INNER JOIN swganh.characters AS ch ON (cbh.bidder_name = ch.firstname)  WHERE ca.auction_id = %"PRIu64"",ItemID);
	asyncContainer = new TradeManagerAsyncContainer(TRMQuery_CancelAuction_BidderMail,client);
	asyncContainer->AuctionID = ItemID;
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);



}


//=======================================================================================================================
void TradeManagerChatHandler::processGetAuctionDetails(Message* message,DispatchClient* client)
{
	TradeManagerAsyncContainer* asyncContainer;

	Player* player;
	PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

	if(accIt != mPlayerAccountMap.end())
		player = (*accIt).second;
	else
	{
		gLogger->logMsgF("Error getting player from account map %u",MSG_NORMAL,client->getAccountId());
		return;
	}

	//the ID of the Auction we want to learn more about
	uint64 AuctionID = message->getUint64();

	//build our query
	int8 sql[1024];
	//we'll need our item description, the iff data and the rest will be done by the items object
	sprintf(sql,"SELECT auction_id, description, object_string FROM swganh.commerce_auction c  WHERE c.auction_id = %"PRIu64"",AuctionID);

	asyncContainer = new TradeManagerAsyncContainer(TRMQuery_GetDetails,client);
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

}

//=======================================================================================================================
void TradeManagerChatHandler::processHandleopAuctionQueryHeadersMessage(Message* message,DispatchClient* client)
{
	TradeManagerAsyncContainer* asyncContainer;

	//uint64 time = (getGlobalTickCount()/1000);

	int8 sql[2024];
	sprintf(sql,"SELECT c.auction_id, owner_id, c.bazaar_id, type, start, premium, category, itemtype, price, name, description, c.region_id, c.bidder_name, c.planet_id, firstname, bazaar_string, cbh.proxy_bid, cbh.max_bid FROM swganh.commerce_auction c INNER JOIN swganh.characters ch on (c.owner_id = ch.id) INNER join swganh.commerce_bazaar cb ON (cb.bazaar_id = c.bazaar_id) left join swganh.commerce_bidhistory cbh ON (cbh.bidder_name = c.bidder_name AND cbh.auction_id = c.auction_id) AND c.owner_id = ch.id  WHERE");

	Player* player;
	PlayerAccountMap::iterator accIt = mPlayerAccountMap.find(client->getAccountId());

	if(accIt != mPlayerAccountMap.end())
		player = (*accIt).second;
	else
	{
		gLogger->logMsgF("Error getting player from account map ",MSG_NORMAL);
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


	//build our db Quer

	// the region to display

	switch(query.Region)
	{
	case TRMVendor:
		{
			sprintf(query.RegionQuery," (c.bazaar_id = %"PRIu64")",query.vendorID);

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
		case TRMVendor_AllAuctions://open up bazaar and look at it without a category
		{
			sprintf(query.WindowQuery," ((c.type = %"PRIu32") or (c.type = %"PRIu32"))",TRMVendor_Auction,TRMVendor_Instant);

		}
		break;

		case TRMVendor_MySales://what Im selling at the bazaar
		{
			sprintf(query.WindowQuery," ((c.type = %"PRIu32") or (c.type = %"PRIu32"))AND",TRMVendor_Auction,TRMVendor_Instant);
			int8 tmp[128];
			sprintf(tmp," (c.owner_id = %"PRIu64")",player->getCharId());
			strcat(query.WindowQuery,tmp);

		}
		break;

		case TRMVendor_MyBids://auctions Im bidding on currently at the bazaar
		{
			//sprintf(query.WindowQuery," ((c.type = %u) or (c.type = %u)) AND",TRMVendor_Auction,TRMVendor_Instant);
			//int8 tmp[128];
			//sprintf(tmp," (cbh.bidder_name = %s)",player->getName().getAnsi());
			//strcat(query.WindowQuery,tmp);
			//sprintf(sql,"SELECT c.auction_id, owner_id, c.bazaar_id, type, start, premium, category, itemtype, price, name, description, c.region_id, c.bidder_name, c.planet_id, firstname, bazaar_string, cbh.proxy_bid, cbh.max_bid FROM swganh.commerce_auction c INNER JOIN swganh.characters ch on (c.owner_id = ch.id) INNER join swganh.commerce_bazaar cb ON (cb.bazaar_id = c.bazaar_id) inner join swganh.commerce_bidhistory cbh ON (cbh.auction_id = c.auction_id) AND c.owner_id = ch.id WHERE");


			//gLogger->logMsgF("trm vendor my bids",MSG_NORMAL);
			sprintf(query.WindowQuery," ((c.type = %"PRIu32") or (c.type = %"PRIu32")) AND",TRMVendor_Auction,TRMVendor_Instant);
			int8 tmp[128],end[128],*sqlPointer;
			sprintf(tmp," (cbh.bidder_name = '");
			sqlPointer = tmp + strlen(tmp);
			sqlPointer += mDatabase->Escape_String(sqlPointer,player->getName().getAnsi(),player->getName().getLength());
			*sqlPointer++ = '\'';
			*sqlPointer++ = '\0';
			strcat(query.WindowQuery,tmp);
			sprintf(end," )");
			strcat(query.WindowQuery,end);
			sprintf(sql,"SELECT c.auction_id, owner_id, c.bazaar_id, type, start, premium, category, itemtype, price, name, description, c.region_id, c.bidder_name, c.planet_id, firstname, bazaar_string, cbh.proxy_bid, cbh.max_bid FROM swganh.commerce_auction c INNER JOIN swganh.characters ch on (c.owner_id = ch.id) INNER join swganh.commerce_bazaar cb ON (cb.bazaar_id = c.bazaar_id) inner join swganh.commerce_bidhistory cbh ON (cbh.auction_id = c.auction_id) AND c.owner_id = ch.id WHERE");

		}
		break;
		case TRMVendor_AvailableItems:
		{
			sprintf(query.WindowQuery," (c.type = %"PRIu32") AND (c.owner_id = %"PRIu64") ",TRMVendor_Ended,player->getCharId());


		}
		break;
		case TRMVendor_Offers:
		{
			sprintf(query.WindowQuery," (c.type = %"PRIu32") AND (c.bidder_name = %s) AND (c.bazaar_id = %"PRIu64")",TRMVendor_Offer,player->getName().getAnsi(),query.vendorID);


		}
		break;
		case TRMVendor_ForSale:
		{
			gLogger->logMsgF("trm vendor for sale",MSG_NORMAL);
			sprintf(query.WindowQuery," ((c.type = %"PRIu32") or (c.type = %"PRIu32")) AND",TRMVendor_Auction,TRMVendor_Instant);
			int8 tmp[128],end[128],*sqlPointer;
			sprintf(tmp," (c.bidder_name = '");
			sqlPointer = tmp + strlen(tmp);
			sqlPointer += mDatabase->Escape_String(sqlPointer,player->getName().getAnsi(),player->getName().getLength());
			*sqlPointer++ = '\'';
			*sqlPointer++ = '\0';
			strcat(query.WindowQuery,tmp);
			sprintf(end," ) AND");
			strcat(query.WindowQuery,end);

			sprintf(tmp," (c.bazaar_id = %"PRIu64")",query.vendorID);
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



	if (query.ItemTyp != 0){
		sprintf(query.ItemTypQuery," (c.itemtype = %"PRIu32")",query.ItemTyp);

	}
	else
		sprintf(query.ItemTypQuery," ");



	int8 Limit[64];
	uint32 StopTime;
	StopTime = (static_cast<uint32>(getGlobalTickCount()) / 1000);
	sprintf(Limit," AND (c.start > %"PRIu32") LIMIT %u, %u",StopTime,query.start, query.start+100);

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

	if (strcmp(query.CategoryQuery, " ") !=0 ){
		if (add == true)
			strcat(sql," AND ");
		add = true;
	}

	strcat(sql,query.CategoryQuery); //

	if (strcmp(query.ItemTypQuery, " ") !=0 ){
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
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

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
		gLogger->logMsgF("Error getting player from account map %u",MSG_NORMAL,client->getAccountId());
		return;
	}

			string servername;

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
		gLogger->logMsgF("Error getting player from account map %u",MSG_NORMAL,client->getAccountId());
		return;
	}

	//Todo ::
	//Vendor or Bazaar?
	//whos vendor is this??? is it ours???

	uint64 mVendorId = message->getUint64();

	string mBazaarString = "";
	uint32 error = 0;

	mBazaarString = getBazaarString(mVendorId);

	if (mBazaarString == ""){
		error = 1;
		gLogger->logMsgF("TradeManager::_processIsVendorRequest: Planet  %u : Vendor %I64u not found in db",MSG_HIGH,player->getPlanetId(),mVendorId);
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
	int8 sql[256];
	sprintf(sql,"UPDATE galaxy SET Global_Tick_Count = '%"PRIu64"' WHERE galaxy_id = '2'",getGlobalTickCount());
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

}

//=======================================================================================================================

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Will be called every 10 seconds to find all expired auctions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TradeManagerChatHandler::handleCheckAuctions()
{
	TradeManagerAsyncContainer* asyncContainer = new TradeManagerAsyncContainer(TRMQuery_ExpiredListing, NULL);
	int8 sql[500];
	uint32 time = static_cast<uint32>(getGlobalTickCount());
	//sprintf(sql," SELECT ca.auction_id, ca.owner_id,ca.type, ca.price, ca.name, ca.bidder_name, ca.bazaar_id, cc.firstname, c.id FROM swganh.characters AS c RIGHT JOIN swganh.commerce_auction AS ca ON (c.firstname = ca.bidder_name) INNER JOIN swganh.characters AS cc ON (cc.id = ca.owner_id) WHERE %u > start;",time/1000);
	sprintf(sql," SELECT ca.auction_id, ca.owner_id,ca.type, ca.price, ca.name, ca.bidder_name, ca.bazaar_id, cc.firstname, c.id FROM swganh.characters AS c RIGHT JOIN swganh.commerce_auction AS ca ON (c.firstname = ca.bidder_name) INNER JOIN swganh.characters AS cc ON (cc.id = ca.owner_id) WHERE %"PRIu32" > start;",time/1000);

	//sprintf(sql,"CALL sp_CommerceFindExpiredListing()");
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);
	//gLogger->logMsgF("TradeManagerChatHandler::handleCheckAuctions()",MSG_NORMAL);
}

//=======================================================================================================================
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//every second the server tick is advanced by one
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TradeManagerChatHandler::handleGlobalTickUpdate()
{
	mGlobalTickCount += 1000;//in our case 1sek as timeintervall sufficient
}

//=======================================================================================================================
void TradeManagerChatHandler::handleTimer(uint32 id, void* container)
{
    boost::mutex::scoped_lock (mTimerMutex);
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
				gLogger->logMsgF("WorldManager::processTimerEvents: Unknown Timer %u",MSG_HIGH,id);
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
		gLogger->logMsgF("Error getting player from account map %u",MSG_NORMAL,client->getAccountId());
		return;
	}

	uint64	recipientID	= message->getUint64();
	/*uint64	playerID	= */message->getUint64();
	uint32	amount		= message->getUint32();

	string	receiverName;
	message->getStringAnsi(receiverName);

	//send the respective EMails
	gChatMessageLib->sendBanktipMail(client, player, receiverName,recipientID, amount);

	//update the bankaccount of the recipient if online
	Player* recipient = gChatManager->getPlayerByName(receiverName);

	if(recipient)
	{
		//let the zone update the players bank account db is already updated at this point
		gChatMessageLib->sendBankTipDeductMessage(client,recipient->getCharId(),amount,recipient);
		gChatMessageLib->sendSystemMessageProper(recipient,0,"","base_player","prose_tip_pass_target","","","",amount,"","","",player->getCharId(),0,0,"","","");

	}
	receiverName.convert(BSTRType_Unicode16);

	gChatMessageLib->sendSystemMessageProper(player,0,"","base_player","prose_tip_pass_self","","","",amount,"","",receiverName.getUnicode16(),0,0,0,"","","");

	//send the respective sys messages

}

