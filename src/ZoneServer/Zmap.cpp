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

	viewRange = VIEWRANGE;

	uint32 x = 0, i = 0, j = 0;

	
	// Setup the lookup array...
	
	for(x = 0; x <= GRIDWIDTH; x++)
	{
		for(j = 0; j <= GRIDHEIGHT; j++)
		{
			zmap_lookup[x][j] = i;
			ObjectStruct* bucket = new(ObjectStruct);
			ZMapCells.insert(std::make_pair(i,bucket));
			i++;

		}
	}
}

zmap::~zmap()
{
	uint32 x = 0, i = 0, j = 0;

	for(x = 0; x <= GRIDWIDTH; x++)
	{
		for(j = 0; j <= GRIDHEIGHT; j++)
		{
			std::map<uint32, ObjectStruct*>::iterator it = ZMapCells.find(i);
	
			
			(*it).second->Creatures.clear();
			(*it).second->Players.clear();
			(*it).second->Objects.clear();
			(*it).second->SubCells.clear();
			
			delete(	ZMapCells[i]);
			i++;

		}
	}
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

			std::map<uint32, ObjectStruct*>::iterator it = ZMapCells.find((lowerLeft + j + i * GRIDWIDTH));
			
			(*it).second->SubCells.push_back(pSubCell);
	
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

	multi_pairing = subCells.equal_range(object->getGridBucket());

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
					std::map<uint32, ObjectStruct*>::iterator it = ZMapCells.find((lowerLeft + j + i * GRIDWIDTH));
			
					SubcellListType				cellList =  (*it).second->SubCells;
					
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
	return zmap_lookup[( ( ((uint32)z) + (MAPWIDTH/2) )/GRIDWIDTH)][((((uint32)x) + (MAPHEIGHT/2))/GRIDHEIGHT)];
}


void zmap::RemoveObject(Object *removeObject)
{
	uint32 cellId = removeObject->getGridBucket();

	if(!GetCellValidFlag(cellId))
	{
		gLogger->log(LogManager::DEBUG,"zmap::RemoveObject :: bucket %u NOT valid", cellId);
		return;
	}

	ObjectListType *list;

	std::map<uint32, ObjectStruct*>::iterator it = ZMapCells.find(cellId);

	switch(removeObject->getType())
	{
		case ObjType_Player:
			{
				list = &(*it).second->Players;
			}
			break;
			
		case ObjType_Creature:
		case ObjType_NPC:
			{
				list = &(*it).second->Creatures;
			}
			break;

		default:
			{
				list = &(*it).second->Objects;
			}
			break;
	}
	

	for(ObjectListType::iterator it = list->begin(); it != list->end(); it++)
	{
		if((*it)->getId() == removeObject->getId())
		{
			list->erase(it);
			break;
		}
	}

	return;
}


void zmap::GetCellContents(uint32 CellID, ObjectListType* list, uint32 type)
{
	if(CellID > (GRIDWIDTH*GRIDHEIGHT))
	{
		gLogger->log(LogManager::DEBUG,"zmap::GetAllCellContents :: bucket %u out of grid", CellID);
		return;
	}

	//gLogger->log(LogManager::DEBUG,"zmap::GetAllCellContents :: bucket %u", CellID);
	
	ObjectListType::iterator it = list->begin();

	std::map<uint32, ObjectStruct*>::iterator mapIt = ZMapCells.find(CellID);
	
	//make a copy of the list !!!!
	//splice removes the entries!!!
	ObjectListType listCopy;

	if(type&Bucket_Objects)
	{
		listCopy = (*mapIt).second->Objects;
		list->splice(it, listCopy);
	}

	if(type&Bucket_Players)
	{
		listCopy = (*mapIt).second->Players;
		list->splice(it, listCopy);
	}

	if(type&Bucket_Creatures)
	{
		listCopy = (*mapIt).second->Creatures;
		list->splice(it, listCopy);
	}
	
}



