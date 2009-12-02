
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "ObjectFactory.h"
#include "HarvesterObject.h"
#include "PlayerObject.h"
#include "Inventory.h"
#include "ResourceContainer.h"
#include "StructureManager.h"
#include "UIManager.h"
#include "MathLib/Quaternion.h"
#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"


//=============================================================================

HarvesterObject::HarvesterObject() : PlayerStructure()
{
	mType = ObjType_Harvester;
	mCurrentExtractionRate = 0.0;
	
}

//=============================================================================

HarvesterObject::~HarvesterObject()
{

}



//=============================================================================
// gets the spec extractionrate
bool HarvesterObject::checkResourceList(uint64 id)
{

	HResourceList::iterator it = mResourceList.begin();
	while (it != mResourceList.end())
	{
		if((*it).first == id)
		{
			return true;
		}
		it++;
	}
	return false;
	
}


//=============================================================================
// gets the spec extractionrate

float HarvesterObject::getSpecExtraction()
{

	if(!this->hasAttribute("harvester_efficiency"))
	{
		this->addAttribute("harvester_efficiency","3.0");
	}

	return this->getAttribute<float>("harvester_efficiency");

}

//=============================================================================
// gets the spec hoppersize

float HarvesterObject::getHopperSize()
{

	if(!this->hasAttribute("examine_hoppersize"))
	{
		this->addAttribute("examine_hoppersize","3000.0");
	}

	return this->getAttribute<float>("examine_hoppersize");

}

//=============================================================================
// gets the current hoppersize

float HarvesterObject::getCurrentHopperSize()
{
	float resourceAmount = 0.0;
	//count all the resource data
	HResourceList::iterator it = mResourceList.begin();
	while (it != mResourceList.end())
	{
		resourceAmount += (*it).second;
		it++;

	}
	return   (resourceAmount);

}

//=============================================================================
//handles the radial selection

void HarvesterObject::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject);
	if(!player)
	{	
		gLogger->logMsgF("HarvesterObject::handleObjectMenuSelect::could not find player",MSG_HIGH);
		return;
	}
	
	switch(messageType)
	{
		case radId_serverTerminalManagementDestroy: 
		{
			StructureAsyncCommand command;
			command.Command = Structure_Command_Destroy;
			command.PlayerId = player->getId();
			command.StructureId = this->getId();

			gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
			
		}
		break;
		case radId_serverTerminalPermissionsAdmin:
		{			
			StructureAsyncCommand command;
			command.Command = Structure_Command_PermissionAdmin;
			command.PlayerId = player->getId();
			command.StructureId = this->getId();

			gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

		}
		break;

		case radId_serverTerminalPermissionsHopper:
		{			
			StructureAsyncCommand command;
			command.Command = Structure_Command_PermissionHopper;
			command.PlayerId = player->getId();
			command.StructureId = this->getId();

			gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

		}
		break;

		case radId_setName:
		{

			StructureAsyncCommand command;
			command.Command = Structure_Command_RenameStructure;
			command.PlayerId = player->getId();
			command.StructureId = this->getId();

			gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

			
		}
		break;

		case radId_operateHarvester:
		{
			  gMessageLib->sendOperateHarvester(this,player);
		}
		break;
		
	}
}

//=============================================================================
// prepares the custom radial for our harvester
void HarvesterObject::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(creatureObject);
	if(!player)
	{	
		gLogger->logMsgF("HarvesterObject::handleObjectMenuSelect::could not find player",MSG_HIGH);
		return;
	}
	
	RadialMenu* radial	= new RadialMenu();
			
	
	//radId_serverHouseManage
	radial->addItem(1,0,radId_examine,radAction_Default,"");
	radial->addItem(2,0,radId_serverHarvesterManage,radAction_ObjCallback,"Structure Management");
	radial->addItem(3,0,radId_serverTerminalManagement,radAction_ObjCallback,"Structure Permissions");
	
	radial->addItem(4,2,radId_serverTerminalManagementDestroy,radAction_ObjCallback,"destroy");//destroy
	radial->addItem(5,2,radId_setName,radAction_ObjCallback,"Rename Structure");//destroy
	radial->addItem(6,2,radId_operateHarvester,radAction_ObjCallback,"operate Harvester");//destroy
	
	radial->addItem(7,3,radId_serverTerminalPermissionsAdmin,radAction_ObjCallback,"Admin List");//destroy
	radial->addItem(8,3,radId_serverTerminalPermissionsHopper,radAction_ObjCallback,"Hopper List");//destroy
	


	RadialMenuPtr radialPtr(radial);
	mRadialMenu = radialPtr;
}


