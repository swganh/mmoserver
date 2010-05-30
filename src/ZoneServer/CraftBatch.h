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

	private:

		uint32					mId;
		uint32					mListId;
		uint32					mExpGroup;
		CraftWeights			mWeights;
		CraftAttributes			mAttributes;
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

		//determines whether the attributes value is integer or float ()draft_craft_item_attribute_link attribute_type
		uint8	getType() { return mType; }

	private:

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


	private:

		uint32	mAttributeId;
		string	mAttributeKey;

		uint32	mAffectedAttributeId;
		string	mAffectedAttributeKey;

		AttributePPME_Enum	mManipulation;
		uint8	mType;

};

//=============================================================================

#endif


