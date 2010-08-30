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

#ifndef ANH_ZONESERVER_TRADEMANAGER_H
#define ANH_ZONESERVER_TRADEMANAGER_H

#include <vector>
#include "DatabaseManager/DatabaseCallback.h"
#include "CharacterBuilderTerminal.h"

#define 	gTradeManager	TradeManager::getSingletonPtr()

//======================================================================================================================

class Database;
class Message;
class MessageDispatch;
class PlayerObject;
class TangibleObject;

enum TRMQueryType
{
    TRMQuery_NULL						=	0,
    TRMQuery_LoadBazaar					=	1,
    TRMQuery_AuctionQuery				=	2,
    TRMQuery_CreateAuction				=	3,
    TRMQuery_GetDetails					=	4,
    TRMQuery_CancelAuction				=	5,
    TRMQuery_RetrieveAuction			=	6,
    TRMQuery_DeleteAuction				=	7,
    TRMQuery_BidAuction					= 8,
    TRMQuery_ACKRetrieval				= 9,
    TRMQuery_MoneyTransaction			= 10,
    TRMQuery_BazaarMoneyTransaction		= 11,
    TRMQuery_CreateAuctionTransaction	= 12,
    TRMQuery_ItemTableFrogQuery			= 13,
    TRMQuery_CheckListingsBazaar		= 14,

    TRMQuery_LoadGlobalTick =	15,
    TRMQuery_SaveGlobalTick =	16,
    TRMQuery_CancelAuction_BidderMail	= 17,
    TRMQuery_ExpiredListing				= 18,
    TRMQuery_GetAttributeDetails		= 19,
    TRMQuery_ProcessBidAuction			= 20,
    TRMQuery_ProcessAuctionRefund		= 21,
    TRMQuery_GetResAttributeDetails		= 22
};
enum TRMVendorType
{
    TRMVendor_bazaar	=	1,
    TRMVendor_player    =   2

};

//======================================================================================================================

enum TRMAuctionType
{
    TRMVendor_Auction   	=	0,
    TRMVendor_Instant       =   1,
    TRMVendor_Ended			=   2,
    TRMVendor_Deleted 		=   3,
    TRMVendor_Offer 		=   4,
    TRMVendor_Cancelled		=   5,
    TRMVendor_Sold  		=   6,
    TRMVendor_NotSold 		=   7
};



enum TRMAuctionWindowType
{
    TRMVendor_AllAuctions	=	2,
    TRMVendor_MySales       =   3,
    TRMVendor_MyBids		=   4,
    TRMVendor_AvailableItems=   5,
    TRMVendor_Offers		=   6,
    TRMVendor_ForSale		=   7,
    TRMVendor_Stockroom		=   8

};

enum TRMPermissionType
{
    TRMOwner        	=	0,
    TRMNotOwner         =   1,
    TRMBazaar           =   2,
    TRMBazaarQuery		=	3

};

enum TRMRegionType
{
    TRMGalaxy        	=	0,
    TRMPlanet	        =   1,
    TRMRegion           =   2,
    TRMVendor			=	3

};

//======================================================================================================================

struct DescriptionItem
{
    uint64			ItemID;
    int8			Description[1024];
    int8			tang[128];
    int8			details[1024];
};

//======================================================================================================================


//thats were the raw Auction db data goes
struct AuctionItem
{
    uint64			ItemID;
    uint64			OwnerID;
    uint64			BazaarID;
    uint32          AuctionTyp;
    uint64			EndTime;
    uint32          Premium;
    uint32			Category;
    uint32			ItemTyp;
    uint32			Price;
    int8			Name[128];
    int8			Description[1024];
    uint16			RegionID;
    uint64			BidderID;
    uint16			PlanetID;
    int8			SellerName[32];
    int8			BazaarName[128];
    uint32			MyBid;
    uint32			MyProxy;

    int8			bidder_name[32];
    uint32			HighProxy;
    uint32			HighBid;
    int8			HighProxyRaw[32];
    int8			HighBidRaw[32];
    int8			BidderIDRaw[32];
    int8			Owner[32];
    uint32			itemcategory;

};

class TradeManagerAsyncContainer
{
public:

