/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_BUFF_H
#define ANH_BUFF_H

#include "Utils\typedefs.h"
#include "BuffAttributeEnums.h"
#include "BuffIconsEnum.h"
#include "BuffDBItem.h"
#include "CreatureObject.h"
//#define	 gMessageLib	MessageLib::getSingletonPtr();

class CreatureObject;

class BuffAttribute
{

public:
	BuffAttribute(BuffAttributeEnum Type, int32 InitialValue, int32	TickValue, int32 FinalValue):mAttribute(Type),mInitialValue(InitialValue), mTickValue(TickValue), mFinalValue(FinalValue){};
	
	static BuffAttribute* FromDB(BuffAttributeDBItem* item){ return new BuffAttribute((BuffAttributeEnum)item->mType, item->mInitialValue, item->mTickValue, item->mFinalValue);}
	~BuffAttribute(){;}
	
	BuffAttributeEnum	GetType()			{ return mAttribute; }
	int32				GetInitialValue()	{ return mInitialValue; }
	int32				GetTickValue()		{ return mTickValue; }
	int32				GetFinalValue()		{ return mFinalValue; }

private:
	BuffAttributeEnum	mAttribute;
	int32				mInitialValue;
	int32				mTickValue;
	int32				mFinalValue;
};

typedef std::vector<BuffAttribute*>	AttributeList;

class Buff
{
	friend class CreatureObject;
	friend class BuffManager;

public:
	///////////////////////////////////////////////////////////////////////////////////////////////
	//Public Constructor/Destructor
	///////////////////////////////////////////////////////////////////////////////////////////////
	static Buff*			SimpleBuff(CreatureObject* Target, CreatureObject* Instigator, uint64 Duration, uint32 Icon, uint64 CurrentGlobalTick){ return new Buff(Target, Instigator, 0, Duration, Icon, CurrentGlobalTick); }
	static Buff*			TickingBuff(CreatureObject* Target, CreatureObject* Instigator, uint NoOfTicks, uint64 Tick, uint32 Icon, uint64 CurrentGlobalTick){ return new Buff(Target, Instigator, NoOfTicks, Tick, Icon, CurrentGlobalTick); }
	static Buff*			FromDB(BuffDBItem* Item, uint64 CurrentGlobalTick);
	~Buff();
	///////////////////////////////////////////////////////////////////////////////////////////////
	//Public Methods
	///////////////////////////////////////////////////////////////////////////////////////////////
	uint64				Update(uint64 CurrentTime, void* ref); //Callback for Tick
	void				ReInit();

	void				AddAttribute(BuffAttribute* Attribute){Attributes.push_back(Attribute);}
	void				SetID(uint64 value){ mID=value; }
	void				SetChildBuff(Buff* value){ mChild=value; if(value!=0)value->SetParent(this); }
	void				setTarget(CreatureObject* creature){mTarget=creature;}
	void				EraseAttributes();
	bool				GetIsMarkedForDeletion(){ return mMarkedForDeletion; }
	
	//mID is the process ID for the timer
	uint64				GetID()		{ return mID;}
	//whereas dbId is the Id in the db
	uint64				GetDBID()	{ return mDBID;}

	CreatureObject*		GetTarget()	{ return mTarget; }
	CreatureObject*		GetInstigator(){ return mInstigator; }
	uint64				GetTickLength() { return mTick; }
	uint32				GetNoOfTicks(){ return mNoTicks;}
	uint32				GetCurrentTickNumber(){ return mCurrentTick; }
	uint32				GetIcon(){ return mIcon; }
	string				GetName(){ return mName; }
	uint64				GetStartGlobalTick(){ return mStartTime; }
	void				SetInit(bool init){mDoInit = init;}

private:
	///////////////////////////////////////////////////////////////////////////////////////////////
	//Private Member Variables
	///////////////////////////////////////////////////////////////////////////////////////////////	
	uint64					mID;
	uint64					mDBID;
	string					mName;
	uint32					mIcon;
	CreatureObject*			mTarget;
	CreatureObject*			mInstigator;
	Buff*					mChild;
	Buff*					mParent;
	bool					mCancelled;
	bool					mMarkedForDeletion;
	uint32					mNoTicks; //Total number of Ticks
	uint32					mCurrentTick; //Current Tick Number
	int64					mTick; //Length of Tick in ms
	AttributeList			Attributes;
	uint64					mStartTime; //GlobalTickCount at Start of Buff
	uint64					mNextTickLength; //Store the length of the next tick (for when reloading from DB)
	bool					mDoInit; //Set whether we should do the Init straight away
	///////////////////////////////////////////////////////////////////////////////////////////////
	//Private Methods
	///////////////////////////////////////////////////////////////////////////////////////////////
	void					ModifyAttribute(BuffAttributeEnum Type, int32 Value);
	void					IncrementTick(){ mCurrentTick++;}
	uint64					GetRemainingTime(){ if(mNoTicks > mCurrentTick)return mTick*(mNoTicks-mCurrentTick); else return 0; }	
	bool					UpdateTick(uint64 CurrentTime);	
	void					FinalChanges();		//last change of a buff before deletion
	void					InitialChanges();	//resend icons and attribute changes
	void					InitializeIcons();	//just resend the icons when we rezone before char delete

protected:
	///////////////////////////////////////////////////////////////////////////////////////////////
	//Protected Methods
	///////////////////////////////////////////////////////////////////////////////////////////////
	Buff(CreatureObject* Target, CreatureObject* Instigator, uint NoOfTicks, uint64 Tick, uint32 Icon, uint64 CurrentGlobalTick);
	void SetParent(Buff* value){this->mParent=value;}
};

#endif ANH_BUFF_H