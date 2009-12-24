/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_PLAYER_OBJECT_H
#define ANH_ZONESERVER_PLAYER_OBJECT_H

#include "CreatureObject.h"
#include "EntertainerManager.h"
#include "Stomach.h"
#include "Trade.h"
#include "PlayerEnums.h"
#include "CraftingSession.h"
#include "SurveyEvent.h"
#include "SampleEvent.h"
#include "LogOutEvent.h"
#include "Common/DispatchClient.h"
#include <map>

class BazaarTerminal;
class BuildingObject;
class Tutorial;
class TravelTerminal;

//=============================================================================

typedef std::vector<std::pair<string,float> >	AttributesList;
typedef std::vector<std::pair<string,uint16> >	ColorList;
typedef std::vector<std::pair<uint32,int32> >	XPList;
typedef std::vector<std::pair<uint32,int32> >	XPCapList;
typedef std::vector<uint64>						SchematicsIdList;
typedef std::vector<uint64>						DenyServiceList;
typedef std::vector<uint64>						GroupMissionList;
typedef std::vector<CreatureObject*>			AudienceList;
typedef std::map<uint64,BuffStruct*>			BuffMap;
typedef std::vector<PlayerObject*>				PlayerList;
typedef std::vector<uint32>						BadgesList;
typedef std::vector<uint32>						UIWindowList;
typedef std::map<uint32,BString>				ContactMap;

//=============================================================================

class PlayerObject : public CreatureObject
{
	friend class PlayerObjectFactory;
	friend class ObjectController;

	public:

		PlayerObject();
		~PlayerObject();

		DispatchClient*	getClient() const { return mClient; }
		void				setClient(DispatchClient* client){ mClient = client; }

		Tutorial*			getTutorial() const { return mTutorial; }
		void				stopTutorial();
		void				startTutorial();

		virtual void		prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		uint32				getAccountId() const{ return mAccountId; }

		PlayerConnState		getConnectionState(){ return mConnState; }
		void				setConnectionState(PlayerConnState state){ mConnState = state; }
		bool				isConnected() const { return(mConnState == PlayerConnState_Connected); }
		bool				isLinkDead(){ return(mConnState == PlayerConnState_LinkDead); }
		bool				isBeingDestroyed(){ return(mConnState == PlayerConnState_Destroying); }

		uint32*				getDisconnectTime(){ return &mDConnTime; }
		void				setDisconnectTime(uint32 time){ mDConnTime = time; }

		uint32				getClientTickCount(){ return mClientTickCount; }
		void				setClientTickCount(uint32 tickCount){ mClientTickCount = tickCount; }

		string				getTitle() const { return mTitle; }
		void				setTitle(const string title){ mTitle = title; }

		uint64				getPlayerObjId(){ return mPlayerObjId; }
		void				setPlayerObjId(uint64 id){ mPlayerObjId = id; }

		void				setTravelPoint(TravelTerminal* tp){ mTravelPoint = tp; }
		TravelTerminal*		getTravelPoint(){ return mTravelPoint; }

		void				setMotdReceived(bool b){ mMotdReceived = b; }
		bool				getMotdReceived(){ return mMotdReceived; }

		bool				checkXpType(uint32 xpType);

		// Here is where we shall add restrictions for JTL, Jedi, Pre-Pub 14
		// or other restrictions for type of XP to allow. Return true if xpType is restricted.
		bool				restrictedXpType(uint32 xpType);

		int32				getXpAmount(uint32 xpType);
		void				addXpType(uint32 xpType,int32 value = 0);
		bool				removeXpType(uint32 xpType);
		bool				UpdateXp(uint32 xpType,int32 value);

		bool				checkXpCapType(uint32 xpType);
		int32				getXpCapAmount(uint32 xpType);
		bool				removeXpCapType(uint32 xpType);
		void				addXpCapType(uint32 xpType,int32 value = 0);
		bool				UpdateXpCap(uint32 xpType,int32 value);

		XPList*				getXpList(){ return &mXpList; }
		XPList*				getXpCapList() { return &mXpCapList; };

		Stomach*			getStomach(){ return mStomach; }

		uint32				getJediState(){ return mJediState; }
		void				setJediState(uint32 js){ mJediState = js; }

		// survey
		bool				getSurveyState(){ return mSampleNode.mPendingSurvey; }
		void				setSurveyState(bool state){ mSampleNode.mPendingSurvey = state; }

		// sample
		bool				getSamplingState(){ return mSampleNode.mPendingSample; }
		void				setSamplingState(bool state){ mSampleNode.mPendingSample = state; }
		uint64				getNextSampleTime(){ return mSampleNode.mNextSampleTime; }
		void				setNextSampleTime(uint64 time){ mSampleNode.mNextSampleTime = time; }
		SampleNode*			getSampleData(){return &mSampleNode;}

