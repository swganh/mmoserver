/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "QTRegion.h"
#include "QuadTree.h"


//=============================================================================
//
// Constructor
//

QTRegion::QTRegion() : 
RegionObject(),
mTree(NULL),
mQTDepth(8)
{
	mRegionType = Region_Zone;
	mActive		= false;
}

//=============================================================================
//
// Deconstructor
//

QTRegion::~QTRegion()
{
	delete(mTree);
}

//=============================================================================
//
// setup the qtree
//

void QTRegion::initTree()
{
	mTree = new QuadTree(mPosition.mX,mPosition.mZ,mWidth,mHeight,mQTDepth);
}

//=============================================================================