//=============================================================================
// creates resource containers ín our inventory
void HarvesterObject::createResourceContainer(uint64 resID, PlayerObject* player, uint32 amount)
{
	//now create the resource container

	ObjectList*	invObjects = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getObjects();
	ObjectList::iterator listIt = invObjects->begin();

	uint32 rAmount = amount;

	bool	foundSameType	= false;

	while(listIt != invObjects->end())
	{
		// we are looking for resource containers
		if(ResourceContainer* resCont = dynamic_cast<ResourceContainer*>(*listIt))
		{
			uint32 targetAmount	= resCont->getAmount();
			uint32 maxAmount	= resCont->getMaxAmount();
			uint32 newAmount;

			if((resCont->getResourceId() == resID))
			{
			   //find out how much we can add to the container
				uint32 addAmount = maxAmount - targetAmount;

				if(addAmount >rAmount)
				{
					addAmount = rAmount;
					rAmount = 0;
				}
				else
					rAmount -= addAmount;

				if(addAmount)
				{
					// find out how big our container is now
					newAmount = targetAmount + addAmount;

					// update target container
					resCont->setAmount(newAmount);

					gMessageLib->sendResourceContainerUpdateAmount(resCont,player);

					gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE resource_containers SET amount=%u WHERE id=%I64u",newAmount,resCont->getId());
				}
			}
		}

		++listIt;
	}

	// or need to create a new one
	
	while(rAmount)
	{
		uint32 a = 100000;
	
		if( a > rAmount)
			a = rAmount;

		gObjectFactory->requestNewResourceContainer(dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)),resID,player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)->getId(),99,a);
		rAmount -= a;
	
	}
	

}

