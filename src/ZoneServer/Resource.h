/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_RESOURCE_H
#define ANH_ZONESERVER_RESOURCE_H

#include "Utils/typedefs.h"

class ResourceType;

//=============================================================================

enum ResourceAttribute
{
	ResAttr_ER	= 0,
	ResAttr_CR	= 1,
	ResAttr_CD	= 2,
	ResAttr_DR	= 3,
	ResAttr_FL	= 4,
	ResAttr_HR	= 5,
	ResAttr_MA	= 6,
	ResAttr_PE	= 7,
	ResAttr_OQ	= 8,
	ResAttr_SR	= 9,
	ResAttr_UT	= 10

};

class PlayerObject;

//=============================================================================

class Resource
{
	friend class ResourceManager;

	public:

		Resource();
		~Resource();

		uint64			getId(){ return mId; }
		void			setId(uint64 id){ mId = id; }
		string			getName(){ return mName; }
		void			setName(const string name){ mName = name; }
		uint32			getTypeId(){ return mTypeId; }
		uint16			getAttribute(uint8 nr){ return mAttributes[nr]; }
		void			setAttribute(uint8 nr,uint16 value){ mAttributes[nr] = value; }
		uint8			getCurrent(){ return mCurrent; }
		void			setCurrent(uint8 c){ mCurrent = c; }
		ResourceType*	getType(){ return mType; }
		void			setType(ResourceType* type){ mType = type; }
		void			sendAttributes(PlayerObject* playerObject);

	protected:

		uint64			mId;
		string			mName;
		uint32			mTypeId;
		uint16			mAttributes[11];
		uint8			mCurrent;
		ResourceType*	mType;
};

#endif

