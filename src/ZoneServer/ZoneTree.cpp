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

#include "ZoneTree.h"
#include "BuildingObject.h"

#include "ObjectContainer.h"
#include "CellObject.h"
#include "WorldManager.h"


using namespace SpatialIndex;

//=============================================================================

ZoneTree::ZoneTree(void) :
mStorageManager(NULL),
mStorageBuffer(NULL),
mTree(NULL),
mIndexIdentifier(0)
{
	// We do have a global clock object, don't use seperate clock and times for every process.
	// mClock = new Anh_Utils::Clock();
}

//=============================================================================

ZoneTree::~ZoneTree(void)
{
	// delete(mClock);
}

//=============================================================================

void ZoneTree::Init(double fillFactor,uint32 indexCap,uint32 leafCap,uint32 dimensions,double horizon)
{
	gLogger->log(LogManager::NOTICE,"SpatialIndex initializing...");		
	gLogger->logCont(LogManager::INFORMATION, "FillFactor:%.2f,",fillFactor);
	gLogger->logCont(LogManager::INFORMATION, "IndexCap:%u,",indexCap);
	gLogger->logCont(LogManager::INFORMATION, "LeafCap:%u,",leafCap);
	gLogger->logCont(LogManager::INFORMATION, "Dimensions:%u,",dimensions);
	gLogger->logCont(LogManager::INFORMATION, "Horizon:%.2f",horizon);
	
	try
	{
		mStorageManager = StorageManager::createNewMemoryStorageManager();
		mStorageBuffer = StorageManager::createNewRandomEvictionsBuffer(*mStorageManager,200,false);

		mTree = RTree::createNewRTree(*mStorageBuffer,fillFactor,indexCap,leafCap,dimensions,SpatialIndex::RTree::RV_RSTAR,mIndexIdentifier);

		mResourceUsage.start();
	}
	catch(Tools::Exception& e)
	{
		gLogger->log(LogManager::EMERGENCY,"*** ERROR: " + e.what() + " ***\n");
	}
	catch(...)
	{
		gLogger->log(LogManager::EMERGENCY,"*** ERROR: Unknown Exception ***\n");
	}
}

//=============================================================================

void ZoneTree::InsertPoint(int64 objId,double x,double z)
{
	double coords[2];
	coords[0] = x;
	coords[1] = z;

	Point p = Point(coords,2);

	mTree->insertData(0,0,p,objId);

	/*std::ostringstream ss;
	ss << "SI(InsertPoint): " << objId << " at " << p;
	gLogger->log(LogManager::DEBUG,ss.str(),MSG_LOW);*/
}

//=============================================================================

void ZoneTree::InsertRegion(int64 objId,double x,double z,double width,double height)
{
	double low[2];
	double high[2];
	low[0] = x - width;
	low[1] = z - height;
	high[0] = x + width;
	high[1] = z + height;

	Region r = Region(low,high,2);

	mTree->insertData(0,0,r,objId);

	/*std::ostringstream ss;
	ss << "SI(InsertRegion): " << objId << " at " << r;
	gLogger->log(LogManager::DEBUG,ss.str(),MSG_LOW);*/
}

//=============================================================================

void ZoneTree::insertQTRegion(int64 objId,double x,double z,double width,double height)
{
	double low[2];
	double high[2];
	low[0] = x;
	low[1] = z;
	high[0] = x + width;
	high[1] = z + height;

	Region r = Region(low,high,2);

	mTree->insertData(0,0,r,objId);
}

//=============================================================================
//
// pointlocation query, primary use for inserts on qt's
// returns the matching qtregion
// remember qtregions shouldnt overlap, so we only get 1 !
// also query based on world coordinates only
//

