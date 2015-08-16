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

#ifndef ANH_CHATSERVER_TradeManager_H
#define ANH_CHATSERVER_TradeManager_H

#include "ChatManager.h"
#include "ChatMessageLib.h"
#include "TradeManagerHelp.h"

#include "DatabaseManager/DatabaseCallback.h"

#include "Utils/TimerCallback.h"

#include <boost/thread/mutex.hpp>

#include <queue>
#include <vector>

#if defined(__GNUC__)
// GCC implements tr1 in the <tr1/*> headers. This does not conform to the TR1
// spec, which requires the header without the tr1/ prefix.
#include <tr1/memory>
#else
#include <memory>
#endif

#define	gTradeManagerChat	TradeManagerChatHandler::getSingletonPtr()

//======================================================================================================================

typedef std::queue<uint32> TimerEventQueue;

class AuctionClass;
class ChatManager;
class CommoditiesClass;
class Database;
class Message;
class MessageDispatch;
class Player;
class Timer;

struct AuctionItem;
struct Bazaar;

//typedef std::map<uint32,Player*>		PlayerAccountMap;
typedef std::vector<Bazaar*>		BazaarList;
//typedef std::vector<Timer*>			TimerList;
typedef std::vector<std::tr1::shared_ptr<Timer> > TimerList;
typedef std::vector<AuctionItem*>	AuctionList;

//======================================================================================================================

class TradeManagerChatHandler : public DatabaseCallback, public TimerCallback
{
public:

    static TradeManagerChatHandler*	getSingletonPtr() {
        return mSingleton;
    }
    static TradeManagerChatHandler*	Init(Database* database,MessageDispatch* dispatch, ChatManager* chatManager);

    ~TradeManagerChatHandler();

    TradeManagerChatHandler(Database* database,MessageDispatch* dispatch, ChatManager* chatManager);

    void				Shutdown();
    void				Process();
    // TimerCallback
    virtual void		handleTimer(uint32 id, void* container);

    virtual void		handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    uint64				getGlobalTickCount() {
        return mGlobalTickCount;
    }

    CommoditiesClass*	Commodities;

    uint32				TerminalRegionbyID(uint64 id);
    uint32				getBazaarRegion(uint64 ID);
    BString				getBazaarString(uint64 ID);
    Bazaar*				getBazaarInfo(uint64 ID);


private:

    void				processRetrieveAuctionItemMessage(Message* message,DispatchClient* client);
    void				processCreateItemMessage(Message* message,DispatchClient* client);
    void				processHandleIsVendorMessage(Message* message,DispatchClient* client);
    void				processHandleopAuctionQueryHeadersMessage(Message* message,DispatchClient* client);
    void				processGetAuctionDetails(Message* message,DispatchClient* client);
    void				processCancelLiveAuctionMessage(Message* message,DispatchClient* client);
    void				processBidAuctionMessage(Message* message,DispatchClient* client);
    void				ProcessCreateAuction(Message* message,DispatchClient* client);
    void				processAuctionBid(TradeManagerAsyncContainer* asynContainer, Player* player);
    void				ProcessRequestTypeList(Message* message,DispatchClient* client);

    void				ProcessBankTip(Message* message,DispatchClient* client);
    void				processAuctionEMails(AuctionItem* AuctionTemp);

    // process chat timers
    void				handleGlobalTickPreserve();
    void				processTimerEvents();
    void				handleGlobalTickUpdate();
    void				handleCheckAuctions();



    static TradeManagerChatHandler*	mSingleton;
    static bool					mInsFlag;

    BazaarList					mBazaars;
    AttributesList				mAtrributesList;

    AuctionClass*				auction;

    ListStringStruct*			ListStringHandler;
    ListStringList				mListStringList;

    Database*					mDatabase;
    MessageDispatch*			mMessageDispatch;
    TRMPermissionType			mPermissionTyp;
    bool						mBazaarsLoaded;
    uint32						mBazaarCount;
    PlayerAccountMap			mPlayerAccountMap;

    ChatManager*				mChatManager;

    uint64      				mGlobalTickCount;
    TimerList					mTimers;
    TimerEventQueue				mTimerEventQueue;
    boost::mutex                mTimerMutex;
    uint64						mTimerQueueProcessTimeLimit;
    uint32						mBazaarMaxBid;

    AuctionList					mAuction;



};

#endif