void	zmap::GetPlayerCellContents(uint32 CellID, ObjectListType* list)
{
	//Pesudo
	// 1. Return list of objects in cell
	//if(CellID) >
	
	if(CellID > (GRIDWIDTH*GRIDHEIGHT))
		return;
	
	ObjectListType::iterator it = list->begin();

	std::map<uint32, ObjectStruct*>::iterator mapIt = ZMapCells.find(CellID);
	
	ObjectListType listCopy;

	listCopy = (*mapIt).second->Players;
	list->splice(it, listCopy);

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

void	zmap::GetChatRangeCellContents(uint32 CellID, ObjectListType* list)
{
	//Pesudo
	// 1. Combine the lists of Neighboring cells to 1

	GetPlayerCellContents(CellID, list);

	GetPlayerCellContents(CellID + GRIDWIDTH, list);

	GetPlayerCellContents(CellID - GRIDWIDTH, list);
	
	GetPlayerCellContents(CellID + 1, list);

	GetPlayerCellContents(CellID - 1, list);

	GetPlayerCellContents(CellID + GRIDWIDTH-1, list);

	GetPlayerCellContents(CellID + GRIDWIDTH+1, list);

	GetPlayerCellContents(CellID - GRIDWIDTH+1, list);

	GetPlayerCellContents(CellID - GRIDWIDTH-1, list);

}

//=====================================================
//Get Contents for a Row based on a middle cell
//depending on how far we are away of the player we need to resize the row
//this will be done by the var iteration
void zmap::GetGridContentsListRow(uint32 CellID, ObjectListType* list, uint32 type)
{
	GetCellContents(CellID,list, type);
	
	for(int i = 1; i <= viewRange; i++)
	{
		GetCellContents(CellID + i,list, type);
		GetCellContents(CellID - i,list, type);	
	}
}


void	 zmap::GetPlayerGridContentsListRow(uint32 CellID, ObjectListType* list)
{
	GetPlayerCellContents(CellID,list);
	
	for(int i = 1; i <= viewRange; i++)
	{		
		GetPlayerCellContents(CellID + i,list);
		GetPlayerCellContents(CellID - i,list);	
	}

}


//==========================================================================
//column downwards
//when we move along the edges
///15.6 sch
//
void	zmap::GetPlayerGridContentsListColumnDown(uint32 CellID, ObjectListType* list)
{
	
	GetPlayerCellContents(CellID, list);

	for(int i = 1; i <= (viewRange*2)-1; i++)
	{		
		GetPlayerCellContents(CellID - (i*GRIDWIDTH), list);
	}
}


void	zmap::GetGridContentsListColumnDown(uint32 CellID, ObjectListType* list, uint32 type)
{
	GetCellContents(CellID, list, type);

	for(int i = 1; i <= (viewRange*2)-1; i++)
	{		
		GetCellContents(CellID - (i*GRIDWIDTH), list, type);
	}
}

void	zmap::GetGridContentsListColumnUp(uint32 CellID, ObjectListType* list, uint32 type)
{

	GetCellContents(CellID, list, type);

	for(int i = 1; i <= (viewRange*2)-1; i++)
	{		
		GetCellContents(CellID + (i*GRIDWIDTH), list, type);
	}
}

void	zmap::GetPlayerGridContentsListColumnUp(uint32 CellID, ObjectListType* list)
{
	
	GetPlayerCellContents(CellID, list);

	for(int i = 1; i <= (viewRange*2)-1; i++)
	{		
		GetPlayerCellContents(CellID + (i*GRIDWIDTH), list);
	}

}



//===============================================================0
// when getting content on the edges just spare the *middle* (pointy) cell
void	zmap::GetGridContentsListRowLeft(uint32 CellID, ObjectListType* list, uint32 type)
{

	GetCellContents(CellID, list, type);

	for(int i = 1; i < ((viewRange*2)-1); i++)
	{
		GetCellContents(CellID - i, list, type);
	}

}

void	zmap::GetPlayerGridContentsListRowLeft(uint32 CellID, ObjectListType* list)
{
	
	GetPlayerCellContents(CellID, list);

	for(int i = 1; i < ((viewRange*2)-1); i++)
	{
		GetPlayerCellContents(CellID - i, list);
	}

}


void	zmap::GetGridContentsListRowRight(uint32 CellID, ObjectListType* list, uint32 type)
{

	GetCellContents(CellID, list, type);

	for(int i = 1; i < ((viewRange*2)-1); i++)
	{
		GetCellContents(CellID + i, list, type);
	}

}

void	zmap::GetPlayerGridContentsListRowRight(uint32 CellID, ObjectListType* list)
{
	
	GetPlayerCellContents(CellID, list);

	for(int i = 1; i < ((viewRange*2)-1); i++)
	{
		GetPlayerCellContents(CellID + i, list);
	}

}


void	zmap::GetGridContentsListColumn(uint32 CellID, ObjectListType* list, uint32 type)
{

	GetCellContents(CellID, list, type);

	for(int i = 1; i <= viewRange; i++)
	{
		GetCellContents(CellID + (i*GRIDWIDTH), list, type);
		
		GetCellContents(CellID - (i*GRIDWIDTH), list, type);
	}

}

void	zmap::GetPlayerGridContentsListColumn(uint32 CellID, ObjectListType* list)
{

	GetPlayerCellContents(CellID, list);

	for(int i = 1; i <= viewRange; i++)
	{
		GetPlayerCellContents(CellID + (i*GRIDWIDTH), list);		
		
		GetPlayerCellContents(CellID - (i*GRIDWIDTH), list);
	}

}

void	zmap::GetViewingRangeCellContents(uint32 CellID, ObjectListType* list, uint32 type)
{
	//gLogger->log(LogManager::DEBUG,"zmap::GetAllViewingRangeCellContents :: bucket %u", CellID);

	GetGridContentsListRow(CellID, list, type);

	for(int i = 1; i <= viewRange; i++)
	{
		GetGridContentsListRow(CellID + (i*GRIDWIDTH), list, type);
		
		GetGridContentsListRow(CellID - (i*GRIDWIDTH), list, type);
	}
}

void	zmap::GetPlayerViewingRangeCellContents(uint32 CellID, ObjectListType* list)
{

	GetPlayerGridContentsListRow(CellID, list);

	for(int i = 1; i <= viewRange; i++)
	{
		GetPlayerGridContentsListRow(CellID + (i*GRIDWIDTH), list);
	
		GetPlayerGridContentsListRow(CellID - (i*GRIDWIDTH), list);
	}

}


// limited to max viewing range for now
//
void	zmap::GetCustomRangeCellContents(uint32 CellID, uint32 range, ObjectListType* list, uint32 type)
{
	// query the grid with the custom range
	// TODO: need any failsafes concerning the cells???

	if(range > VIEWRANGE)
		range = VIEWRANGE;

	GetGridContentsListRow(CellID, list, type);

	for(uint32 i = 0; i < range; i++)
	{
		GetGridContentsListRow(CellID + (i*GRIDWIDTH), list, type);
		
		GetGridContentsListRow(CellID - (i*GRIDWIDTH), list, type);
	}

}





uint32 zmap::AddObject(Object *newObject)
{
	
	uint32 finalBucket = _getCellId(newObject->getWorldPosition().x, newObject->getWorldPosition().z);

	if(!GetCellValidFlag(finalBucket))
	{
		//something fishy here
		assert(false && "zmap::AddObject :: couldnt find grid cell :(");
		return 0xffffffff;
	}
	
	newObject->setGridBucket(finalBucket);
	
	ObjectListType* list;

	std::map<uint32, ObjectStruct*>::iterator mapIt = ZMapCells.find(finalBucket);

	switch(newObject->getType())
	{
		case ObjType_Player:
		{
			list = &(*mapIt).second->Players;
		}
		break;

		case ObjType_Creature:
		case ObjType_NPC:
		{
			list = &(*mapIt).second->Creatures;
		}
		break;

		default:
		{
			list = &(*mapIt).second->Objects;
		}
		break;
	}

	//this *is* certainly stupid, *but*
	//the most important thing, is that the reads are fast, thus, a list
	for(ObjectListType::iterator i = list->begin(); i != list->end(); i++)
	{
		if((*i)->getId() == newObject->getId())
		{
			gLogger->log(LogManager::DEBUG,"zmap::AddObject :: add Object %I64u to bucket %u failed because Object was already in bucket",newObject->getId(), finalBucket);
			return 0xffffffff;
		}
	}
	
	list->push_back(newObject);

	//have we just entered a region ???

	SubcellListType				cellList = (*mapIt).second->SubCells;
					
	SubcellListType::iterator	CellListit = cellList.begin();

	while(CellListit != cellList.begin())
	{
		if(isObjectInSubCell(newObject,(*CellListit)->subCellId))
		{
			(*CellListit)->region->onObjectEnter(newObject);
		}
		CellListit++;
	}


	return finalBucket;
}

void zmap::UpdateObject(Object *updateObject)
{
	

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
	
	uint32 newBucket	= getCellId(position.x, position.z);
	uint32 oldBucket		= updateObject->getGridBucket();

	//no need for an update
	if(newBucket == oldBucket)
	{
		return;
	}

	//get out of old bucket
	RemoveObject(updateObject);

	//put into new bucket
	updateObject->setGridBucket(newBucket);
	
	ObjectListType* list;

	std::map<uint32, ObjectStruct*>::iterator mapIt = ZMapCells.find(newBucket);

	if(updateObject->getType() == ObjType_Player)
	{
		list = &(*mapIt).second->Players;
	}
	else
	{
		list = &(*mapIt).second->Objects;
	}

	
	//this *is* certainly stupid, *but*
	//the most important thing, is that the reads are fast, thus, a list
	for(ObjectListType::iterator i = list->begin(); i != list->end(); i++)
	{
		if((*i)->getId() == updateObject->getId())
		{
			return;
		}
	}
	
	list->push_back(updateObject);

	
	//update sucells (regions)
	std::multimap<uint32, SubCell*>::iterator it;

	//remove any old subcells that are not in the new cell

	//iterate through the subcells the object is in
	Uint32Set::iterator subCellIt = updateObject->zmapSubCells.begin();
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

			subCellIt = updateObject->zmapSubCells.erase(subCellIt);
			
		}
		else
			subCellIt++;
	}

	if(updateObject->getType() != ObjType_Player)
		return;

	//We need to check subregions -> New cell
	
	std::pair<std::multimap<uint32, SubCell*>::iterator, std::multimap<uint32, SubCell*>::iterator> multi_pairing;

	multi_pairing = subCells.equal_range(updateObject->getGridBucket());

	for(std::multimap<uint32, SubCell*>::iterator it = multi_pairing.first; it != multi_pairing.second; ++it)
	{
		bool isInRegion = false;
		bool isTrulyInRegion = isObjectInSubCell(updateObject, (*it).second->subCellId);

		Uint32Set::iterator subCellIt = updateObject->zmapSubCells.find((*it).second->subCellId);

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
			subCellIt = updateObject->zmapSubCells.erase(subCellIt);
			(*it).second->region->onObjectLeave(updateObject);
		}
		else
		{
			//No change has occurred.
		}
	}

	
}