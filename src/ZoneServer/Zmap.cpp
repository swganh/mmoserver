#include "zmap.h"

#include "Object.h"

zmap* zmap::ZMAP = NULL;

struct SubCell
{
public:
	uint32 subCellId;

	float x;
	float z; 
	float height; 
	float width;

	ZmapSubCellCallback* callback;
};

zmap::zmap()
{
	ZMAP = this;

	uint32 x = 0, i = 0, j = 0;

	
	// Setup the lookup array...
	i = 0;
	for(x = 0; x <= 410; x++)
	{
		for(j = 0; j <= 410; j++)
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

uint32	zmap::AddSubCell(float low_x, float low_z, float height, float width, ZmapSubCellCallback* callback)
{
	SubCell* pSubCell		= new SubCell();
	pSubCell->x				= low_x;
	pSubCell->z				= low_z;
	pSubCell->height		= height;
	pSubCell->width			= width;
	pSubCell->callback		= callback;

	uint32 lowerLeft		= _getCellId(low_x,			low_z);
	uint32 lowerRight		= _getCellId(low_x+width,	low_z);
	uint32 upperLeft		= _getCellId(low_x,			low_z+height);
	uint32 upperRight		= _getCellId(low_x+width,	low_z+height);

	unsigned int cellCountZ = (lowerLeft - upperLeft)/410;
	unsigned int cellCountX = (lowerLeft - lowerRight);

	for(unsigned int i=0; i < cellCountZ; i++)
	{
		for(unsigned int j=0; j < cellCountX; j++)
		{
			subCells.insert(std::make_pair((lowerLeft + j + i * 410), pSubCell));
		}
	}
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
	return zmap_lookup[((((uint32)z) + 8500)/150)][((((uint32)x) + 8500)/150)];
}

void zmap::AddObject(Object *newObject)
{
	//Pesudo
	// 1. Calculate CellID
	// 2. Set CellID
	// 3. Insert object into the cell in the hast table

	uint32 FinalCell = _getCellId(newObject->mPosition.x, newObject->mPosition.y);


	newObject->zmapCellID = FinalCell;
	for(std::list<Object*>::iterator i = ZMapCells[FinalCell].begin(); i != ZMapCells[FinalCell].end(); i++)
	{
		if((*i)->getId() == newObject->getId())
		{
			
			return;
		}

	}
	ZMapCells[FinalCell].push_back(newObject);
	return;

}

void zmap::RemoveObject(Object *removeObject)
{
	MapHandler::iterator i = ZMapCells.find(removeObject->zmapCellID);

	if(i != ZMapCells.end())
	{
		for(std::list<Object*>::iterator i = ZMapCells[removeObject->zmapCellID].begin(); i != ZMapCells[removeObject->zmapCellID].end(); i++)
		{
			if((*i)->getId() == removeObject->getId())
			{
				
				ZMapCells[removeObject->zmapCellID].erase(i);
				break;
			}
		}
	}

	return;
}

void zmap::UpdateObject(Object *updateObject)
{
	// Pesudo
	// 1. Calculate Cell
	// 2. If cell is different then current cell
	//		2a. Remove him from the current cell, insert into new cell at new cell
	//		2b. Update info....

	uint32 FinalCell = _getCellId(updateObject->mPosition.x, updateObject->mPosition.y);
	
	if(updateObject->zmapCellID != FinalCell)
	{
	
		UpdateBackCells(updateObject,FinalCell);
		UpdateFrontCells(updateObject,FinalCell);
		RemoveObject(updateObject);
		AddObject(updateObject);
	
	}

	//We need to check subregions
	std::multimap<uint32, SubCell*>::iterator it;
	std::pair<std::multimap<uint32, SubCell*>::iterator, std::multimap<uint32, SubCell*>::iterator> multi_pairing;

	multi_pairing = subCells.equal_range(updateObject->zmapCellID);

	for(std::multimap<uint32, SubCell*>::iterator it = multi_pairing.first; it != multi_pairing.second; ++it)
	{
		bool isInRegion = false;
		bool isTrulyInRegion = isObjectInSubCell(updateObject, (*it).second->subCellId);

		std::set<uint32>::iterator subCell = updateObject->zmapSubCells.find((*it).second->subCellId);

		if(subCell != updateObject->zmapSubCells.end())
			isInRegion = true;

		if(isTrulyInRegion && !isInRegion)
		{
			//Has just entered the region
			updateObject->zmapSubCells.insert((*it).second->subCellId);
			(*it).second->callback->ZmapCallback_OnEnterSubCell();
		}
		else if(!isTrulyInRegion && isInRegion)
		{
			//Has just left the region
			updateObject->zmapSubCells.erase(subCell);
			(*it).second->callback->ZmapCallback_OnExitSubCell();
		}
		else
		{
			//No change has occurred.
		}
	}
}

std::list<Object*>*	zmap::GetCellContents(uint32 CellID)
{
	//Pesudo
	// 1. Return list of objects in cell

	return &ZMapCells[CellID];
}

std::list<Object*>* zmap::GetChatRangeCellContents(uint32 CellID)
{
	//Pesudo
	// 1. Combine the lists of Neiboring cells to 1

	std::list<Object*>* ReturnList = new std::list<Object*>;
	std::list<Object*>::iterator it = ReturnList->begin();

	std::list<Object*> temp = *GetCellContents(CellID);
	ReturnList->splice(it, temp);

	temp = *GetCellContents(CellID + 411);
	ReturnList->splice(it, temp);

	temp = *GetCellContents(CellID - 411);
	ReturnList->splice(it, temp);

	temp = *GetCellContents(CellID + 1);
	ReturnList->splice(it, temp);

	temp = *GetCellContents(CellID - 1);
	ReturnList->splice(it, temp);

	temp = *GetCellContents(CellID + 410);
	ReturnList->splice(it, temp);

	temp = *GetCellContents(CellID + 412);
	ReturnList->splice(it, temp);

	temp = *GetCellContents(CellID - 412);
	ReturnList->splice(it, temp);

	temp = *GetCellContents(CellID - 410);
	ReturnList->splice(it, temp);


	return ReturnList;
}

void zmap::UpdateBackCells(Object* updateObject, uint32 newCell)
{

	//ZMAP Northbound! TODO: Sync with game
	if((updateObject->zmapCellID + 411) == newCell)
	{
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents((updateObject->zmapCellID - 411));
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 412));
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 410));
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			
			if((*i)->getId() != updateObject->getId())
			{
				/*
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
				*/
			}
		}

		return;
	}

	//ZMAP Southbound! TODO: Sync with game
	else if((updateObject->zmapCellID - 411) == newCell)
	{
		
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();
		
		std::list<Object*> temp = *GetCellContents(updateObject->zmapCellID + 411);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID + 412);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID + 410);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			if((*i)->getId() != updateObject->getId())
			{
				/*
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
				*/
			}
		}

		return;
	}

		//ZMAP Westbound! TODO: Sync with game
	else if((updateObject->zmapCellID - 1) == newCell)
	{
		
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents(updateObject->zmapCellID - 410);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID + 1 );
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID + 412);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			if((*i)->getId() != updateObject->getId())
			{
			/*
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
				*/
			}
		}

		return;
	}

			//ZMAP Eastbound! TODO: Sync with game
	else if((updateObject->zmapCellID + 1) == newCell)
	{
		
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents(updateObject->zmapCellID + 410);
		FinalList.splice(it, temp);
		
		temp = *GetCellContents(updateObject->zmapCellID - 1 );
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID - 412);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			if((*i)->getId() != updateObject->getId())
			{
				/*
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
				*/
			}
		}

		return;
	}

	// NorthEastbound
	else if((updateObject->zmapCellID + 412) == newCell)
	{
		
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents(updateObject->zmapCellID + 410);
		FinalList.splice(it, temp);
		
		temp = *GetCellContents(updateObject->zmapCellID - 410);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID - 1 );
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID - 412);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID - 411);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			if((*i)->getId() != updateObject->getId())
			{
			/*
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
				*/
			}
		}

		return;
	}

	// NorthWestbound
	else if((updateObject->zmapCellID + 410) == newCell)
	{
		
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents(updateObject->zmapCellID - 412);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID - 410);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID + 1 );
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID + 412);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID - 411);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			if((*i)->getId() != updateObject->getId())
			{
				/*
		
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
				*/
			}
		}

		return;
	}

		// SouthWestbound
	else if((updateObject->zmapCellID - 412) == newCell)
	{
		
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents(updateObject->zmapCellID + 410);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID - 410);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID + 1 );
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID + 412);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID + 411);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			if((*i)->getId() != updateObject->getId())
			{
				/*
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
				*/
			}
		}

		return;
	}

		// SouthEestbound
	else if((updateObject->zmapCellID - 410) == newCell)
	{
		
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents(updateObject->zmapCellID + 412);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID + 410);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID - 1 );
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID - 412);
		FinalList.splice(it, temp);

		temp = *GetCellContents(updateObject->zmapCellID + 411);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			if((*i)->getId() != updateObject->getId())
			{
		/*
		(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
				*/
			}
		}

		return;
	}
	else
	{
	
	}
}


