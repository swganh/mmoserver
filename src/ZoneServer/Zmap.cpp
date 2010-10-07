#include "zmap.h"

#include "Object.h"
#include "regionObject.h"
//#include "PlayerObject.h"




zmap* zmap::ZMAP = NULL;

struct SubCell
{
public:
	uint32			subCellId;

	float			x;
	float			z; 
	float			height; 
	float			width;

	RegionObject*	region;

};

zmap::zmap()
{
	mCurrentSubCellID = 0;

	ZMAP = this;

	uint32 viewRange = VIEWRANGE;

	uint32 x = 0, i = 0, j = 0;

	
	// Setup the lookup array...
	i = 0;
	for(x = 0; x <= GRIDWIDTH; x++)
	{
		for(j = 0; j <= GRIDHEIGHT; j++)
		{
			zmap_lookup[x][j] = i;
			i++;

		}
	}
}

zmap::~zmap()
{

}

bool zmap::_isInSubCellExtent(SubCell* subCell, float x, float z)
{
	if(((x >= subCell->x) && (x <= (subCell->x + subCell->height))) && 
		((z >= subCell->z) && (z <= (subCell->z + subCell->width))))
		return true;
	else
		return false;
}

uint32	zmap::AddSubCell(float low_x, float low_z, float height, float width, RegionObject* region)
{
	SubCell* pSubCell		= new SubCell();
	pSubCell->x				= low_x;
	pSubCell->z				= low_z;
	pSubCell->height		= height;
	pSubCell->width			= width;
	pSubCell->region		= region;
	pSubCell->subCellId		= mCurrentSubCellID++;

	region->subCellId		= pSubCell->subCellId;

	uint32 lowerLeft		= _getCellId(low_x,			low_z);
	uint32 lowerRight		= _getCellId(low_x+width,	low_z);
	uint32 upperLeft		= _getCellId(low_x,			low_z+height);
	uint32 upperRight		= _getCellId(low_x+width,	low_z+height);

	unsigned int cellCountZ = (lowerLeft - upperLeft)/GRIDWIDTH;
	unsigned int cellCountX = (lowerLeft - lowerRight);

	for(unsigned int i=0; i < cellCountZ; i++)
	{
		for(unsigned int j=0; j < cellCountX; j++)
		{
			subCells.insert(std::make_pair((lowerLeft + j + i * GRIDWIDTH), pSubCell));
			ZMapCells[(lowerLeft + j + i * GRIDWIDTH)].SubCells.push_back(pSubCell);
		}
	}

	return pSubCell->subCellId;
}

RegionObject* zmap::getSubCell(uint32 subCellId)
{
	std::multimap<uint32, SubCell*>::iterator it = subCells.begin();
	std::multimap<uint32, SubCell*>::iterator end = subCells.end();

	while(it != end)
	{
		if((*it).second->subCellId == subCellId)
		{
			//get cells
			return (*it).second->region;
		}
		else
		{
			++it;
		}
	}

	return NULL;
}

bool zmap::isObjectInSubCell(Object* object, uint32 subCellId)
{
	std::multimap<uint32, SubCell*>::iterator it;

	std::pair<std::multimap<uint32, SubCell*>::iterator, std::multimap<uint32, SubCell*>::iterator> multi_pairing;

	multi_pairing = subCells.equal_range(object->zmapCellID);

	for(std::multimap<uint32, SubCell*>::iterator it = multi_pairing.first; it != multi_pairing.second; ++it)
	{
		if((*it).second->subCellId == subCellId)
			return _isInSubCellExtent((*it).second, object->mPosition.x, object->mPosition.z);
	}

	return false;
}