		// Charsheet
		uint32				getPlayerMatch(uint8 num){ return mPlayerMatch[num]; }
		void				setPlayerMatch(uint8 num,uint32 match){ mPlayerMatch[num] = match; }

		uint8				getCsrTag(){ return mCsrTag; }
		void				setCsrTag(uint8 csrTag){ mCsrTag = csrTag; }

		void				togglePlayerFlagOn(uint32 flag){ mPlayerFlags = mPlayerFlags | flag; }
		void				togglePlayerFlagOff(uint32 flag){ mPlayerFlags = mPlayerFlags & ~flag; }
		void				togglePlayerFlag(uint32 flag){ mPlayerFlags = mPlayerFlags ^ flag; }
		bool				checkPlayerFlag(uint32 flag){ return((mPlayerFlags & flag) == flag); }
		bool				checkPlayerFlags(uint64 flags){ return((mPlayerFlags & flags) == flags); }
		uint32				getPlayerFlags() const { return mPlayerFlags; }
		void				setPlayerFlags(uint32 flags){ mPlayerFlags = flags; }

		BadgesList*			getBadges(){ return &mBadgeList;}
		bool				checkBadges(uint32 badgeId);
		void				addBadge(uint32 badgeId);

		void				setBiography(const string bio){ mBiography = bio; }
		string				getBiography(){ return mBiography; }

		void				setMarriage(const string marriage){ mMarriage = marriage; }
		string				getMarriage(){ return mMarriage; }

		uint32				getBornyear(){ return mBornyear; }
		void				setBornyear(uint32 bornyear){ mBornyear = bornyear; }

		int8				getBindPlanet(){ return mBindPlanet; }
		void				setBindPlanet(int8 planetId){ mBindPlanet = planetId; }

		int8				getHomePlanet(){ return mHomePlanet; }
		void				setHomePlanet(int8 planetId){ mHomePlanet = planetId; }

		Anh_Math::Vector3	getBindCoords(){ return mBindCoords; }
		void				setBindCoords(const Anh_Math::Vector3 coords){ mBindCoords = coords; }

		Anh_Math::Vector3	getHomeCoords(){ return mHomeCoords; }
		void				setHomeCoords(const Anh_Math::Vector3 coords){ mHomeCoords = coords; }