QTRegion* ZoneTree::getQTRegion(double x,double z)
{
	ObjectIdList	resultIdList;
	MyVisitor		vis(&resultIdList);
	double			dP[2];

	dP[0] = x;
	dP[1] = z;

	Region r = Region(dP,dP,2);
	Tools::Geometry::Point p(dP,2);
	
	//mTree->containsWhatQuery(r,vis);
	//mTree->intersectsWithQuery(r,vis);
	mTree->pointLocationQuery(p,vis);

	// find the region
	ObjectIdList::iterator it = resultIdList.begin();
	while(it != resultIdList.end())
	{
		if(QTRegion* region = gWorldManager->getQTRegion(static_cast<uint32>(*it)))
		{
			return(region);
		}

		++it;
	}

	// We need a region for the Tutorial...
	// gLogger->log(LogManager::DEBUG,"SI could not find qtregion at %f %f",x,z);

	return(NULL);
}


void ZoneTree::getObjectsInRangeIntersection(Object* object,ObjectSet* resultSet,uint32 objTypes,float range)
{
	ObjectIdList	resultIdList;
	Object*			tmpObject;
	ObjectType		tmpType;
	uint64			objectId = object->getId();

	resultIdList.reserve(100);

	double plow[2],phigh[2];

	// we in world space, outside -> inside , outside -> outside checking
	if(!object->getParentId())
	{
		plow[0] = object->mPosition.x - range;
		plow[1] = object->mPosition.z - range;
		phigh[0] = object->mPosition.x + range;
		phigh[1] = object->mPosition.z + range;

		Region r = Region(plow,phigh,2);
		MyVisitor vis(&resultIdList);

		mTree->intersectsWithQuery(r,vis);

		// filter needed objects
		ObjectIdList::iterator it = resultIdList.begin();
		while(it != resultIdList.end())
		{
			// check if its us and the object still exists
			if((static_cast<uint64>(*it) != objectId)
					&& ((tmpObject = gWorldManager->getObjectById((*it))) != NULL))
			{
				// if we are in same parent
				if(!tmpObject->getParentId())
				{
					tmpType = tmpObject->getType();
					// add it
					if((tmpType & objTypes) == static_cast<uint32>(tmpType))
					{
						resultSet->insert(tmpObject);
					}
					// if its a building, add objects of our types it contains
					if(tmpType == ObjType_Building)
					{
						// gLogger->log(LogManager::DEBUG,"Found a building");

						ObjectList cellChilds = (dynamic_cast<BuildingObject*>(tmpObject))->getAllCellChilds();
						ObjectList::iterator cellChildsIt = cellChilds.begin();

						while(cellChildsIt != cellChilds.end())
						{
							Object* cellChild = (*cellChildsIt);

							tmpType = cellChild->getType();

							if((tmpType & objTypes) == static_cast<uint32>(tmpType))
							{
								// TODO: We could add a range check to every object...
								resultSet->insert(cellChild);
							}

							++cellChildsIt;
						}
					}
				}
			}

			++it;
		}
	}
	// we inside a building, inside -> outside, inside -> inside checking
	// need to query based on buildings world position
	else if(object->getParentId() != 0)
	{
		CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(object->getParentId()));
		BuildingObject* buildingObject;

		if(cell)
		{
			buildingObject = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));
		}
		else
		{
			gLogger->log(LogManager::WARNING,"SI could not find cell %"PRIu64"",object->getParentId());
			return;
		}

		if(!buildingObject)
		{
			gLogger->log(LogManager::WARNING,"SI could not find building %"PRIu64"",cell->getParentId());
			return;
		}

		float queryWidth,queryHeight;
		float buildingWidth		= buildingObject->getWidth();
		float buildingHeight	= buildingObject->getHeight();

		// adjusting inside -> outside viewing range
		// we always want to see a bit outside

		// Comment by ERU
		// "Max(range,buildingWidth + 32);"
		// or in words: 'The "query range" is at least 32m outside building, or longer if range permits that.'
		// If the above comment was not the intention, please review original code. It was kinda stranage and unclear...
		if (range > (buildingWidth + 32))
		{
			queryWidth = range;
		}
		else
		{
			queryWidth = buildingWidth + 32;
		}

		if (range > (buildingHeight + 32))
		{
			queryHeight = range;
		}
		else
		{
			queryHeight = buildingHeight + 32;
		}

		plow[0] = buildingObject->mPosition.x - queryWidth;
		plow[1] = buildingObject->mPosition.z - queryHeight;
		phigh[0] = buildingObject->mPosition.x + queryWidth;
		phigh[1] = buildingObject->mPosition.z + queryHeight;

		Region r = Region(plow,phigh,2);
		MyVisitor vis(&resultIdList);

		mTree->intersectsWithQuery(r,vis);

		ObjectIdList::iterator it = resultIdList.begin();
		while(it != resultIdList.end())
		{
			// check if its us and the object still exists
			if(static_cast<uint64>(*it) != objectId && ((tmpObject = gWorldManager->getObjectById((*it))) != NULL))
			{
				tmpType = tmpObject->getType();

				// add it
				if((tmpType & objTypes) == static_cast<uint32>(tmpType))
				{
					resultSet->insert(tmpObject);
				}
				// if its a building, add objects of our types it contains
				if(tmpType == ObjType_Building)
				{
					ObjectList cellChilds = (dynamic_cast<BuildingObject*>(tmpObject))->getAllCellChilds();
					ObjectList::iterator cellChildsIt = cellChilds.begin();

					while(cellChildsIt != cellChilds.end())
					{
						Object* cellChild = (*cellChildsIt);

						tmpType = cellChild->getType();

						if(((tmpType & objTypes) == static_cast<uint32>(tmpType)) && cellChild->getId() != object->getId())
						{
							// TODO: We could add a range check to every object...
							resultSet->insert(cellChild);
						}

						++cellChildsIt;
					}
				}
			}
			++it;
		}
	}
}