void zmap::UpdateFrontCells(Object* updateObject, uint32 newCell)
{

	//ZMAP Northbound! TODO: Sync with game
	if((updateObject->zmapCellID + 411) == newCell)
	{
	
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents((updateObject->zmapCellID + 411) + 411);
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID + 411) + 412);
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID + 411) + 410);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			//printf("OBJECT!!!!!!!!!!!!!!\n");
			if((*i)->getId() != updateObject->getId())
			{
				/*
				(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
				  */
			}
		}

		return;
	}

	//ZMAP Southbound! TODO: Sync with game
	else if((updateObject->zmapCellID - 411) == newCell)
	{
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents((updateObject->zmapCellID - 411) - 411);
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 411) - 412);
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 411) -410);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			if((*i)->getId() != updateObject->getId())
			{
		/*		(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
		  
				if((*i)->ClassType == TYPE_CHARACTER)
				{
					updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				}
		  */
			
			}
		}

		return;
	}

			//ZMAP Eastbound! TODO: Sync with game
	else if((updateObject->zmapCellID + 1) == newCell)
	{
	
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents((updateObject->zmapCellID + 1) + 412);
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID + 1) + 1 );
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID + 1) - 410);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
	
			if((*i)->getId() != updateObject->getId())
			{
			/*	(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
					fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
			  */
			}
		}

		return;
	}

		//ZMAP Westbound! TODO: Sync with game
	else if((updateObject->zmapCellID - 1) == newCell)
	{
		
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents((updateObject->zmapCellID - 1) - 412);
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 1) - 1 );
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 1) + 410);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
		
			if((*i)->getId() != updateObject->getId())
			{
		/*		(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
				*/
			}
		}

		return;
	}

	// NorthEastbound
	else if((updateObject->zmapCellID + 412) == newCell)
	{
		
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents((updateObject->zmapCellID + 412) + 410);//
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID + 412) - 410);//
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID + 412) + 1 );//
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID + 412) + 412);//
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID + 412) + 411);//
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
		
			if((*i)->getId() != updateObject->getId())
			{
		/*		(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
							fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
		*/
			}
		}

		return;
	}

	// NorthWestbound
	else if((updateObject->zmapCellID + 410) == newCell)
	{
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents((updateObject->zmapCellID + 410) - 412);
		FinalList.splice(it, temp);

		std::list<Object*> objList2 = *GetCellContents((updateObject->zmapCellID + 410) + 410);
		FinalList.splice(it, temp);

		std::list<Object*> objList3 = *GetCellContents((updateObject->zmapCellID + 410) - 1 );
		FinalList.splice(it, temp);

		std::list<Object*> objList4 = *GetCellContents((updateObject->zmapCellID + 410) + 412);
		FinalList.splice(it, temp);

		std::list<Object*> objList5 = *GetCellContents((updateObject->zmapCellID + 410) + 411);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
	
			if((*i)->getId() != updateObject->getId())
			{
	/*			(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
				*/
			}
		}

		return;
	}

		// SouthWestbound
	else if((updateObject->zmapCellID - 412) == newCell)
	{
		
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents((updateObject->zmapCellID - 412) + 410);
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 412) - 410);
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 412) - 1 );
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 412) - 412);
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 412) - 411);
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
		
			if((*i)->getId() != updateObject->getId())
			{
		/*		(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
		  */
			}
		}

		return;
	}

		// SouthEestbound
	else if((updateObject->zmapCellID - 410) == newCell)
	{
		
		std::list<Object*> FinalList;
		std::list<Object*>::iterator it = FinalList.end();

		std::list<Object*> temp = *GetCellContents((updateObject->zmapCellID - 410) + 412);//
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 410) - 410);//
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 410) + 1 );//
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 410) - 412);//
		FinalList.splice(it, temp);

		temp = *GetCellContents((updateObject->zmapCellID - 410) - 411);//
		FinalList.splice(it, temp);

		for(std::list<Object*>::iterator i = FinalList.begin(); i != FinalList.end(); i++)
		{
			
			if((*i)->getId() != updateObject->getId())
			{
				/*
				(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
				  */
			}
		}

		return;
	}
}