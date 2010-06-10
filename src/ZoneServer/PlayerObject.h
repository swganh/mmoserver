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

#ifndef ANH_ZONESERVER_PLAYER_OBJECT_H
#define ANH_ZONESERVER_PLAYER_OBJECT_H

#include "CreatureObject.h"
#include "EntertainerManager.h"
#include "Stomach.h"
#include "Trade.h"
#include "MountObject.h"
#include "PlayerEnums.h"
#include "SurveyEvent.h"
#include "SampleEvent.h"
#include "LogOutEvent.h"
#include "BurstRunEvent.h"
#include "ItemDeleteEvent.h"
#include "InjuryTreatmentEvent.h"
#include "QuickHealInjuryTreatmentEvent.h"
#include "WoundTreatmentEvent.h"
#include "Common/DispatchClient.h"
#include <map>

class BazaarTerminal;
class BuildingObject;
class Tutorial;
class TravelTerminal;
class CraftingStation;
class CraftingSession;
class Datapad;
//=============================================================================

typedef std::list<std::pair<string,float> >	AttributesList;
typedef std::list<std::pair<string,uint16> >	ColorList;
typedef std::list<std::pair<uint32,int32> >	XPList;
typedef std::list<std::pair<uint32,int32> >	XPCapList;
typedef std::vector<uint64>						SchematicsIdList;
typedef std::list<uint64>						DenyServiceList;
typedef std::list<uint64>						GroupMissionList;
typedef std::list<CreatureObject*>			AudienceList;
typedef std::map<uint64,BuffStruct*>			BuffMap;
typedef std::list<PlayerObject*>				PlayerList;
typedef std::list<uint32>						BadgesList;
typedef std::list<uint32>						UIWindowList;
typedef std::map<uint32,BString>				ContactMap;

//=============================================================================

class PlayerObject : public CreatureObject
{
	friend class PlayerObjectFactory;
	friend class ObjectController;

	public:

		PlayerObject();
		~PlayerObject();

		DispatchClient*		getClient() const { return mClient; }
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

		//datapad
		Datapad*			getDataPad(){return mDataPad;}
		void				setDataPad(Datapad* pad){mDataPad = pad;}

		void				setMotdReceived(bool b){ mMotdReceived = b; }
		bool				getMotdReceived(){ return mMotdReceived; }

		bool				checkXpType(uint32 xpType);

		virtual void		setParentIdIncDB(uint64 parentId);

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

		void				togglePlayerCustomFlagOn(uint32 flag){ mPlayerCustomFlags = mPlayerCustomFlags | flag; }
		void				togglePlayerCustomFlagOff(uint32 flag){ mPlayerCustomFlags = mPlayerCustomFlags & ~flag; }
		void				togglePlayerCustomFlag(uint32 flag){ mPlayerCustomFlags = mPlayerCustomFlags ^ flag; }
		bool				checkPlayerCustomFlag(uint32 flag){ return((mPlayerCustomFlags & flag) == flag); }
		bool				checkPlayerCustomFlags(uint64 flags){ return((mPlayerCustomFlags & flags) == flags); }
		uint32				getPlayerCustomFlags() const { return mPlayerCustomFlags; }
		void				setPlayerCustomFlags(uint32 flags){ mPlayerCustomFlags = flags; }

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

		const glm::vec3&	getBindCoords(){ return mBindCoords; }
		void				setBindCoords(const glm::vec3& coords){ mBindCoords = coords; }

		const glm::vec3&	getHomeCoords(){ return mHomeCoords; }
		void				setHomeCoords(const glm::vec3& coords){ mHomeCoords = coords; }

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
		// Healing
		Object*				getHealingTarget(PlayerObject* Player) const;
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
		void				setImageDesignerTaskId(uint64 taskId){mImageDesignerId = taskId;}
		bool				UpdateIdAttributes(BString attribute,float value);
		bool				UpdateIdColors(BString attribute,uint16 value);
		AttributesList*		getIdAttributesList(){return &mIDAttributesList;}
		ColorList*			getIdColorList(){return &mIDColorList;}
		void				SetImageDesignSession(IDSession theSession){mIDSession = theSession;}
		IDSession 			getImageDesignSession(){return mIDSession;}
		uint32				getHoloEmote(){ return mHoloEmote; }
		void				setHoloEmote(uint32 emote){ mHoloEmote = emote; }
		
		uint8				getHoloCharge(){ return mHoloCharge; }
		void				setHoloCharge(uint8 charge){ mHoloCharge = charge; }
		bool				decHoloCharge(){ if(mHoloCharge == 0) return false; mHoloCharge--; return true; }

