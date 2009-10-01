/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "PlayerObject.h"
//#include "Item.h"
//#include "skill.h"
#include "DatabaseManager/Transaction.h"

#ifndef ANH_ZONESERVER_Trade_H
#define ANH_ZONESERVER_Trade_H


 class Skill;
 class Item;
//=============================================================================
typedef std::map<uint32,Skill*>					mySkillList;

class SkillTeachContainer
{
public:

	SkillTeachContainer(){}
	~SkillTeachContainer(){mTradeSkills.clear();}
	void				addSkill(uint32 nr, uint32 id){mTradeSkills.insert(std::make_pair(nr,gSkillManager->getSkillById(id)));}
	//	mPlayerAccountMap.insert(accountId,player);
	mySkillList*		getList(){return &mTradeSkills;}
	Skill*				getEntry(uint32 nr){mySkillList::iterator it = mTradeSkills.find(nr);
											if(it != mTradeSkills.end()){
												return (*it).second;
											}else
												return NULL;
											}
	
private:

	mySkillList			mTradeSkills;
	
};


class TradeContainer
{
public:

	TradeContainer(){}
	~TradeContainer(){}
	void				setOldOwner(PlayerObject* oO){mOldOwner = oO;}
	PlayerObject*		getOldOwner(){return mOldOwner;}
	void				setNewOwner(PlayerObject* nO){mNewOwner = nO;}
	PlayerObject*		getNewOwner(){return mNewOwner;}
	void				setObject(TangibleObject* object){mObject = object;}
	TangibleObject*		getObject(){return mObject;}

private:

	PlayerObject*		mOldOwner;		
	PlayerObject*		mNewOwner;		
	TangibleObject*		mObject;
};

typedef std::vector<PlayerObject*> PlayerObjectList;
typedef std::vector<TradeContainer*> ItemTradeList;

class Trade
{

	public:

		Trade(PlayerObject* playerobject);
		~Trade();

		void				cancelTradeSession();
		void				endTradeSession();
		void				updateCash(uint32 amount);
		void				updateBank(uint32 amount);
		bool				getTradeFinishedStatus(){return mTradingFin;}
		void				setTradeFinishedStatus(bool tradeStatus){mTradingFin = tradeStatus;}
		bool				getAcceptStatus(){return mAcceptTrade;}
		void				setAcceptStatus(bool acceptStatus){mAcceptTrade = acceptStatus;}
		void				setTradeCredits(uint32 credits){mMoney = credits;}//sets the amount of credits we want to give
		uint32				getTradeCredits(){return mMoney;}
		bool				ItemTradeCheck(uint64 ItemId);
		bool				checkEquipped(Item* addedItem);
		void				ItemTradeAdd(TangibleObject* addedItem,PlayerObject* newOwner,PlayerObject* oldOwner);
		void				TradeListDelete();
		bool				checkTradeListtoInventory();
		void				processTradeListPostTransaction();
		void				processTradeListPreTransaction(Transaction* mTransaction);
		void				deleteTradeInvitation(PlayerObject* player);
		void				tradeInvitationAdded(PlayerObject* player);
		bool				verifyInvitation(PlayerObject* player);
		bool				_handleCancelTradeInvitation(uint64 callTime, void* nix);
		PlayerObject*		getPlayerObject(){return mPlayerObject;}
		PlayerObject*		getTeacher(){return mteacher;}
		void				setTeacher(PlayerObject* teacher){mteacher = teacher;}
		
	private:

		PlayerObjectList	mPlayerObjectList;
		bool				mTradingFin;
		bool				mAcceptTrade;
		PlayerObject*		mPlayerObject;
		PlayerObject*		mteacher;
		ItemTradeList		mItemTradeList;
		TradeContainer*		mTradeList;
		uint32				mMoney;
};

//=============================================================================

#endif

