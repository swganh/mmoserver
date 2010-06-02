/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CRAFTBATCH_H
#define ANH_ZONESERVER_CRAFTBATCH_H

#include "Utils/typedefs.h"

class CraftWeight;
class CraftAttribute;
class CraftAttributeWeight;

typedef std::vector<CraftWeight*>		CraftWeights;
typedef std::vector<CraftAttribute*>	CraftAttributes;
typedef std::vector<CraftAttributeWeight*>	CraftAttributeWeights;

//=============================================================================

class CraftBatch
{
	friend class SchematicManager;

	public:

		CraftBatch();
		~CraftBatch();

		uint32					getId(){ return mId; }
		uint32					getListId(){ return mListId; }
		uint32					getExpGroup(){ return mExpGroup; }
		CraftWeights*			getCraftWeights(){ return &mWeights; }
		CraftAttributes*		getCraftAttributes(){ return &mAttributes; }
		uint64  getSchemId() {return mSchematicId;}
		uint32  getSchemGroupId(){return mSchemGroupId;}
	private:

		uint32					mId;
		uint32					mListId;
		uint32					mExpGroup;
		CraftWeights			mWeights;
		CraftAttributes			mAttributes;
		uint64  mSchematicId;
		uint32  mSchemGroupId;
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
		uint64  getSchemId() {return mSchematicId;}
		uint32  getSchemGroupId(){return mSchemGroupId;}
	private:

		uint8	mDataType;
		float	mDistribution;
		uint64  mSchematicId;
		uint32  mSchemGroupId;
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
		uint64  getSchemId() {return mSchematicId;}
		uint32  getSchemGroupId(){return mSchemGroupId;}
		//determines whether the attributes value is integer or float ()draft_craft_item_attribute_link attribute_type
		uint8	getType() { return mType; }

	private:
		uint64  mSchematicId;
		uint32  mSchemGroupId;
		uint32	mAttributeId;
		string	mAttributeKey;
		uint8	mType;
		float	mMin;
		float	mMax;
};

enum AttributePPME_Enum
{
	AttributePPME_AddValue = 1,
	AttributePPME_AddAttribute = 2
};

class CraftAttributeWeight
{
	friend class SchematicManager;

	public:

		CraftAttributeWeight(){}
		~CraftAttributeWeight(){}

		uint32	getAttributeId(){ return mAttributeId; }
		string	getAttributeKey(){ return mAttributeKey; }

		uint32	getAffectedAttributeId(){ return mAffectedAttributeId; }
		string	getAffectedAttributeKey(){ return mAffectedAttributeKey; }

		uint32	getManipulation(){ return mManipulation; }

		uint64  getSchemId() {return mSchematicId;}
		uint32  getSchemGroupId(){return mSchemGroupId;}
	private:
		uint64  mSchematicId;
		uint32  mSchemGroupId;
		uint32	mAttributeId;
		string	mAttributeKey;

		uint32	mAffectedAttributeId;
		string	mAffectedAttributeKey;

		AttributePPME_Enum	mManipulation;
		uint8	mType;

};

//=============================================================================

#endif


