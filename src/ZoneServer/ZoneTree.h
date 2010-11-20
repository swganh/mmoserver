/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_ZONETREE_H
#define ANH_ZONESERVER_ZONETREE_H

#include <vector>
#include <spatialindex/SpatialIndex.h>

#include "Utils/typedefs.h"

#include "ObjectController.h"

//======================================================================================================================

class Object;
class QTRegion;

typedef std::vector<int64>		ObjectIdList;
typedef std::list<Object*>	ObjectList;

//======================================================================================================================

class MyVisitor : public SpatialIndex::IVisitor
{
public:

    MyVisitor(ObjectIdList* objList) {
        mvObjList = objList;
    }
    void visitNode(const SpatialIndex::INode& n) {}

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
    std::shared_ptr<QTRegion>	getQTRegion(double x, double z);

private:

    SpatialIndex::IStorageManager*			mStorageManager;
    SpatialIndex::StorageManager::IBuffer*	mStorageBuffer;
    SpatialIndex::ISpatialIndex*			mTree;
    int64						            mIndexIdentifier;
    //Tools::ResourceUsage 		            mResourceUsage;
};

//======================================================================================================================

#endif