    TradeManagerAsyncContainer(TRMQueryType qt,DispatchClient* client) {
        mQueryType = qt;
        mClient = client;
    }
    ~TradeManagerAsyncContainer() {}


    uint64				BuyerID;
    uint32				Itemsstart;
    uint32				Itemsstop;
    DescriptionItem*	mItemDescription;
    AuctionItem*		AuctionTemp;

    TRMQueryType		mQueryType;
    DispatchClient*		mClient;

    uint64				AuctionID;
    uint32				BazaarWindow;
    uint32				BazaarPage;
    uint64				BazaarID;
    uint32				MyBid;
    uint32				MyProxy;
    TangibleObject*		tangible;
    uint32				crc;
    uint32				time;
    BString				name;
    BString				mPlanet;
    BString				mRegion;
    BString				mOwner;

    //trading
    int32				amount1,amount2;//inv credits
    int32				amountbank,amountcash;
    int32				bank1;//bankcredits
    int32				bank2;
    PlayerObject*		player1;
    PlayerObject*		player2;
    uint64				sellerID;
    int32				mX;
    int32				mY;

    //TRMVendorType		mVendorType;
    TRMAuctionType		auctionType;

    uint8				premium;
    uint32				itemType;
    uint32				price;
    BString				description;
    BString				tang;
};

class Vendor
{
    long			ownerid;
    long			id;
    uint32			regionid;
    uint32			planetid;
    int8			string[128];

};


//======================================================================================================================

class TradeManager : public DatabaseCallback
{
    friend class ObjectFactory;
public:

    static TradeManager*	getSingletonPtr() {
        return mSingleton;
    }
    static TradeManager*	Init(Database* database,MessageDispatch* dispatch);

    ~TradeManager();

    void				Shutdown();

    virtual void		handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    void				addTradeInvitation(PlayerObject* invitedPlayer, PlayerObject* invitingPlayer);

    ItemFrogClass		mItemFrogClass;

private:

    TradeManager(Database* database,MessageDispatch* dispatch);

    void				_HandleAuctionCreateMessage(Message* message,DispatchClient* client, TRMAuctionType auction);

    void 				_processBankTipDeduct(Message* message,DispatchClient* client);
    void 				_processFindFriendCreateWaypointMessage(Message* message,DispatchClient* client);
    void 				_processFindFriendRequestPositionMessage(Message* message,DispatchClient* client);

    void				_processBanktipUpdate(Message* message,DispatchClient* client);

    void				_processHandleAuctionCreateMessage(Message* message,DispatchClient* client);
    void				_processHandleImmediateAuctionCreateMessage(Message* message,DispatchClient* client);
    void 				_processDeductMoneyMessage(Message* message,DispatchClient* client);


    void				_processCancelLiveAuctionMessage(Message* message,DispatchClient* client);
    void				_processCreateItemMessage(Message* message,DispatchClient* client);
    void				checkPlacedInstrument(Item* addedItem, DispatchClient* client);

    //Trade
    void				_processAbortTradeMessage(Message* message,DispatchClient* client);
    void				_processTradeCompleteMessage(Message* message,DispatchClient* client);
    void				_processAddItemMessage(Message* message,DispatchClient* client);
    void				informTradePartner(TangibleObject* Item,PlayerObject* TradePartner);
    void				_processRemoveItemMessage(Message* message,DispatchClient* client);
    void				_processAcceptTransactionMessage(Message* message,DispatchClient* client);
    void				_processUnacceptTransactionMessage(Message* message,DispatchClient* client);
    void				_processBeginVerificationMessage(Message* message,DispatchClient* client);
    void				_processVerificationMessage(Message* message,DispatchClient* client);
    void				_processGiveMoneyMessage(Message* message,DispatchClient* client);
    void				TradeTransaction(DispatchClient* client,PlayerObject* player1,PlayerObject* player2);


    static TradeManager*		mSingleton;
    static bool					mInsFlag;

    Database*					mDatabase;
    MessageDispatch*			mMessageDispatch;
    TRMPermissionType			mPermissionTyp;

    uint32						mErrorCount;

    uint32						mZoneId;


};



#endif