		// UI Windows
		void				handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);
		UIWindowList*		getUIWindowList(){ return &mUIWindowList; }
		void				addUIWindow(uint32 id){ mUIWindowList.push_back(id); }
		bool				removeUIWindow(uint32 id);
		void				clearAllUIWindows();
		void				resetUICallbacks(Object* object);
		bool				checkUIWindow(string text);

		ContactMap*			getFriendsList(){ return &mFriendsList; }
		void				addFriend(string name){ mFriendsList.insert(std::make_pair(name.getCrc(),name.getAnsi())); }
		bool				removeFriend(uint32 nameCrc);
		bool				checkFriendList(uint32 nameCrc);

		ContactMap*			getIgnoreList(){ return &mIgnoreList; }
		void				addIgnore(string name){ mIgnoreList.insert(std::make_pair(name.getCrc(),name.getAnsi())); }

		bool				removeIgnore(uint32 nameCrc);
		bool				checkIgnoreList(uint32 nameCrc) const;

		uint32				getFriendsListUpdateCounter(){ return mFriendsListUpdateCounter; }
		void				advanceFriendsListUpdateCounter(uint32 count){ mFriendsListUpdateCounter += count; }
		uint32				getIgnoresListUpdateCounter(){ return mIgnoresListUpdateCounter; }
		void				advanceIgnoresListUpdateCounter(uint32 count){ mIgnoresListUpdateCounter += count; }

		bool				getContactListUpdatePending(){ return mContactListUpdatePending; }
		void				setContactListUpdatePending(bool b){ mContactListUpdatePending = b; }

		virtual	void		handleObjectMenuSelect(uint8 messageType,Object* srcObject);

		// Entertainment

		EMLocationType		getPlayerLocation();
		uint64				getPlacedInstrumentId(){return mPlacedInstrument;}
		void				setPlacedInstrumentId(uint64 id){mPlacedInstrument = id;}
		uint64				getEntertainerWatchToId(){return mEntertainerWatchToId;}
		void				setEntertainerWatchToId(uint64 entertainer){mEntertainerWatchToId = entertainer;}
		uint64				getEntertainerTaskId(){return mEntertainerTaskId;}
		void				setEntertainerTaskId(uint64 entertainerTaskId){mEntertainerTaskId = entertainerTaskId;}
		uint64				getEntertainerPauseId(){return mEntertainerPauseId;}
		void				setEntertainerPauseId(uint64 entertainerPauseId){mEntertainerPauseId = entertainerPauseId;}

		void				setActiveInstrumentId(uint64 instrumentId) {mSelectedInstrument = instrumentId;}
		uint64				getActiveInstrumentId(void) {return mSelectedInstrument;}

		BuffMap*			getEntertainerBuffMap(){return &mEntertainerBuffMap;}
		AudienceList*		getAudienceList() { return &mAudienceList; }
		DenyServiceList*	getDenyAudienceList(){ return &mDenyAudienceList; }

		uint8				getFlourishCount(){return mFlourishCount;}
		void				setFlourishCount(uint8 flourishCount){mFlourishCount = flourishCount;}

		uint8				getGroupMusicXp(){return static_cast<uint8>(mGroupXp);}
		void				setGroupMusicXp(uint8 groupXp){mGroupXp = groupXp;}

		// ID
		bool				UpdateIdAttributes(BString attribute,float value);
		bool				UpdateIdColors(BString attribute,uint16 value);
		AttributesList*		getIdAttributesList(){return &mIDAttributesList;}
		ColorList*			getIdColorList(){return &mIDColorList;}
		void				SetImageDesignSession(IDSession theSession){mIDSession = theSession;}
		IDSession 			getImageDesignSession(){return mIDSession;}
		uint32				getHoloEmote(){ return mHoloEmote; }
		void				setHoloEmote(uint32 emote){ mHoloEmote = emote; }
		uint32				getHoloCharge(){ return mHoloCharge; }
		void				setHoloCharge(uint32 emote){ mHoloCharge = static_cast<uint8>(emote); }

		// trade
		void				giveBankCredits(uint32 amount);
		void	            giveInventoryCredits(uint32 amount);
		Trade*				getTrade(){return mTrade;}
		void				setTradePartner(uint64 id){mTradePartner = id;}
		uint64				getTradePartner(){return mTradePartner;}
		bool				getTradeStatus(){return mTrading;}
		void				setTradeStatus(bool tradeStatus){mTrading = tradeStatus;}
		bool				deductCredits(int32 amount);
		bool				testCash(int32 amount);
		bool				testBank(int32 amount);
		bool				checkDeductCredits(int32 amount);

		void				setGender(bool gender){mFemale = gender;}
		bool				getGender(){return mFemale;}

		bool				checkIfMounted() { return mMounted; }
		void				setMounted(bool mounted) { mMounted = mounted; }

		CreatureObject*		getMount() { return mMount; }
		void				setMount(CreatureObject* mount) { mMount = mount; }

		bool				checkIfMountCalled() { return mMountCalled; }
		void				setMountCalled(bool mount_called) { mMountCalled = mount_called; }


		// crafting
		CraftingStation*	getCraftingStation(ObjectSet	inRangeObjects, ItemType	toolType);
		uint32				getCraftingStage(){ return mCraftingStage; }
		void				setCraftingStage(uint32 stage){ mCraftingStage = stage; }
		uint32				getExperimentationFlag(){ return mExperimentationFlag; }
		void				setExperimentationFlag(uint32 expFlag){ mExperimentationFlag = expFlag; }
		uint32				getExperimentationPoints(){ return mExperimentationPoints; }
		void				setExperimentationPoints(uint32 points){mExperimentationPoints = points; }
		uint64				getNearestCraftingStation(){ return mNearestCraftingStation; }
		void				setNearestCraftingStation(uint64 station){ mNearestCraftingStation = station; }
		CraftingSession*	getCraftingSession(){ return mCraftingSession; }
		void				setCraftingSession(CraftingSession* craftingSession){ mCraftingSession = craftingSession; }
		void				prepareSchematicIds();
		void				addSchematicIds(Skill* skill);
		bool				checkSchematicId(uint64 id);
		SchematicsIdList*	getSchematicsIdList(){ return &mSchematicIdList; }
		SchematicsIdList*	getFilteredSchematicsIdList(){ return &mFilteredSchematicIdList; }

		SchematicsIdList*	getSchematicsAddList(){ return &mSchematicAddList; }

		// groups
		PlayerList			getInRangeGroupMembers(bool self = false) const;
		uint64				getLastGroupMissionUpdateTime(){return mLastGroupMissionUpdateTime;}
		void				setLastGroupMissionUpdateTime(uint64 time){mLastGroupMissionUpdateTime = time;}

		// duels
		PlayerList*			getDuelList(){ return &mDuelList; }
		bool				checkDuelList(PlayerObject* player);
		void				addToDuelList(PlayerObject* player){ mDuelList.push_back(player); }
		void				removeFromDuelList(PlayerObject* player);
		void				clearDuelList(){ mDuelList.clear(); }

		// reset properties on zoning
		void				resetProperties();

		// event functions
		void				onSurvey(const SurveyEvent* event);
		void				onSample(const SampleEvent* event);
		void				onLogout(const LogOutEvent* event);

		// cloning
		void				clone(uint64 parentId,Anh_Math::Quaternion dir,Anh_Math::Vector3 pos);
		void				setPreDesignatedCloningFacilityId(uint64 cloningId);
		uint64				getPreDesignatedCloningFacilityId(void);
		void				saveNearestCloningFacility(BuildingObject* nearestCloningFacility);
		void				cloneAtNearestCloningFacility(void);

		// newbie
		uint8				getNewPlayerExemptions(void);
		void				setNewPlayerExemptions(uint8 npe, bool displayMessage=false);
		void				newPlayerMessage(void);

		//Lots
		uint8				getLots(){ return mLots; }
		void				setLots(uint8 lots){ mLots = lots; }
		
		bool				checkLots(uint8 check){int16 intCheck = mLots -check;return (intCheck>=0);}
		bool				useLots(uint8 usedLots);
		bool				regainLots(uint8 lots);
		
		void				setStructurePermissionId(uint64 id){mPermissionId = id;}
		uint64				getStructurePermissionId(){return mPermissionId;}


		uint32				mXpUpdateCounter;

		//Player Specific Buffs
		void InitialiseBuffs();

		//Missions
		uint16 *			getMissionIdMask() { return &mMissionIdMask; }
		void				setmissionIdMask(uint16 mask) { mMissionIdMask = mask; }

		// Targetting
		uint64				getCombatTargetId(void);
		void				setCombatTargetId(uint64 targetId);
		void				enableAutoAttack(void);
		void				disableAutoAttack(void);
		bool				autoAttackEnabled(void);

	private:

		void				_verifyBadges();
		void				_verifyExplorationBadges();

		AudienceList			mAudienceList;
		BadgesList				mBadgeList;
		DenyServiceList		mDenyAudienceList;
		PlayerList				mDuelList;
		SchematicsIdList	mFilteredSchematicIdList;
		AttributesList		mIDAttributesList;
		ColorList					mIDColorList;
		SchematicsIdList	mSchematicAddList;
		SchematicsIdList	mSchematicIdList;
		UIWindowList			mUIWindowList;
		XPCapList					mXpCapList;
		XPList						mXpList;

		BuffMap 					mEntertainerBuffMap;
		ContactMap				mFriendsList;
		ContactMap 				mIgnoreList;

		string				mBiography;
		string				mMarriage;
		string				mTitle;

		Anh_Math::Vector3	mBindCoords;
		Anh_Math::Vector3	mHomeCoords;

		uint32				mPlayerMatch[4];

		BazaarTerminal*     mBazaarPoint;
		DispatchClient*		mClient;
		CraftingSession*	mCraftingSession;
		CreatureObject*		mMount;
							
		// Default cloningfacility if revive timer expires.
		BuildingObject*		mNearestCloningFacility; 
		Stomach*			mStomach;
		Trade*				mTrade;
		TravelTerminal*		mTravelPoint;
		Tutorial*			mTutorial;

		//the data for the sample node game
		SampleNode			mSampleNode;

		PlayerConnState		mConnState;

		//logs whether weve got an IDsession running at this point
		IDSession			mIDSession; 

		uint64				mCombatTargetId; // The actual target player are hitting, not always the same as the "look-at" target.
		uint64				mEntertainerPauseId;
		uint64				mEntertainerTaskId;
		uint64				mEntertainerWatchToId;
		uint64				mLastGroupMissionUpdateTime;
		uint64				mNearestCraftingStation;
		uint64				mPlacedInstrument;
		uint64				mPlayerObjId;
		uint64				mPreDesignatedCloningFacilityId;
		uint64				mSelectedInstrument;
		uint64				mTradePartner;
		
		// ID of the structure were setting the Premission for - as this is no callback and the Objectcontroller doesnt always provide us with an ID
		uint64				mPermissionId;	


		uint32				mAccountId;
		uint32				mBornyear;
		uint32				mClientTickCount;
		uint32				mCraftingStage;
		uint32				mDConnTime;
		uint32				mExperimentationFlag;
		uint32				mExperimentationPoints;
		uint32				mFriendsListUpdateCounter;
		uint32				mHoloEmote;
		uint32 				mIgnoresListUpdateCounter;
		uint32				mJediState;
		uint32				mPlayerFlags;
		uint16				mGroupXp;
		uint16				mMissionIdMask;
		int8					mBindPlanet;
		int8					mHomePlanet;
		uint8					mCsrTag;
		uint8					mFlourishCount;
		uint8					mHoloCharge;
		uint8					mLots;
		uint8					mNewPlayerExemptions;
		bool				mAutoAttack;
		bool				mContactListUpdatePending;
		bool				mFemale;
		bool				mMotdReceived;
		bool				mMountCalled;
		bool				mMounted;
		bool				mNewPlayerMessage;
		
		bool				mTrading;
};



//=============================================================================

#endif



