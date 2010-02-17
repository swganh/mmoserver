/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CELL_OBJECT_H
#define ANH_ZONESERVER_CELL_OBJECT_H

#include "StaticObject.h"

//=============================================================================

/*
- cells, need a building as parent
*/

class CellObject :	public StaticObject
{
	friend class CellFactory;

	public:

		//TODO : use the objectcontainer methods instead!
		//please note that the objects need to stay in the worlmanager then though
		//alternatively keep this for creatureObjects

		CellObject();
		CellObject(uint64 id,uint64 parentId,const string model = "object/cell/shared_cell.iff");
		~CellObject();

		uint32		getLoadCount(){ return mTotalLoadCount; }
		void		setLoadCount(uint32 count){ mTotalLoadCount = count; }

	private:

		//ObjectList	mChildObjects;
		uint32		mTotalLoadCount;
	
};

//=============================================================================

#endif