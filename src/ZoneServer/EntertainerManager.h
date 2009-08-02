/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_ENTERTAINERMANAGER_H
#define ANH_ZONESERVER_ENTERTAINERMANAGER_H

#include "Common/MessageDispatchCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "ObjectFactory.h"
#include "ZoneOpcodes.h"
#include "Item.h"
#include "PlayerObject.h"
#include "PlayerEnums.h"
#include <vector>
#include "Utils/Scheduler.h"

#define 	gEntertainerManager	EntertainerManager::getSingletonPtr()

//======================================================================================================================

class Message;
class Database;
class MessageDispatch;
class PlayerObject;

//======================================================================================================================


enum EMQueryType
{
	EMQuery_NULL				=	0,
	EMQuery_LoadPerformances	=	1,
	EMQuery_DenyServiceFindName	=	2,
	EMQuery_DenyServiceListNames=	3,
	EMQuery_LoadIDAttributes	=	4,
	EMQuery_IDMoneyTransaction	=	5,
	EMQuery_IDMigrateStats		=	6,
	EMQuery_LoadHoloEmotes		=	7,
	EMQuery_IDFinances			=	8,
	
};

//======================================================================================================================

enum EMTimer
{
	EMTimer_PerformanceTick		=	0,
};



//======================================================================================================================

struct PerformanceStruct
{
	int8			performanceName[32];
	uint32			instrumentAudioId;
	uint32			requiredInstrument;
	uint32			danceVisualId;
	int32			actionPointPerLoop;
	uint32			loopDuration;
	uint32			type;
	uint32			florishXpMod;
	uint32			healMindWound;
	uint32			healShockWound;
	uint32			musicVisualId;
};

//======================================================================================================================

struct IDStruct
{
	//SELECT CRC, Atr1ID, Atr1Name, Atr2ID, Atr2Name, name FROM swganh.id_attributes");
	uint32			CustomizationCRC;
	uint32			SpeciesCRC;
	uint32			Atr1ID;
	int8			Atr1Name[12];
	uint32			Atr2ID;
	int8			Atr2Name[12];
	uint32			XP;
	uint8			hair;
	uint32			divider;
};

//======================================================================================================================

struct HoloStruct
{
	uint32 pCRC;
	uint32 pId;
	int8   pEmoteName[64];
};

//======================================================================================================================

struct ModifierStruct
{
	uint32 pHealingDanceMindMod; 
	int32 pHealingDanceShockMod; 
	uint32 pHealingDanceWoundMod;
	uint32 pHealingMusicMindMod; 
	int32 pHealingMusicShockMod; 
	uint32 pHealingMusicWoundMod;
};


//======================================================================================================================

typedef std::map<uint64,BuffStruct*>		BuffMap;
typedef std::vector<PerformanceStruct*>		PerformanceList;
typedef std::vector<IDStruct*>				IdList;
typedef std::vector<CreatureObject*>		AudienceList;
typedef std::vector<HoloStruct*>			HoloEmoteEffects;

//======================================================================================================================

class EntertainerManagerAsyncContainer
{
public:

	EntertainerManagerAsyncContainer(EMQueryType qt,DispatchClient* client){ mQueryType = qt; mClient = client; }
	~EntertainerManagerAsyncContainer(){}

	EMQueryType			mQueryType;
	DispatchClient*		mClient;	

	PlayerObject*		performer;
	PlayerObject*		customer;
	string				outCastName;
	int32				amountcash;
	int32				amountbank;
};

//======================================================================================================================

class EntertainerManager : public DatabaseCallback,public ObjectFactoryCallback
{
	friend class ObjectFactory;
	friend class PlayerObject;

	public:
		//System

		static EntertainerManager*	getSingletonPtr() { return mSingleton; }
		static EntertainerManager*	Init(Database* database,MessageDispatch* dispatch);

		~EntertainerManager();

		void					Shutdown();

		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void					handleObjectReady(Object* object,DispatchClient* client);

		//=========================================================
		//=========================================================
		//get db data
		IDStruct*				getIDAttribute(uint32 CustomizationCRC,uint32 SpeciesCRC);
		IDStruct*				getIDAttribute(uint32 CustomizationCRC);
		HoloStruct*				getHoloEmoteIdbyCRC(uint32 crc);
		string					getHoloNames();
		HoloStruct*				getHoloEmoteIdbyName(string name);
		PerformanceStruct*		getPerformance(string performance);
		PerformanceStruct*		getPerformance(string performance,uint32 type);
		
