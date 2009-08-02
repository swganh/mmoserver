/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_HAMPROPERTY_H
#define ANH_ZONESERVER_HAMPROPERTY_H

#include "Utils/typedefs.h"


//=============================================================================

typedef std::vector<int32*>	ValueTypes;

enum ValueIndex
{
	//Current hitpoints is what you currently have after that MOB bit a bite out of you
	HamProperty_CurrentHitpoints	=	0,
	
	//MaxHitpoints are the Hitpoints you can have maximally with Modifiers applied (NO WOUNDS)
	HamProperty_MaxHitpoints		=	1,
	HamProperty_Modifier			=	2,
	HamProperty_Encumbrance			=	3,
	HamProperty_Wounds				=	4,
	
	//Modified Hitpoints are the virtual maximal hitpoints you have available
	//basically its Maxhitpoints without Wounds
	HamProperty_ModifiedHitpoints	=	5,
	
	//Basehitpoints are the UNMODIFIED base hitpoints you have without Wounds or Buffs or loss of hitpoints
	HamProperty_BaseHitpoints		=	6,
};

//=============================================================================

class HamProperty
{
	friend class PlayerObjectFactory;
	friend class ObjectController;

	public:

		//please note that on load BaseHitpoints and MaxHitpoints are identical
		//only when we apply a buff will MaxHitpoints differ from BaseHitpoints
		HamProperty(int32 currentHitPoints = 500,int32 maxHitPoints = 500,int32 modifier = 0,int32 encumbrance = 0,int32 wounds = 0)
			:mBaseHitPoints(maxHitPoints),mCurrentHitPoints(currentHitPoints),mMaxHitPoints(maxHitPoints),mModifier(modifier),mEncumbrance(encumbrance),mWounds(wounds)
		{
			int32*	v[] = { &mCurrentHitPoints,&mMaxHitPoints,&mModifier,&mEncumbrance,&mWounds,&mModifiedHitPoints,&mBaseHitPoints };
			mValues = ValueTypes(v,v + 7);
		}

		~HamProperty();

		void	Init(int32 currentHitPoints,int32 maxHitPoints,int32 modifier,int32 encumbrance,int32 wounds);

		int32	getCurrentHitPoints(){ return mCurrentHitPoints; }
		void	setCurrentHitPoints(int32 hp){ mCurrentHitPoints = hp; }
		uint8	updateCurrentHitpoints(int32 hp,bool damage = false);

		int32	getMaxHitPoints(){ return mMaxHitPoints; }
		void	setMaxHitPoints(int32 hp){ mMaxHitPoints = hp; }

		int32	getBaseHitPoints(){ return mBaseHitPoints; }
		void	setBaseHitPoints(int32 hp){ mBaseHitPoints = hp; }
		
		int32	getModifier(){ return mModifier; }
		void	setModifier(int32 mod){ mModifier = mod; }
		int32	getEncumbrance(){ return mEncumbrance; }
		void	setEncumbrance(int32 enc){ mEncumbrance = enc; }
		int32	getWounds(){ return mWounds; }
		void	setWounds(int32 wounds){ mWounds = wounds; }
		bool	updateWounds(int32 wounds); 
		bool	updateBaseHitpoints(int32 hp);

		bool	updateModifiedHitpoints(int32 hp);

		int32	getModifiedHitPoints(){ return mModifiedHitPoints; }
		void	calcModifiedHitPoints();

		int32	getValue(uint8 valueIndex);
		void	setValue(uint8 valueIndex, int32 propertyValue);
		void	updateValue(uint8 valueIndex, int32 propertyDelta);

		void	log();
		
	private:
		
		ValueTypes	mValues;
		int32		mCurrentHitPoints; //current available hitpoints
		int32		mModifiedHitPoints;//max hitpoints available after BF and wounds have been catered for
		int32		mMaxHitPoints;     // maximal available hitpoints
		int32		mBaseHitPoints;    // normally available hitpoints
		int32		mModifier;
		int32		mEncumbrance;
		int32		mWounds;
};

#endif
