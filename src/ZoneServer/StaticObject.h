/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_STATIC_OBJECT_H
#define ANH_ZONESERVER_STATIC_OBJECT_H

#include "ObjectContainer.h"


//=============================================================================

class StaticObject : public ObjectContainer
{
	friend class ObjectFactory;

	public:

		StaticObject();
		StaticObject(uint64 id,uint64 parentId,const string model,ObjectType type);
		virtual ~StaticObject();

	protected:

};

//=============================================================================

#endif