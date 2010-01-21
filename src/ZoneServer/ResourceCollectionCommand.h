/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_RESOURCECOLLECTIONCOMMAND_H
#define ANH_ZONESERVER_RESOURCECOLLECTIONCOMMAND_H

#include "Utils/typedefs.h"

//=============================================================================

class ResourceCollectionCommand
{
	friend class ResourceCollectionManager;

	public:

		ResourceCollectionCommand();
		~ResourceCollectionCommand();

		// getters
		uint32	getId()							{ return mId; }
		string	getCommandName()				{ return mCommandName; }
		int32	getHealthCost()					{ return mHealthCost; }
		int32	getActionCost()					{ return mActionCost; }
		int32	getMindCost()					{ return mMindCost; }
		int32	getDamageModifier()				{ return mDamageModifier; }

		// setters
		void	setId(uint32 id)				{ mId = id; }
		void	setCommandName(string cName)	{ mCommandName = cName; }
		void	setHealthCost(int32 hCost)		{ mHealthCost = hCost; }
		void	setActionCost(int32 aCost)		{ mActionCost = aCost; }
		void	setMindCost(int32 mCost)		{ mMindCost = mCost; }
		void	setDamageModifier(int32 damage) { mDamageModifier = damage; }


	private:

		uint32	mId;
		string	mCommandName;
		int32	mHealthCost;
		int32	mActionCost;
		int32	mMindCost;
		int32	mDamageModifier;
};

//=============================================================================

#endif