void	zmap::RemoveSubCell(uint32 subCellId)
{
	std::multimap<uint32, SubCell*>::iterator it = subCells.begin();
	std::multimap<uint32, SubCell*>::iterator end = subCells.end();



	while(it != end)
	{
		if((*it).second->subCellId == subCellId)
		{
			//get cells
			uint32 lowerLeft		= _getCellId((*it).second->x, (*it).second->z);
			uint32 lowerRight		= _getCellId((*it).second->x + (*it).second->width,	(*it).second->z);
			uint32 upperLeft		= _getCellId((*it).second->x, (*it).second->z + (*it).second->height);
			uint32 upperRight		= _getCellId((*it).second->x + (*it).second->width,	(*it).second->z + (*it).second->height);

			unsigned int cellCountZ = (lowerLeft - upperLeft)/GRIDWIDTH;
			unsigned int cellCountX = (lowerLeft - lowerRight);

			for(unsigned int i=0; i < cellCountZ; i++)
			{
				for(unsigned int j=0; j < cellCountX; j++)
				{					
					SubcellListType				cellList = ZMapCells[(lowerLeft + j + i * GRIDWIDTH)].SubCells;
					
					SubcellListType::iterator	CellListit = cellList.begin();

					while(CellListit != cellList.begin())
					{
						if((*CellListit)->subCellId == subCellId)
						{
							cellList.erase(CellListit);
							break;
						}
						CellListit++;
					}

				}
			}

			it = subCells.erase(it);
		}
		else
		{
			++it;
		}
	}
}

uint32 zmap::_getCellId(float x, float z)
{
	return zmap_lookup[((((uint32)z) + (MAPWIDTH/2))/GRIDWIDTH)][((((uint32)x) + (MAPHEIGHT/2))/GRIDHEIGHT)];
}


void zmap::RemoveObject(Object *removeObject)
{
	uint32 cellId = removeObject->zmapCellID;

	ObjectListType list;
	if(removeObject->getType() == ObjType_Player)
	{
		list = ZMapCells[cellId].Players;
	}
	else
	{
		list = ZMapCells[cellId].Objects;
	}

	for(ObjectListType::iterator i = list.begin(); i != list.end(); i++)
	{
		if((*i)->getId() == removeObject->getId())
		{
			list.erase(i);
			break;
		}
	}

	return;
}



ObjectStruct*	zmap::GetCellContents(uint32 CellID)
{
	//Pesudo
	// 1. Return list of objects in cell
	//if(CellID) >
	
	if(CellID > (GRIDWIDTH*GRIDHEIGHT))
		return &EmptyStruct;

	return &ZMapCells[CellID];
}

ObjectListType*	zmap::GetAllCellContents(uint32 CellID)
{
	//Pesudo
	// 1. Return list of objects in cell
	//if(CellID) >
	
	if(CellID > (GRIDWIDTH*GRIDHEIGHT))
		return &EmptyCell;

	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = ZMapCells[CellID].Objects;
	ReturnList->splice(it, temp);

	temp = ZMapCells[CellID].Players;
	ReturnList->splice(it, temp);

	return ReturnList;
}

ObjectListType*	zmap::GetPlayerCellContents(uint32 CellID)
{
	//Pesudo
	// 1. Return list of objects in cell
	//if(CellID) >
	
	if(CellID > (GRIDWIDTH*GRIDHEIGHT))
		return &EmptyCell;

	return &ZMapCells[CellID].Players;
}

ObjectListType*	zmap::GetObjectCellContents(uint32 CellID)
{
	//Pesudo
	// 1. Return list of objects in cell
	//if(CellID) >
	
	if(GetCellValidFlag(CellID))
		return &ZMapCells[CellID].Objects;
	
	return &EmptyCell;
	
	
}

//=================================================
//returns Players in chatrange

bool zmap::GetCellValidFlag(uint32 CellID)
{
	if(CellID > (GRIDWIDTH*GRIDHEIGHT))
		return false;
	return true;
}

//=================================================
//returns Players in chatrange