		//=========================================================
		//=========================================================
		// Entertainer
		
		//Music
		void					startMusicPerformance(PlayerObject* pEntertainer,string performance);
		void					changeMusic(PlayerObject* entertainer,string performance);

		void					stopListening(PlayerObject* audience,bool ooRange = false);
		void					startListening(PlayerObject* audience, PlayerObject* entertainer);

		void					useInstrument(PlayerObject* pPlayerObject, Item* pItem);
		//  Private
		// void					playInstrument(PlayerObject* entertainer, Item* pInstrument);
		void					usePlacedInstrument(PlayerObject* entertainer, Item* usedInstrument);
		void					playPlacedInstrument(PlayerObject* entertainer);

		void					handlestartmusic(PlayerObject* performer);
		bool					handleStartBandIndividual(PlayerObject* performer, string performance);
		bool					handleStartBandDanceIndividual(PlayerObject* performer, string performance);
		uint64					gettargetedInstrument(PlayerObject* entertainer);
		bool					checkInstrumentSkillbyType(PlayerObject* entertainer,uint32 instrumentType);
		bool					checkInstrumentSkill(PlayerObject* entertainer,uint64 instrumentID);

		//Dance
		void					startDancePerformance(PlayerObject* mPerformer,string performance);
		void					changeDance(PlayerObject* entertainer,string performance);

		void					stopWatching(PlayerObject* audience,bool ooRange = false);
		void					startWatching(PlayerObject* audience, PlayerObject* entertainer);
		
		//Audience
		bool					checkAudience(PlayerObject* entertainer,CreatureObject* audience);
		void					addAudience(PlayerObject* entertainer,CreatureObject* audience);
		void					addOutcastName(PlayerObject* entertainer,PlayerObject* outcast);
		void					toggleOutcastId(PlayerObject* entertainer,uint64 outCastId, string outCastName);
		void					verifyOutcastName(PlayerObject* entertainer,string outCastName);
		void					removeAudience(PlayerObject* entertainer,CreatureObject* mAudience);

		bool					checkDenyServiceList(PlayerObject* audience, PlayerObject* entertainer);	
		void					showOutcastList(PlayerObject* entertainer);

		
		
		bool					handlePerformanceTick(CreatureObject* mObject);
		void					handlePerformancePause(CreatureObject* mObject);
		void					stopEntertaining(PlayerObject* entertainer);
		void					grantXP(PlayerObject* entertainer);
		void					heal(PlayerObject* entertainer);
		void					buff(PlayerObject* entertainer);
		ModifierStruct			getGroupHealSkillValues(PlayerObject* entertainer);
		
		void					CheckDistances(PlayerObject* entertainer);
		void					flourish(PlayerObject* entertainer, uint32 mFlourishId);
		void					entertainInRangeNPCs(PlayerObject* entertainer);

		//=========================================================
		//=========================================================
		// Imagedesigner
		void					commitIdChanges(PlayerObject* customer,PlayerObject* designer, string hair, uint32 amount,uint8 statMigration,string holoEmote,uint8 flagHair);
		string					commitIdAttribute(PlayerObject* customer, string attribute, float value);
		string					commitIdColor(PlayerObject* customer, string attribute, uint16 value);
		string					commitIdheight(PlayerObject* customer, float value);
		uint32					getIdXP(string attribute, uint16 value);
		void					applyHoloEmote(PlayerObject* customer,string holoEmote);
		void					applyHair(PlayerObject* customer,string hair);
		void					applyMoney(PlayerObject* customer,PlayerObject* designer,int32 amount);

	private:

		EntertainerManager(Database* database,MessageDispatch* dispatch);
		
		void					playInstrument(PlayerObject* entertainer, Item* pInstrument);
		uint64					getInstrument(PlayerObject* entertainer);
		bool					approachInstrument(PlayerObject* entertainer, uint64 instrumentId);


	
		static EntertainerManager*	mSingleton;
		static bool					mInsFlag;

		Database*					mDatabase;
		MessageDispatch*			mMessageDispatch;
		
		PerformanceStruct*			mPerformanceHandler;
		PerformanceList				mPerformanceList;
		IdList						mIDList;
		HoloEmoteEffects			mHoloList;
};

#endif 

