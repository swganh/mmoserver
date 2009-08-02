/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_QTREGION_H
#define ANH_ZONESERVER_QTREGION_H

#include "RegionObject.h"
#include "QuadTree.h"


//=============================================================================

class QTRegion : public RegionObject
{
	friend class QTRegionFactory;

	public:

		QTRegion();
		virtual ~QTRegion();

		void		initTree();

		QuadTree*	mTree;

	private:

		uint8		mQTDepth;
		
};

//=============================================================================

#endif


