/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef	ANH_ZONESERVER_QUADTREE_H
#define	ANH_ZONESERVER_QUADTREE_H

#include "Utils/typedefs.h"
#include "Object.h"
#include "Mathlib/Shape.h"
#include "Mathlib/Rectangle.h"
#include <map>

typedef std::map<uint64,Object*>	StdObjectMap;

//======================================================================================================================

class QuadTreeNode : public Anh_Math::Rectangle
{
	public:

		QuadTreeNode(float lowX,float lowZ,float width,float height);
		virtual ~QuadTreeNode();

		int32	addObject(Object* object);
		int32	removeObject(Object* object);
		int32	updateObject(Object* object,Anh_Math::Vector3 newPosition);

		bool	checkBounds(Object* object);
		bool	intersects(Anh_Math::Shape* shape);
		bool	ObjectContained(Anh_Math::Shape* shape, Object* object);
		void	getObjectsInRange(Object* object,ObjectSet* resultSet,uint32 typeMask,Anh_Math::Shape* shape);
		void	getObjectsInRangeContains(Object* object,ObjectSet* resultSet,uint32 typeMask,Anh_Math::Shape* shape);
		
		void	subDivide();

	protected:

		QuadTreeNode**	mSubNodes;
		StdObjectMap	mObjects;
};

//======================================================================================================================

#endif