void ZoneTree::getObjectsInRange(const Object* const object,ObjectSet* resultSet,uint32 objTypes,float range, bool cellContent)
{
	ObjectIdList	resultIdList;
	Object*			tmpObject;
	ObjectType		tmpType;
	uint64			objectId = object->getId();

	resultIdList.reserve(100);

	double plow[2],phigh[2];

	// we in world space, outside -> inside , outside -> outside checking
	if(!object->getParentId())
	{
		plow[0] = object->mPosition.x - range;
		plow[1] = object->mPosition.z - range;
		phigh[0] = object->mPosition.x + range;
		phigh[1] = object->mPosition.z + range;

		Region r = Region(plow,phigh,2);
		MyVisitor vis(&resultIdList);

		//please note that the containsWhatQuery regularly fails to find objects were standing next to - 
		//mTree->containsWhatQuery(r,vis);

		mTree->intersectsWithQuery(r,vis);
		// filter needed objects
		ObjectIdList::iterator it = resultIdList.begin();
		while(it != resultIdList.end())
		{
			// check if its us and the object still exists
			if((static_cast<uint64>(*it) != objectId) && ((tmpObject = gWorldManager->getObjectById((*it))) != NULL))
			{
				// if we are in same parent	   (world)
				if(!tmpObject->getParentId())
				{
					tmpType = tmpObject->getType();
					// add it
					if((tmpType & objTypes) == static_cast<uint32>(tmpType))
					{
						resultSet->insert(tmpObject);
					}
					// if its a building, add objects of our types it contains
				
					//should we query cellchildren here or rather just create them with their cell regardless
					if((tmpType == ObjType_Building)&&cellContent)
					{
						// gLogger->log(LogManager::DEBUG,"Found a building");

						ObjectList cellChilds = (dynamic_cast<BuildingObject*>(tmpObject))->getAllCellChilds();
						ObjectList::iterator cellChildsIt = cellChilds.begin();

						while(cellChildsIt != cellChilds.end())
						{
							Object* cellChild = (*cellChildsIt);

							tmpType = cellChild->getType();

							if((tmpType & objTypes) == static_cast<uint32>(tmpType))
							{
								// TODO: We could add a range check to every object...
								resultSet->insert(cellChild);
							}

							++cellChildsIt;
						}
					}
				}
			}

			++it;
		}
	}
	// we inside a building, inside -> outside, inside -> inside checking
	// need to query based on buildings world position
	else if(object->getParentId() != 0)
	{
		CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(object->getParentId()));
		BuildingObject* buildingObject;

		if(!cell)
		{
			gLogger->log(LogManager::WARNING,"SI could not find cell %"PRIu64"",object->getParentId());
			return;
		}

	
		buildingObject = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));
		if(!buildingObject)
		{
			gLogger->log(LogManager::WARNING,"SI could not find building %"PRIu64"",cell->getParentId());
			return;
		}
		float buildingWidth		= buildingObject->getWidth();
		float buildingHeight	= buildingObject->getHeight();
		

		float queryWidth,queryHeight;
		

		// adjusting inside -> outside viewing range
		// we always want to see a bit outside

		// Comment by ERU
		// "Max(range,buildingWidth + 32);"
		// or in words: 'The "query range" is at least 32m outside building, or longer if range permits that.'
		// If the above comment was not the intention, please review original code. It was kinda stranage and unclear...
		if (range > (buildingWidth + 32))
		{
			queryWidth = range;
		}
		else
		{
			queryWidth = buildingWidth + 32;
		}

		if (range > (buildingHeight + 32))
		{
			queryHeight = range;
		}
		else
		{
			queryHeight = buildingHeight + 32;
		}

		plow[0] = buildingObject->mPosition.x - queryWidth;
		plow[1] = buildingObject->mPosition.z - queryHeight;
		phigh[0] = buildingObject->mPosition.x + queryWidth;
		phigh[1] = buildingObject->mPosition.z + queryHeight;

		Region r = Region(plow,phigh,2);
		MyVisitor vis(&resultIdList);

		mTree->intersectsWithQuery(r,vis);

		//containswhat query regularly misses objects we stand next to - do *not* use it
		//this might have been because the width and height of buildings was set by default to 128 (ie our viewing range)
		//mTree->containsWhatQuery(r,vis);

		ObjectIdList::iterator it = resultIdList.begin();
		while(it != resultIdList.end())
		{
			// check if its us and the object still exists
			if(static_cast<uint64>(*it) != objectId && ((tmpObject = gWorldManager->getObjectById((*it))) != NULL))
			{
				tmpType = tmpObject->getType();

				// add it
				if((tmpType & objTypes) == static_cast<uint32>(tmpType))
				{
					resultSet->insert(tmpObject);
					//gLogger->log(LogManager::DEBUG,"inserted %"PRIu64"",tmpObject->getId());
				}
		
				// if its a building, add objects of queried types it contains				
				if(tmpType == ObjType_Building)
				{

					ObjectList cellChilds = (dynamic_cast<BuildingObject*>(tmpObject))->getAllCellChilds();
					ObjectList::iterator cellChildsIt = cellChilds.begin();

					while(cellChildsIt != cellChilds.end())
					{
						Object* cellChild = (*cellChildsIt);

						tmpType = cellChild->getType();

						if(((tmpType & objTypes) == static_cast<uint32>(tmpType)) && cellChild->getId() != object->getId())
						{
							// TODO: We could add a range check to every object...
							resultSet->insert(cellChild);
							//gLogger->log(LogManager::DEBUG,"inserted cellchild %"PRIu64"",cellChild->getId());
						}
						
						++cellChildsIt;
					}
				}
			}
			++it;
		}

	}
}

