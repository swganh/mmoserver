/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_QTREGION_H
#define ANH_ZONESERVER_QTREGION_H

#include "RegionObject.h"

class QuadTree;

//=============================================================================

class QTRegion : public RegionObject
{
	friend class QTRegionFactory;

	public:

		QTRegion();
		virtual ~QTRegion();

		void		initTree();
		bool		checkPlayerPosition(float x, float y);

		QuadTree*	mTree;

	private:

		uint8		mQTDepth;
		
};

//=============================================================================

#endif


