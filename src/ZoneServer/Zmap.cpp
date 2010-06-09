#include "zmap.h"

#include "Object.h"

zmap* zmap::ZMAP = NULL;

zmap::zmap()
{
	ZMAP = this;

	uint32 x = 0, i = 0, j = 0;

	ZoneLogs = fopen("ZoneMapLog.txt","w");

	printf("ZMAP: Mallocing Lookup array!\n");

	// Setup the lookup array...
	i = 0;
	for(x = 0; x <= 410; x++)
	{
		for(j = 0; j <= 410; j++)
		{
			zmap_lookup[x][j] = i;
			fprintf(ZoneLogs,"zmap_lookup[%d][%d] = %d \n",x,j,i);
			i++;

		}
	}

	printf("ZMAP: End Mallocing Lookup array!\n");
}

zmap::~zmap()
{
	fclose(ZoneLogs);
}


void zmap::AddObject(Object *newObject)
{
	//Pesudo
	// 1. Calculate CellID
	// 2. Set CellID
	// 3. Insert object into the cell in the hast table

	uint32 CellX = ((((uint32)newObject->mPosition.x) + 8500)/150);
	uint32 CellY = ((((uint32)newObject->mPosition.y) + 8500)/150);
	uint32 FinalCell = zmap_lookup[CellY][CellX];

	fprintf(ZoneLogs,"==================ZMap Add Object========================\n");
	fprintf(ZoneLogs,"getId() : %d \n",newObject->getId());
	fprintf(ZoneLogs,"Cell X : %d \n",CellX);
	fprintf(ZoneLogs,"Cell Y : %d \n",CellY);
	fprintf(ZoneLogs,"CellID : %d \n",FinalCell);
	fprintf(ZoneLogs,"=========================================================\n\n");


	newObject->zmapCellID = FinalCell;
	for(std::list<Object*>::iterator i = ZMapCells[FinalCell].begin(); i != ZMapCells[FinalCell].end(); i++)
	{
		if((*i)->getId() == newObject->getId())
		{
			fprintf(ZoneLogs,"Multiple insertion for one object attempted on zmap, object id was %d \n",newObject->getId());
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
				fprintf(ZoneLogs,"Removing Item %d \n" , (*i)->getId());
				ZMapCells[removeObject->zmapCellID].erase(i);
				break;
			}
		}
	}
	else
	{
		fprintf(ZoneLogs,"ZMAP FATAL ERROR: Cell %d does not exist!!! [1] \n",removeObject->zmapCellID);
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

	uint32 CellX = ((((uint32)updateObject->mPosition.x) + 8500)/150);
	uint32 CellY = ((((uint32)updateObject->mPosition.y) + 8500)/150);
	uint32 FinalCell = zmap_lookup[CellY][CellX];
	
	if(updateObject->zmapCellID != FinalCell)
	{
		fprintf(ZoneLogs,"==========================ZMAP UPDATE==========================\n");
		fprintf(ZoneLogs,"Object %d moved into new cell!\n",updateObject->getId());
		fprintf(ZoneLogs,"New Cell is %d \n",FinalCell);
		UpdateBackCells(updateObject,FinalCell);
		UpdateFrontCells(updateObject,FinalCell);
		RemoveObject(updateObject);
		AddObject(updateObject);
		fprintf(ZoneLogs,"================================================================\n\n");
	}
	else
	{
		fprintf(ZoneLogs,"=========================ZMAP UPDATE============================\n");
		fprintf(ZoneLogs,"No need for update, object is in same cell! :)\n");
		fprintf(ZoneLogs,"Object still in cell %d \n",FinalCell);
		fprintf(ZoneLogs,"================================================================\n\n");
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

	fprintf(ZoneLogs,"Request for list of objects in cell.\n");
	for(std::list<Object*>::iterator i = ReturnList->begin(); i != ReturnList->end(); i++)
	{
		fprintf(ZoneLogs,"Object [%u] \n",(*i)->getId());
	}
	fprintf(ZoneLogs,"Request Complete.\n");


	return ReturnList;
}

void zmap::UpdateBackCells(Object* updateObject, uint32 newCell)
{

	//ZMAP Northbound! TODO: Sync with game
	if((updateObject->zmapCellID + 411) == newCell)
	{
	
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		fprintf(ZoneLogs,"Northbound!\n");
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			fprintf(ZoneLogs,"OBJECT!!!!!!!!!!!!!!\n");
			if((*i)->getId() != updateObject->getId())
			{
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
			}
		}

		return;
	}

	//ZMAP Southbound! TODO: Sync with game
	else if((updateObject->zmapCellID - 411) == newCell)
	{
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		fprintf(ZoneLogs,"Southbound!\n");
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
				fprintf(ZoneLogs,"OBJECT!!!!!!!!!!!!!!\n");
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
			}
		}

		return;
	}

		//ZMAP Westbound! TODO: Sync with game
	else if((updateObject->zmapCellID - 1) == newCell)
	{
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		fprintf(ZoneLogs,"Westbound!\n");
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			fprintf(ZoneLogs,"OBJECT!!!!!!!!!!!!!!\n");
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
			}
		}

		return;
	}

			//ZMAP Eastbound! TODO: Sync with game
	else if((updateObject->zmapCellID + 1) == newCell)
	{
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		fprintf(ZoneLogs,"Eastbound!\n");
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			fprintf(ZoneLogs,"OBJECT!!!!!!!!!!!!!!\n");
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
			}
		}

		return;
	}

	// NorthEastbound
	else if((updateObject->zmapCellID + 412) == newCell)
	{
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		fprintf(ZoneLogs,"NorthEast bound!\n");
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			fprintf(ZoneLogs,"OBJECT!!!!!!!!!!!!!!\n");
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
			}
		}

		return;
	}

	// NorthWestbound
	else if((updateObject->zmapCellID + 410) == newCell)
	{
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		fprintf(ZoneLogs,"NorthWest bound!\n");
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			fprintf(ZoneLogs,"OBJECT!!!!!!!!!!!!!!\n");
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
			}
		}

		return;
	}

		// SouthWestbound
	else if((updateObject->zmapCellID - 412) == newCell)
	{
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("SouthWest bound!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
			}
		}

		return;
	}

		// SouthEestbound
	else if((updateObject->zmapCellID - 410) == newCell)
	{
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("SouthEast bound!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
				(*i)->Despawn((*i)->getId(),updateObject->getId()); //Despawn them

				if((*i)->ClassType == TYPE_CHARACTER)
				{
					(*i)->pkt.QUEUE_SceneDestroyObject(&updateObject->getId());
					(*i)->pkt.SEND_QueuedPackets((*i)->getId(),(*i)->getId(),false);
				}
			}
		}

		return;
	}
	else
	{
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("CRAP!!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	}
}


void zmap::UpdateFrontCells(Object* updateObject, uint32 newCell)
{

	//ZMAP Northbound! TODO: Sync with game
	if((updateObject->zmapCellID + 411) == newCell)
	{
	
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("Northbound!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			printf("OBJECT!!!!!!!!!!!!!!\n");
			if((*i)->getId() != updateObject->getId())
			{
				(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
				
			}
		}

		return;
	}

	//ZMAP Southbound! TODO: Sync with game
	else if((updateObject->zmapCellID - 411) == newCell)
	{
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("Southbound!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			printf("OBJECT!!!!!!!!!!!!!!\n");
			if((*i)->getId() != updateObject->getId())
			{
				(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				{
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
				}
			}
		}

		return;
	}

			//ZMAP Eastbound! TODO: Sync with game
	else if((updateObject->zmapCellID + 1) == newCell)
	{
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("Eastbound!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			printf("OBJECT!!!!!!!!!!!!!!\n");
			if((*i)->getId() != updateObject->getId())
			{
				(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
					fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);

			}
		}

		return;
	}

		//ZMAP Westbound! TODO: Sync with game
	else if((updateObject->zmapCellID - 1) == newCell)
	{
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("Westbound!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			printf("OBJECT!!!!!!!!!!!!!!\n");
			if((*i)->getId() != updateObject->getId())
			{
				(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
			}
		}

		return;
	}

	// NorthEastbound
	else if((updateObject->zmapCellID + 412) == newCell)
	{
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("NorthEast bound!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			printf("OBJECT!!!!!!!!!!!!!!\n");
			if((*i)->getId() != updateObject->getId())
			{
				(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
							fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
			}
		}

		return;
	}

	// NorthWestbound
	else if((updateObject->zmapCellID + 410) == newCell)
	{
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("NorthWest bound!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			printf("OBJECT!!!!!!!!!!!!!!\n");
			if((*i)->getId() != updateObject->getId())
			{
				(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
			}
		}

		return;
	}

		// SouthWestbound
	else if((updateObject->zmapCellID - 412) == newCell)
	{
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("SouthWest bound!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			printf("OBJECT!!!!!!!!!!!!!!\n");
			if((*i)->getId() != updateObject->getId())
			{
				(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
		
			}
		}

		return;
	}

		// SouthEestbound
	else if((updateObject->zmapCellID - 410) == newCell)
	{
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("SouthEast bound!\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
			printf("OBJECT!!!!!!!!!!!!!!\n");
			if((*i)->getId() != updateObject->getId())
			{
				(*i)->Spawn((*i)->getId(),updateObject->getId()); //Spawn them
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",(*i)->getId(),(*i)->zmapCellID);
				if((*i)->ClassType == TYPE_CHARACTER)
				updateObject->Spawn(updateObject->getId(),(*i)->getId()); //Have them Spawn you
				fprintf(ZoneLogs,"Spawning Object [%u][%u] \n",updateObject->getId(),updateObject->zmapCellID);
			
			}
		}

		return;
	}


	else
	{
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		fprintf(ZoneLogs,"CRAP!!\n");
		fprintf(ZoneLogs,"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		fprintf(ZoneLogs,"New Cell = %u \n",newCell);
	}
}