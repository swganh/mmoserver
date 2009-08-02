/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CRAFTBATCH_H
#define ANH_ZONESERVER_CRAFTBATCH_H

#include "Utils/typedefs.h"

class CraftWeight;
class CraftAttribute;

typedef std::vector<CraftWeight*>		CraftWeights;
typedef std::vector<CraftAttribute*>	CraftAttributes;

//=============================================================================

class CraftBatch
{
	friend class SchematicManager;

	public:

		CraftBatch();
		~CraftBatch();

		uint32				getId(){ return mId; }
		uint32				getListId(){ return mListId; }
		uint32				getExpGroup(){ return mExpGroup; }
		CraftWeights*		getCraftWeights(){ return &mWeights; }
		CraftAttributes*	getCraftAttributes(){ return &mAttributes; }

	private:

		uint32			mId;
		uint32			mListId;
		uint32			mExpGroup;
		CraftWeights	mWeights;
		CraftAttributes	mAttributes;
};

//=============================================================================

class CraftWeight
{
	friend class SchematicManager;

	public:

		CraftWeight(){}
		~CraftWeight(){}

		//returns the resource attribute like oq/pe etc
		uint8	getDataType(){ return mDataType; }
		float	getDistribution(){ return mDistribution; }

	private:

		uint8	mDataType;
		float	mDistribution;
};

//=============================================================================

class CraftAttribute
{
	friend class SchematicManager;

	public:

		CraftAttribute(){}
		~CraftAttribute(){}

		uint32	getAttributeId(){ return mAttributeId; }
		string	getAttributeKey(){ return mAttributeKey; }
		float	getMin(){ return mMin; }
		float	getMax(){ return mMax; }
		uint8	getType() { return mType; }

	private:

		uint32	mAttributeId;
		string	mAttributeKey;
		uint8	mType;
		float	mMin;
		float	mMax;
};

//=============================================================================

#endif