void HarvesterObject::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	StructureManagerAsyncContainer* asynContainer = (StructureManagerAsyncContainer*)ref;

	switch(asynContainer->mQueryType)
	{

		case Structure_ResourceRetrieve:
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));
			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(asynContainer->mStructureId));

			DataBinding* binding = gWorldManager->getDatabase()->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);

			uint64 count;
			uint32 error;
			count = result->getRowCount();

			if(!count)
			{
				assert(false);
				return;
			}

			result->GetNextRow(binding,&error);
			if(error > 0)
			{
				//mmh there was something fishy ... no changes db side
				gMessageLib->sendResourceEmptyHopperResponse(harvester,player,error, asynContainer->command.b1, asynContainer->command.b2);
				return;
			}
			
			createResourceContainer(asynContainer->command.ResourceId, player, asynContainer->command.Amount);
		
			
			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_ResourceDiscardUpdateHopper,player->getClient());
			asyncContainer->mStructureId	= asynContainer->mStructureId;
			asyncContainer->mPlayerId		= asynContainer->mPlayerId;	
			asyncContainer->command			= asynContainer->command;

			gWorldManager->getDatabase()->ExecuteSqlAsync(harvester,asyncContainer,"SELECT hr.resourceID, hr.quantity FROM harvester_resources hr WHERE hr.ID = '%"PRIu64"' ",harvester->getId());

		}
		break;

		case Structure_ResourceDiscard:
		{
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));
			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(asynContainer->mStructureId));

			DataBinding* binding = gWorldManager->getDatabase()->CreateDataBinding(1);
			binding->addField(DFT_uint32,0,4);

			uint64 count;
			uint32 error;
			count = result->getRowCount();

			if(!count)
			{
				assert(false);
				return;
			}

			result->GetNextRow(binding,&error);
			if(result > 0)
			{
				//mmh there was something fishy ... no changes db side
				gMessageLib->sendResourceEmptyHopperResponse(harvester,player,error, asynContainer->command.b1, asynContainer->command.b2);
				return;
			}

			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_ResourceDiscardUpdateHopper,player->getClient());
			asyncContainer->mStructureId	= asynContainer->mStructureId;
			asyncContainer->mPlayerId		= asynContainer->mPlayerId;	
			asyncContainer->command			= asyncContainer->command;
			
			gWorldManager->getDatabase()->ExecuteSqlAsync(harvester,asyncContainer,"SELECT hr.resourceID, hr.quantity FROM harvester_resources hr WHERE hr.ID = '%"PRIu64"' ",harvester->getId());
		}
		break;

		case Structure_ResourceDiscardUpdateHopper:
		{
			uint64 count = result->getRowCount();

			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(asynContainer->mStructureId));
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

			DataBinding* binding = gWorldManager->getDatabase()->CreateDataBinding(2);
			binding->addField(DFT_uint64,offsetof(HarvesterHopperItem,ResourceID),8,0);
			binding->addField(DFT_float,offsetof(HarvesterHopperItem,Quantity),4,1);
			
			HResourceList*	hRList = harvester->getResourceList();
			
			hRList->clear();

			HarvesterHopperItem hopperTemp;
			for(uint64 i=0;i <count;i++)
			{
				result->GetNextRow(binding,&hopperTemp);
				hRList->push_back(std::make_pair(hopperTemp.ResourceID,hopperTemp.Quantity));
			}

			//now send the update to the client
			gMessageLib->SendHarvesterHopperUpdate(harvester,player);

			gMessageLib->sendResourceEmptyHopperResponse(harvester,player,0, asynContainer->command.b2, asynContainer->command.b2);
			
			gWorldManager->getDatabase()->DestroyDataBinding(binding);												   	

		}
		break;
		
		case Structure_GetResourceData:
		{
			uint64 count = result->getRowCount();

			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(asynContainer->mStructureId));
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

			DataBinding* binding = gWorldManager->getDatabase()->CreateDataBinding(2);
			binding->addField(DFT_uint64,offsetof(HarvesterHopperItem,ResourceID),8,0);
			binding->addField(DFT_float,offsetof(HarvesterHopperItem,Quantity),4,1);
			
			HResourceList*	hRList = harvester->getResourceList();
			
			hRList->clear();

			HarvesterHopperItem hopperTemp;
			for(uint64 i=0;i <count;i++)
			{
				result->GetNextRow(binding,&hopperTemp);
				hRList->push_back(std::make_pair(hopperTemp.ResourceID,hopperTemp.Quantity));
			}

			//now send the update to the client
			gMessageLib->sendHarvesterResourceData(harvester,player);
			gMessageLib->sendBaselinesHINO_7(harvester,player);
			
			gWorldManager->getDatabase()->DestroyDataBinding(binding);												   	

		}
		break;

		//read in the current resource hoppers contents
		case Structure_HopperUpdate:
		{
			uint64 count = result->getRowCount();

			HarvesterObject* harvester = dynamic_cast<HarvesterObject*>(gWorldManager->getObjectById(asynContainer->mStructureId));
			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));

			DataBinding* binding = gWorldManager->getDatabase()->CreateDataBinding(2);
			binding->addField(DFT_uint64,offsetof(HarvesterHopperItem,ResourceID),8,0);
			binding->addField(DFT_float,offsetof(HarvesterHopperItem,Quantity),4,1);
			
			HResourceList*	hRList = harvester->getResourceList();
			
			hRList->clear();

			HarvesterHopperItem hopperTemp;
			for(uint64 i=0;i <count;i++)
			{
				result->GetNextRow(binding,&hopperTemp);
				hRList->push_back(std::make_pair(hopperTemp.ResourceID,hopperTemp.Quantity));
			}

			//now send the update to the client
			gMessageLib->SendHarvesterHopperUpdate(harvester,player);
			
			gWorldManager->getDatabase()->DestroyDataBinding(binding);												   	
		}
		break;

		case Structure_Query_Hopper_Data:
		{
			//PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

			string playerName;
			DataBinding* binding = gWorldManager->getDatabase()->CreateDataBinding(1);
			binding->addField(DFT_bstring,0,64);

			uint64 count;
			count = result->getRowCount();

			for(uint64 i = 0;i < count;i++)
			{
				result->GetNextRow(binding,&playerName);

				addStructureHopper(playerName);

			}

			sendStructureHopperList(asynContainer->mPlayerId);

			gWorldManager->getDatabase()->DestroyDataBinding(binding);
		}
		break;

		case Structure_HopperDiscard:
		{
			//PlayerStructure* structure = dynamic_cast<PlayerStructure*>(gWorldManager->getObjectById(asynContainer->mStructureId));

			PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(asynContainer->mPlayerId));
			if(!player)
			{
				return;
			}
	
			StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(Structure_HopperUpdate,player->getClient());

			asyncContainer->mStructureId	= this->getId();
			asyncContainer->mPlayerId		= player->getId();
			
			int8 sql[250];
			sprintf(sql,"SELECT hr.resourceID, hr.quantity FROM harvester_resources hr WHERE hr.ID = '%"PRIu64"' ",this->getId());
			
			gWorldManager->getDatabase()->ExecuteSqlAsync(this,asyncContainer,sql);	

		
		}
		break;


		default:break;

	}

	SAFE_DELETE(asynContainer);
}


/*

void HarvesterObject::handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window)
{

	PlayerObject* playerObject = window->getOwner();

	if(!playerObject || action || playerObject->getSurveyState() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead())
	{
		return;
	}

	switch(window->getWindowType())
	{

		case SUI_Window_Structure_Delete:
		{
		}
		break;

	}
}
*/