		uint64				getIDPartner(){ return mIDPartner; }
		void				setIDPartner(uint64 id){ mIDPartner= id; }

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

		//mounts
		bool				checkIfMounted() { return mMounted; }
		void				setMounted(bool mounted) { mMounted = mounted; }

		MountObject*		getMount() { return mMount; }
		void				setMount(MountObject* mount) { mMount = mount; }

		bool				checkIfMountCalled() { return mMountCalled; }
		void				setMountCalled(bool mount_called) { mMountCalled = mount_called; }

		// crafting
		CraftingStation*	getCraftingStation(ObjectSet*	inRangeObjects, ItemType	toolType);
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
		void				addToDuelList(PlayerObject* player);
		void				removeFromDuelList(PlayerObject* player);
		//PlayerList::iterator removeFromDuelList(PlayerList::iterator& it);
		void				clearDuelList(){ mDuelList.clear(); }

		// reset properties on zoning
		void				resetProperties();

		// event functions
		void				onSurvey(const SurveyEvent* event);
		void				onSample(const SampleEvent* event);
		void				onLogout(const LogOutEvent* event);
		void				onBurstRun(const BurstRunEvent* event);
		void				onItemDeleteEvent(const ItemDeleteEvent* event);
		void				onInjuryTreatment(const InjuryTreatmentEvent* event);
		void				onWoundTreatment(const WoundTreatmentEvent* event);
		void				onQuickHealInjuryTreatment(const QuickHealInjuryTreatmentEvent* event);
								 

		// cloning
		void				clone(uint64 parentId, const glm::quat& dir, const glm::vec3& pos, bool preDesignatedFacility=false);
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

		bool				HasCamp(){ return mHasCamp;}
		void				setHasCamp(bool val){mHasCamp = val;}

		//Foraging
		bool				isForaging(){ return mIsForaging; }
		void				setForaging(bool status) { mIsForaging = status; }

		//Harvesting
		int					getPreviousHarvestSelection() { return mPreviousHarvestingSelection;}
		void				setPreviousHarvestSelection(int newSelection){ mPreviousHarvestingSelection = newSelection;}

		//Posture
		void				setUpright();
		void				setSitting(Message* message=NULL);
		void				setProne();
		void				setCrouched();

		bool				getAcceptBandFlourishes() {return mAcceptsBandFlourishes;}
		void				setAcceptBandFlourishes(bool b) { mAcceptsBandFlourishes = b;}

	private:

		void				_verifyBadges();
		void				_verifyExplorationBadges();

		bool				mHasCamp;

		Datapad*			mDataPad;
		bool				mAcceptsBandFlourishes;
		AudienceList		mAudienceList;
		BadgesList			mBadgeList;
		DenyServiceList		mDenyAudienceList;
		PlayerList			mDuelList;
		SchematicsIdList	mFilteredSchematicIdList;
		AttributesList		mIDAttributesList;
		ColorList			mIDColorList;
		SchematicsIdList	mSchematicAddList;
		SchematicsIdList	mSchematicIdList;
		UIWindowList		mUIWindowList;
		XPCapList			mXpCapList;
		XPList				mXpList;
		BuffMap 			mEntertainerBuffMap;
		ContactMap			mFriendsList;
		ContactMap 			mIgnoreList;

		string				mBiography;
		string				mMarriage;
		string				mTitle;

		glm::vec3 mBindCoords;
		glm::vec3 mHomeCoords;

		uint32				mPlayerMatch[4];

		BazaarTerminal*     mBazaarPoint;
		DispatchClient*		mClient;
		CraftingSession*	mCraftingSession;
		MountObject*		mMount;
							
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
		uint64				mIDPartner;

		uint64				mCombatTargetId; // The actual target player are hitting, not always the same as the "look-at" target.
		uint64				mEntertainerPauseId;
		uint64				mEntertainerTaskId;
		uint64				mImageDesignerId;
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
		uint32				mPlayerCustomFlags;
		uint32				mLots;
		uint16				mGroupXp;
		uint16				mMissionIdMask;
		int8				mBindPlanet;
		int8				mHomePlanet;
		uint8				mCsrTag;
		uint8				mFlourishCount;
		uint8				mHoloCharge; //thats the amount of charges our hologenerator has
		uint8				mNewPlayerExemptions;
		bool				mAutoAttack;
		bool				mContactListUpdatePending;
		bool				mFemale;
		bool				mMotdReceived;
		bool				mMountCalled;
		bool				mMounted;
		bool				mNewPlayerMessage;
		
		bool				mTrading;
		bool				mIsForaging;

		int					mPreviousHarvestingSelection;
};



//=============================================================================

#endif