//=============================================================================

void ZoneTree::RemovePoint(int64 objId,double x,double z)
{
	double coords[2];
	coords[0] = x;
	coords[1] = z;

	Point p = Point(coords,2);

	if(mTree->deleteData(p,objId) == false)
	{
		std::ostringstream ss;
		ss << "ZoneTree::RemovePoint *** ERROR: Cannot delete id: " << objId << std::endl;
		gLogger->log(LogManager::DEBUG,ss.str());
	}
	else
	{
		/*std::ostringstream ss;
		ss << "SI(RemovePoint): " << objId << " at " << p;
		gLogger->log(LogManager::DEBUG,ss.str(),MSG_LOW);*/
	}
}

//=============================================================================

void ZoneTree::RemoveRegion(int64 objId,double xLow,double zLow,double xHigh,double zHigh)
{
	double low[2];
	double high[2];
	low[0] = xLow;
	low[1] = zLow;
	high[0] = xHigh;
	high[1] = zHigh;

	Region r = Region(low,high,2);

	if(mTree->deleteData(r,objId) == false)
	{
		std::ostringstream ss;
		ss << " ZoneTree::RemoveRegion *** ERROR: Cannot delete id: " << objId << std::endl;
		gLogger->log(LogManager::DEBUG,ss.str());
	}
	else
	{
		/*std::ostringstream ss;
		ss << "SI(RemoveRegion): " << objId << " at " << r;
		gLogger->log(LogManager::DEBUG,ss.str(),MSG_LOW);*/
	}
}

