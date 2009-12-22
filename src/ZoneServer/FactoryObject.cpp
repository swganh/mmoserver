
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "ObjectFactory.h"
#include "FactoryObject.h"
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


//============================================================================
FactoryObject::FactoryObject() : PlayerStructure()
{
	mType = ObjType_Structure;
	
}

//=============================================================================

FactoryObject::~FactoryObject()
{

}




//=============================================================================
//handles the radial selection

void FactoryObject::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject);
	if(!player)
	{	
		gLogger->logMsgF("HarvesterObject::handleObjectMenuSelect::could not find player",MSG_HIGH);
		return;
	}
	
	switch(messageType)
	{
		case radId_StructureStatus:
		{
			StructureAsyncCommand command;
			command.Command = Structure_Command_ViewStatus;
			command.PlayerId = player->getId();
			command.StructureId = this->getId();

			gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

		}
		break;

		case radId_depositPower:
		{
			StructureAsyncCommand command;
			command.Command = Structure_Command_DepositPower;
			command.PlayerId = player->getId();
			command.StructureId = this->getId();

			gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

		}
		break;

		case radId_payMaintenance:
		{
			StructureAsyncCommand command;
			command.Command = Structure_Command_PayMaintenance;
			command.PlayerId = player->getId();
			command.StructureId = this->getId();

			gStructureManager->checkNameOnPermissionList(this->getId(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
			
		}
		break;

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
void FactoryObject::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
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
	
	radial->addItem(4,2,radId_serverTerminalManagementDestroy,radAction_ObjCallback,"Destroy Structure");//destroy
	radial->addItem(5,2,radId_StructureStatus,radAction_ObjCallback,"Status");//destroy
	radial->addItem(6,2,radId_payMaintenance,radAction_ObjCallback,"Pay Maintenance");//destroy
	radial->addItem(7,2,radId_setName,radAction_ObjCallback,"Set Name");//destroy
	radial->addItem(8,2,radId_operateHarvester,radAction_ObjCallback,"operate Harvester");//destroy
	radial->addItem(9,2,radId_depositPower,radAction_ObjCallback,"Deposit Power");//destroy
	
	
	radial->addItem(10,3,radId_serverTerminalPermissionsAdmin,radAction_ObjCallback,"Admin List");//destroy
	radial->addItem(11,3,radId_serverTerminalPermissionsHopper,radAction_ObjCallback,"Hopper List");//destroy
	


	RadialMenuPtr radialPtr(radial);
	mRadialMenu = radialPtr;
}



void FactoryObject::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	StructureManagerAsyncContainer* asynContainer = (StructureManagerAsyncContainer*)ref;

//	switch(asynContainer->mQueryType)
//	{

		
//		default:break;

//	}

	SAFE_DELETE(asynContainer);
}




