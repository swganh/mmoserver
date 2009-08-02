/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_ZONETREE_H
#define ANH_ZONESERVER_ZONETREE_H

#include "spatialindex/include/SpatialIndex.h"
#include <vector>
#include "ObjectFactory.h"

//#include "Utils/typedefs.h"
#include "LogManager/LogManager.h"


//======================================================================================================================

using namespace SpatialIndex;

typedef std::vector<int64>		ObjectIdList;
typedef std::vector<Object*>	ObjectList;

class QTRegion;
//class Clock;

//======================================================================================================================

class MyVisitor : public IVisitor
{
	public:

		MyVisitor(ObjectIdList* objList) { mvObjList = objList; }
		void visitNode(const INode& n){}

		void visitData(const IData& d)
		{
			mvObjList->push_back(d.getIdentifier());
		}

		void visitData(std::vector<const IData*>& v) {}

	private:

		ObjectIdList*	mvObjList;
};

//======================================================================================================================

class ZoneTree
{
	public:

		ZoneTree();
		~ZoneTree();

		void			Init(double fillFactor,uint32 indexCap,uint32 leafCap,uint32 dimensions,double horizon);
		void			ShutDown();

		void			DumpStats();

		void			insertQTRegion(int64 objId,double x,double z,double width,double height);
		void			InsertPoint(int64 objId,double x,double z);
		void			InsertRegion(int64 objId,double x,double z,double width,double height);
		void			RemovePoint(int64 objId,double x,double z);
		void			RemoveRegion(int64 objId,double xLow,double zLow,double xHigh,double zHigh);

		void			getObjectsInRange(const Object* const object,ObjectSet* resultSet,uint32 objTypes,float range);
		void			getObjectsInRangeIntersection(Object* object,ObjectSet* resultSet,uint32 objTypes,float range);
		void			getObjectsInRangeEx(Object* object,ObjectSet* resultSet,uint32 objTypes,float range);
		QTRegion*		getQTRegion(double x,double z);

	private:

		IStorageManager*			mStorageManager;
		StorageManager::IBuffer*	mStorageBuffer;
		ISpatialIndex*				mTree;
		int64						mIndexIdentifier;
		Tools::ResourceUsage 		mResourceUsage;
		// Anh_Utils::Clock*			mClock;
};

//======================================================================================================================

#endif