ObjectListType* zmap::GetChatRangeCellContents(uint32 CellID)
{
	//Pesudo
	// 1. Combine the lists of Neiboring cells to 1

	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetPlayerCellContents(CellID);
	ReturnList->splice(it, temp);

	temp = *GetPlayerCellContents(CellID + GRIDWIDTH);
	ReturnList->splice(it, temp);

	temp = *GetPlayerCellContents(CellID - GRIDWIDTH);
	ReturnList->splice(it, temp);

	temp = *GetPlayerCellContents(CellID + 1);
	ReturnList->splice(it, temp);

	temp = *GetPlayerCellContents(CellID - 1);
	ReturnList->splice(it, temp);

	temp = *GetPlayerCellContents(CellID + GRIDWIDTH-1);
	ReturnList->splice(it, temp);

	temp = *GetPlayerCellContents(CellID + GRIDWIDTH+1);
	ReturnList->splice(it, temp);

	temp = *GetPlayerCellContents(CellID - GRIDWIDTH+1);
	ReturnList->splice(it, temp);

	temp = *GetPlayerCellContents(CellID - GRIDWIDTH-1);
	ReturnList->splice(it, temp);


	return ReturnList;
}

//=====================================================
//Get Contents for a Row based on a middle cell
//depending on how far we are away of the player we need to resize the row
//this will be done by the var iteration

ObjectListType* zmap::GetAllGridContentsListRow(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectStruct temp = *GetCellContents(CellID);
	ReturnList->splice(it, temp.Objects);
	ReturnList->splice(it, temp.Players);
	

	for(int i = 1; i <= viewRange; i++)
	{
		//if(fmod(CellID+1,))
		temp = *GetCellContents(CellID + i);
		ReturnList->splice(it, temp.Objects);
		ReturnList->splice(it, temp.Players);
		
		temp = *GetCellContents(CellID - i);
		ReturnList->splice(it, temp.Objects);
		ReturnList->splice(it, temp.Players);
	}

	return ReturnList;

}

ObjectListType* zmap::GetPlayerGridContentsListRow(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectStruct temp = *GetCellContents(CellID);
	ReturnList->splice(it, temp.Players);
	

	for(int i = 1; i <= viewRange; i++)
	{
		//if(fmod(CellID+1,))
		temp = *GetCellContents(CellID + i);
		ReturnList->splice(it, temp.Players);
		
		temp = *GetCellContents(CellID - i);
		ReturnList->splice(it, temp.Players);
	}

	return ReturnList;

}

ObjectListType* zmap::GetObjectGridContentsListRow(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectStruct temp = *GetCellContents(CellID);
	ReturnList->splice(it, temp.Objects);
	

	for(int i = 1; i <= viewRange; i++)
	{
		//if(fmod(CellID+1,))
		temp = *GetCellContents(CellID + i);
		ReturnList->splice(it, temp.Objects);
		
		temp = *GetCellContents(CellID - i);
		ReturnList->splice(it, temp.Objects);
	}

	return ReturnList;

}

