/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef	ANH_ZONESERVER_QUADTREENODE_H
#define	ANH_ZONESERVER_QUADTREENODE_H

#include "QuadTreeNode.h"
#include "Utils/typedefs.h"


//======================================================================================================================

class QuadTree : public QuadTreeNode
{
	public:

		QuadTree(float lowX,float lowZ,float width,float height,uint8 depth);
		virtual ~QuadTree();

	protected:

};

//======================================================================================================================

#endif