//=============================================================================

void ZoneTree::DumpStats()
{
	std::ostringstream ss;
	ss << "Dumping TreeStats..." << std::endl;
	ss << *mTree;
	ss << "Buffer Hits: " << mStorageBuffer->getHits() << std::endl;
	ss << "IndexIdentifier: " << mIndexIdentifier << std::endl;
	gLogger->log(LogManager::DEBUG,ss.str());
}

//=============================================================================

void ZoneTree::ShutDown()
{
	gLogger->log(LogManager::DEBUG,"SpatialIndex Shutdown\n");

	try
	{
		mResourceUsage.stop();
	}
	catch(Tools::Exception& e)
	{
		gLogger->log(LogManager::WARNING,"*** ERROR: " + e.what() + " ***\n");
	}
	catch(...)
	{
		gLogger->log(LogManager::WARNING,"*** ERROR: Unknown Exception ***\n");
	}

	delete(mTree);
	delete(mStorageBuffer);
	delete(mStorageManager);

	mIndexIdentifier = 0;

	gLogger->log(LogManager::WARNING,"SpatialIndex Shutdown complete\n");
}
//=============================================================================

// NOTE: THIS USEAGE OF intersectsWithQuery(..)

void ZoneTree::getObjectsInRangeEx(Object* object,ObjectSet* resultSet,uint32 objTypes,float range)
{
	ObjectIdList	resultIdList;
	Object*			tmpObject;
	ObjectType		tmpType;
	uint64			objectId = object->getId();

	resultIdList.reserve(100);

	double plow[2],phigh[2];

	// we in world space, outside -> inside , outside -> outside checking
	if(!object->getParentId())
	{
		plow[0] = object->mPosition.x - range;
		plow[1] = object->mPosition.z - range;
		phigh[0] = object->mPosition.x + range;
		phigh[1] = object->mPosition.z + range;

		Region r = Region(plow,phigh,2);
		MyVisitor vis(&resultIdList);

		mTree->intersectsWithQuery(r,vis);
		// mTree->containsWhatQuery(r,vis);

		// filter needed objects
		ObjectIdList::iterator it = resultIdList.begin();
		while(it != resultIdList.end())
		{
			// check if its us and the object still exists
			if((static_cast<uint64>(*it) != objectId) && ((tmpObject = gWorldManager->getObjectById((*it))) != NULL))
			{
				// if we are in same parent
				if(!tmpObject->getParentId())
				{
					tmpType = tmpObject->getType();
					// add it
					if((tmpType & objTypes) == static_cast<uint32>(tmpType))
					{
						resultSet->insert(tmpObject);
					}
					// if its a building, add objects of our types it contains
					if(tmpType == ObjType_Building)
					{
						// gLogger->log(LogManager::DEBUG,"Found a building");

						ObjectList cellChilds = (dynamic_cast<BuildingObject*>(tmpObject))->getAllCellChilds();
						ObjectList::iterator cellChildsIt = cellChilds.begin();

						while(cellChildsIt != cellChilds.end())
						{
							Object* cellChild = (*cellChildsIt);

							tmpType = cellChild->getType();

							if((tmpType & objTypes) == static_cast<uint32>(tmpType))
							{
								// TODO: We could add a range check to every object...
								// gLogger->log(LogManager::DEBUG,"Found object PRId32",cellChild->getId());
								resultSet->insert(cellChild);
							}

							++cellChildsIt;
						}
					}
				}
			}

			++it;
		}
	}
	// we inside a building, inside -> outside, inside -> inside checking
	// need to query based on buildings world position
	else if(object->getParentId() != 0)
	{
		CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(object->getParentId()));
		BuildingObject* buildingObject;

		if(cell)
		{
			buildingObject = dynamic_cast<BuildingObject*>(gWorldManager->getObjectById(cell->getParentId()));
		}
		else
		{
			gLogger->log(LogManager::WARNING,"SI could not find cell %"PRIu64"",object->getParentId());
			return;
		}

		if(!buildingObject)
		{
			gLogger->log(LogManager::WARNING,"SI could not find building %"PRIu64"",cell->getParentId());
			return;
		}

		float queryWidth,queryHeight;
		float buildingWidth		= buildingObject->getWidth();
		float buildingHeight	= buildingObject->getHeight();

		// adjusting inside -> outside viewing range
		// we always want to see a bit outside

		// ERU: No, no, no...NO!
		// The solution is: "Max(range,buildingWidth + 32);"
		// or in words: 'The "query range" is at least 32m outside building, or longer if range permits that.'
		/*
		if(buildingWidth - range <= 0)
			queryWidth = buildingWidth - (buildingWidth - range);
			// ERU: ehh saying queryWidth = range; in a more complicated way :)
		else
			queryWidth = buildingWidth + 32;

		if(buildingHeight - range <= 0)
			queryHeight = buildingHeight - (buildingHeight - range);
		else
			queryHeight = buildingHeight + 32;
		*/

		if (range > (buildingWidth + 32))
		{
			queryWidth = range;
		}
		else
		{
			queryWidth = buildingWidth + 32;
		}

		if (range > (buildingHeight + 32))
		{
			queryHeight = range;
		}
		else
		{
			queryHeight = buildingHeight + 32;
		}

		plow[0] = buildingObject->mPosition.x - queryWidth;
		plow[1] = buildingObject->mPosition.z - queryHeight;
		phigh[0] = buildingObject->mPosition.x + queryWidth;
		phigh[1] = buildingObject->mPosition.z + queryHeight;

		Region r = Region(plow,phigh,2);
		MyVisitor vis(&resultIdList);

		mTree->intersectsWithQuery(r,vis);

		ObjectIdList::iterator it = resultIdList.begin();
		while(it != resultIdList.end())
		{
			// check if its us and the object still exists
			if(static_cast<uint64>(*it) != objectId && ((tmpObject = gWorldManager->getObjectById((*it))) != NULL))
			{
				tmpType = tmpObject->getType();

				// add it
				if((tmpType & objTypes) == static_cast<uint32>(tmpType))
				{
					resultSet->insert(tmpObject);
				}
				// if its a building, add objects of our types it contains
				if(tmpType == ObjType_Building)
				{
					ObjectList cellChilds = (dynamic_cast<BuildingObject*>(tmpObject))->getAllCellChilds();
					ObjectList::iterator cellChildsIt = cellChilds.begin();

					while(cellChildsIt != cellChilds.end())
					{
						Object* cellChild = (*cellChildsIt);

						tmpType = cellChild->getType();

						if(((tmpType & objTypes) == static_cast<uint32>(tmpType)) && cellChild->getId() != object->getId())
						{
							// TODO: We could add a range check to every object...
							resultSet->insert(cellChild);
						}

						++cellChildsIt;
					}
				}
			}
			++it;
		}
	}
}