//==========================================================================
//column downwards
//when we move along the edges
///15.6 sch
//
ObjectListType* zmap::GetPlayerGridContentsListColumnDown(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetPlayerCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i <= (viewRange*2)-1; i++)
	{		
		temp = *GetPlayerCellContents(CellID - (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;
}

ObjectListType* zmap::GetObjectGridContentsListColumnDown(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetObjectCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i <= (viewRange*2)-1; i++)
	{		
		temp = *GetObjectCellContents(CellID - (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;
}

ObjectListType* zmap::GetAllGridContentsListColumnDown(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();
								 
	ObjectListType temp = *GetAllCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i <= (viewRange*2)-1; i++)
	{		
		temp = *GetAllCellContents(CellID - (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;
}

ObjectListType* zmap::GetAllGridContentsListColumnUp(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetAllCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i <= (viewRange*2)-1; i++)
	{		
		temp = *GetAllCellContents(CellID + (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;
}

ObjectListType* zmap::GetPlayerGridContentsListColumnUp(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetPlayerCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i <= (viewRange*2)-1; i++)
	{		
		temp = *GetPlayerCellContents(CellID + (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;
}

ObjectListType* zmap::GetObjectGridContentsListColumnUp(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetObjectCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i <= (viewRange*2)-1; i++)
	{		
		temp = *GetObjectCellContents(CellID + (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;
}

//===============================================================0
// when getting content on the edges just spare the *middle* (pointy) cell
ObjectListType* zmap::GetAllGridContentsListRowLeft(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetAllCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i < ((viewRange*2)-1); i++)
	{
		temp = *GetAllCellContents(CellID - i);
		ReturnList->splice(it, temp);
	}

	return ReturnList;

}

ObjectListType* zmap::GetPlayerGridContentsListRowLeft(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetPlayerCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i < ((viewRange*2)-1); i++)
	{
		temp = *GetPlayerCellContents(CellID - i);
		ReturnList->splice(it, temp);
	}

	return ReturnList;

}

ObjectListType* zmap::GetObjectGridContentsListRowLeft(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetObjectCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i < ((viewRange*2)-1); i++)
	{
		temp = *GetObjectCellContents(CellID - i);
		ReturnList->splice(it, temp);
	}

	return ReturnList;

}

ObjectListType* zmap::GetAllGridContentsListRowRight(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetAllCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i < ((viewRange*2)-1); i++)
	{
		temp = *GetAllCellContents(CellID + i);
		ReturnList->splice(it, temp);
	}

	return ReturnList;

}

ObjectListType* zmap::GetPlayerGridContentsListRowRight(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetPlayerCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i < ((viewRange*2)-1); i++)
	{
		temp = *GetPlayerCellContents(CellID + i);
		ReturnList->splice(it, temp);
	}

	return ReturnList;

}

ObjectListType* zmap::GetObjectGridContentsListRowRight(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetObjectCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i < ((viewRange*2)-1); i++)
	{
		temp = *GetObjectCellContents(CellID + i);
		ReturnList->splice(it, temp);
	}

	return ReturnList;
}

ObjectListType* zmap::GetAllGridContentsListColumn(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetAllCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i <= viewRange; i++)
	{
		//if(fmod(CellID+1,))
		temp = *GetAllCellContents(CellID + (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
		
		temp = *GetAllCellContents(CellID - (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;

}

ObjectListType* zmap::GetPlayerGridContentsListColumn(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetPlayerCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i <= viewRange; i++)
	{
		//if(fmod(CellID+1,))
		temp = *GetPlayerCellContents(CellID + (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
		
		temp = *GetPlayerCellContents(CellID - (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;

}


ObjectListType* zmap::GetObjectGridContentsListColumn(uint32 CellID)
{
	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetObjectCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 1; i <= viewRange; i++)
	{
		//if(fmod(CellID+1,))
		temp = *GetObjectCellContents(CellID + (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
		
		temp = *GetObjectCellContents(CellID - (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;

}


ObjectListType* zmap::GetAllViewingRangeCellContents(uint32 CellID)
{
	//Pesudo
	// 1. Combine the lists of Neiboring cells to 1

	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetAllCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 0; i < viewRange; i++)
	{
		temp = *GetAllGridContentsListRow(CellID + (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
		
		temp = *GetAllGridContentsListRow(CellID - (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;
}

ObjectListType* zmap::GetPlayerViewingRangeCellContents(uint32 CellID)
{
	//Pesudo
	// 1. Combine the lists of Neiboring cells to 1

	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType	temp = *GetPlayerCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 0; i < viewRange; i++)
	{
		temp = *GetPlayerGridContentsListRow(CellID + (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
		
		temp = *GetPlayerGridContentsListRow(CellID - (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;
}

ObjectListType* zmap::GetObjectViewingRangeCellContents(uint32 CellID)
{
	//Pesudo
	// 1. Combine the lists of Neiboring cells to 1

	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetObjectCellContents(CellID);
	ReturnList->splice(it, temp);

	for(int i = 0; i < viewRange; i++)
	{
		temp = *GetObjectGridContentsListRow(CellID + (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
		
		temp = *GetObjectGridContentsListRow(CellID - (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;
}

// limited to max viewing range for now
//
ObjectListType* zmap::GetObjectCustomRangeCellContents(uint32 CellID, uint32 range)
{
	// query the grid with the custom range
	// TODO: need any failsafes concerning the cells???

	if(range > VIEWRANGE)
		range = VIEWRANGE;


	ObjectListType* ReturnList = new ObjectListType;
	ObjectListType::iterator it = ReturnList->begin();

	ObjectListType temp = *GetObjectCellContents(CellID);
	ReturnList->splice(it, temp);

	for(uint32 i = 0; i < range; i++)
	{
		temp = *GetObjectGridContentsListRow(CellID + (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
		
		temp = *GetObjectGridContentsListRow(CellID - (i*GRIDWIDTH));
		ReturnList->splice(it, temp);
	}

	return ReturnList;
}





uint32 zmap::AddObject(Object *newObject)
{
	
	uint32 finalCell = _getCellId(newObject->mPosition.x, newObject->mPosition.y);

	if(!GetCellValidFlag(finalCell))
	{
		//something fishy here
		assert(false && "SpatialIndexManager::AddObject :: couldnt find grid cell :(");
		return 0xffffffff;
	}

	newObject->zmapCellID = finalCell;
	
	ObjectListType list;

	if(newObject->getType() == ObjType_Player)
	{
		list = ZMapCells[finalCell].Players;
	}
	else
	{
		list = ZMapCells[finalCell].Objects;
	}

	for(ObjectListType::iterator i = list.begin(); i != list.end(); i++)
	{
		if((*i)->getId() == newObject->getId())
		{
			return 0xffffffff;
		}
	}
	
	list.push_back(newObject);

	//have we just entered a region ???
	SubcellListType				cellList = ZMapCells[finalCell].SubCells;
					
	SubcellListType::iterator	CellListit = cellList.begin();

	while(CellListit != cellList.begin())
	{
		if(isObjectInSubCell(newObject,(*CellListit)->subCellId))
		{
			(*CellListit)->region->onObjectEnter(newObject);
		}
		CellListit++;
	}


	return finalCell;
}

void zmap::UpdateObject(Object *updateObject)
{
	// Pesudo
	// 1. Calculate Cell
	// 2. If cell is different then current cell
	//		2a. Remove him from the current cell, insert into new cell at new cell
	//		2b. Update info....

	glm::vec3   position;
	
	//cater for players in cells
	if (updateObject->getParentId())
	{
		position = updateObject->getWorldPosition(); 
	}
	else
	{
		position = updateObject->mPosition;
	}
	
	uint32 finalCell	= getCellId(position.x, position.y);
	uint32 oldCell		= updateObject->zmapCellID;

	updateObject->zmapCellID = finalCell;

	std::multimap<uint32, SubCell*>::iterator it;

	//remove any old subcells that are not in the new cell

	//iterate through the subcells the object is in
	std::set<uint32>::iterator subCellIt = updateObject->zmapSubCells.begin();
	while(subCellIt != updateObject->zmapSubCells.end())
	{
		//are we still in the subcell?
		if(!isObjectInSubCell(updateObject, (*subCellIt)))
		{
			//nope we left
			it = subCells.find((*subCellIt));
			if (it != subCells.end())
			{
				(*it).second->region->onObjectLeave(updateObject);
			}

			updateObject->zmapSubCells.erase(subCellIt);
			
		}
		subCellIt++;
	}

	//We need to check subregions -> New cell
	
	std::pair<std::multimap<uint32, SubCell*>::iterator, std::multimap<uint32, SubCell*>::iterator> multi_pairing;

	multi_pairing = subCells.equal_range(updateObject->zmapCellID);

	for(std::multimap<uint32, SubCell*>::iterator it = multi_pairing.first; it != multi_pairing.second; ++it)
	{
		bool isInRegion = false;
		bool isTrulyInRegion = isObjectInSubCell(updateObject, (*it).second->subCellId);

		std::set<uint32>::iterator subCellIt = updateObject->zmapSubCells.find((*it).second->subCellId);

		if(subCellIt != updateObject->zmapSubCells.end())
			isInRegion = true;

		if(isTrulyInRegion && !isInRegion)
		{
			//Has just entered the region
			updateObject->zmapSubCells.insert((*it).second->subCellId);
			(*it).second->region->onObjectEnter(updateObject);
			
		}
		else if(!isTrulyInRegion && isInRegion)
		{
			// Has just left the region - we already handled that above in a different way
			// that was necessary due to subregions of the old cell
			// which might not be part of the new cell
			updateObject->zmapSubCells.erase(subCellIt);
			(*it).second->region->onObjectLeave(updateObject);
		}
		else
		{
			//No change has occurred.
		}
	}

	
}