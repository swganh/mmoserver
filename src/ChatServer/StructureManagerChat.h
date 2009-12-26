/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CHATSERVER_StructureManager_H
#define ANH_CHATSERVER_StructureManager_H

#include "ChatManager.h"
#include "ChatMessageLib.h"
#include "TradeManagerHelp.h"

#include "DatabaseManager/DatabaseCallback.h"

#include "Common/MessageDispatchCallback.h"

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

#define	gStructureManager	StructureManager::getSingletonPtr()

//======================================================================================================================

typedef std::queue<uint32> TimerEventQueue;

class ChatManager;
class StructureManagerAsyncContainer;
class CommoditiesClass;
class Database;
class Message;
class MessageDispatch;
class Player;
class Timer;
//struct HarvesterHopperItem;
struct HarvesterItem;


//typedef std::map<uint32,Player*>		PlayerAccountMap;
//typedef std::vector<Bazaar*>		BazaarList;

typedef std::vector<HarvesterItem*>					HarvesterList;
typedef std::vector<std::tr1::shared_ptr<Timer> >	TimerList;
//typedef std::vector<HarvesterHopperItem*>			HopperResourceList;


enum SRMTimer
{
	SRMTimer_CheckHarvesterHopper		=	1,
	SRMTimer_CheckHarvesterMaintenance	=	2,
	SRMTimer_CheckHarvesterPower		=	3
};


struct structure
{
	uint64 owner;
	string file;
	string dir;
	float x;
	float z;
	string planet;
	uint32 maxcondition;
	uint32 condition;
	uint32 maint;
	uint64 lastMail;
};

//======================================================================================================================

class StructureManagerChatHandler : public MessageDispatchCallback, public DatabaseCallback, public TimerCallback
{
	public:

		static StructureManagerChatHandler*	getSingletonPtr() { return mSingleton; }
		static StructureManagerChatHandler*	Init(Database* database,MessageDispatch* dispatch, ChatManager* chatManager);

		~StructureManagerChatHandler();

		StructureManagerChatHandler(Database* database,MessageDispatch* dispatch, ChatManager* chatManager);

		void				Shutdown();
		void				Process();
				// TimerCallback
		virtual void		handleTimer(uint32 id, void* container);

		virtual void		handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client);
		virtual void		handleDatabaseJobComplete(void* ref,DatabaseResult* result);


	private:

		void				ProcessAddHarvesterHopperUpdate(Message* message,DispatchClient* client);

		// process chat timers
		void				processTimerEvents();
		void				handleGlobalTickUpdate();

		void				handleCheckHarvesterPower();
		void				handleCheckHarvesterHopper();
		void				handleCheckHarvesterMaintenance();


		HarvesterList*		getHarvesterList(){return &mHarvesterList;}


		static StructureManagerChatHandler*	mSingleton;
		static bool					mInsFlag;

		
		Database*					mDatabase;
		MessageDispatch*			mMessageDispatch;
		PlayerAccountMap			mPlayerAccountMap;

		ChatManager*				mChatManager;

		TimerList					mTimers;
		TimerEventQueue				mTimerEventQueue;
        boost::mutex                mTimerMutex;
		uint64						mTimerQueueProcessTimeLimit;

		HarvesterList				mHarvesterList;

};

enum STRMQueryType
{
	STRMQuery_NULL						=	0,
	STRMQuery_HopperUpdate				=	1,
	STRMQuery_DoneHarvestUpdate			=	2,
	STRMQuery_DoneHarvesterUsePower		=	3,
	STRMQuery_DoneHarvesterMaintenance	=	4,
	STRMQuery_StructureMailOOFMaint		=	5,
	STRMQuery_StructureMailDamage		=	6,
	STRMQuery_StructureMailCondZero		=	7,
	STRMQuery_MaintenanceUpdate			=	8,
	STRMQuery_PowerUpdate				=	9,

};

class StructureManagerAsyncContainer
{
public:

	StructureManagerAsyncContainer(STRMQueryType qt,DispatchClient* client){ mQueryType = qt; mClient = client; }
	~StructureManagerAsyncContainer(){}


	STRMQueryType		mQueryType;
	DispatchClient*		mClient;
	uint64				harvesterID;
	uint32				updateCounter;

};

struct HarvesterHopperItem
{
		uint64			HarvesterID;
		uint64			ResourceID;
		float			Quantity;
};

struct HarvesterItem
{
		uint64			HarvesterID;
		uint32			UpdateCounter;
		uint32			PlayerAccount;
};


#endif

