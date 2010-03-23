/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_ZONETREE_H
#define ANH_ZONESERVER_ZONETREE_H

#include "Utils/typedefs.h"
#include <vector>
#include <SpatialIndex.h>
#include "ObjectController.h"

//======================================================================================================================

class Object;
class QTRegion;

typedef std::vector<int64>		ObjectIdList;
typedef std::vector<Object*>	ObjectList;

//======================================================================================================================

class MyVisitor : public SpatialIndex::IVisitor
{
	public:

		MyVisitor(ObjectIdList* objList) { mvObjList = objList; }
        void visitNode(const SpatialIndex::INode& n){}

        void visitData(const SpatialIndex::IData& d)
		{
			mvObjList->push_back(d.getIdentifier());
		}

        void visitData(std::vector<const SpatialIndex::IData*>& v) {}

	private:

		ObjectIdList*	mvObjList;
};

//======================================================================================================================

class ZoneTree
{
	public:

		ZoneTree();
		~ZoneTree();

		void			Init(double fillFactor, uint32 indexCap, uint32 leafCap, uint32 dimensions, double horizon);
		void			ShutDown();

		void			DumpStats();

		void			insertQTRegion(int64 objId, double x, double z, double width, double height);
		void			InsertPoint(int64 objId, double x, double z);
		void			InsertRegion(int64 objId, double x, double z, double width, double height);
		void			RemovePoint(int64 objId, double x, double z);
		void			RemoveRegion(int64 objId, double xLow, double zLow, double xHigh, double zHigh);

		void			getObjectsInRange(const Object* const object, ObjectSet* resultSet, uint32 objTypes, float range, bool cellContent = false);
		void			getObjectsInRangeIntersection(Object* object, ObjectSet* resultSet, uint32 objTypes, float range);
		void			getObjectsInRangeEx(Object* object, ObjectSet* resultSet, uint32 objTypes, float range);
		QTRegion*		getQTRegion(double x, double z);

	private:

        SpatialIndex::IStorageManager*			mStorageManager;
        SpatialIndex::StorageManager::IBuffer*	mStorageBuffer;
        SpatialIndex::ISpatialIndex*			mTree;
		int64						            mIndexIdentifier;
		Tools::ResourceUsage 		            mResourceUsage;
};

//======================================================================================================================

#endif