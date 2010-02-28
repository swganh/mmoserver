/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "PlayerStructureTerminal.h"
#include "PlayerStructure.h"

#include "PlayerObject.h"
#include "TangibleObject.h"
#include "HouseObject.h"

#include "UIManager.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "MathLib/Quaternion.h"


//=============================================================================

PlayerStructureTerminal::PlayerStructureTerminal() : Terminal()
{
	mStructure = 0;

	
}

void PlayerStructureTerminal::prepareCustomRadialMenu(CreatureObject* player, uint8 itemCount)
{
	RadialMenu* radial	= new RadialMenu();

	// any object with callbacks needs to handle those (received with menuselect messages) !
	radial->addItem(1,0,radId_examine,radAction_Default,"");
	radial->addItem(2,0,radId_serverTerminalManagement,radAction_ObjCallback,"@player_structure:management");
	radial->addItem(3,0,radId_serverTerminalPermissions,radAction_ObjCallback, "@player_structure:permissions");

	//test if the caller is on the permission list

	//radial->addItem(4,2,radId_serverTerminalManagementDestroy,radAction_ObjCallback,"@player_structure:permission_destroy ");//destroy

	HouseObject* house = dynamic_cast<HouseObject*>(gWorldManager->getObjectById(this->getStructure()));
	if(house)
	{
		radial->addItem(4,2,radId_serverTerminalManagementDestroy,radAction_ObjCallback,"@player_structure:permission_destroy ");//destroy
		radial->addItem(5,2,radId_serverTerminalManagementStatus,radAction_ObjCallback,"@player_structure:management_status");
		radial->addItem(6,2,radId_serverTerminalManagementPay,radAction_ObjCallback,"@player_structure:management_pay");
		radial->addItem(7,2,radId_serverTerminalManagementPrivacy,radAction_ObjCallback,"@player_structure:management_privacy");

		radial->addItem(8,3,radId_serverTerminalPermissionsAdmin,radAction_ObjCallback,"@player_structure:permission_admin");
		radial->addItem(9,3,radId_serverTerminalPermissionsBanned,radAction_ObjCallback,"@player_structure:permission_banned");
		radial->addItem(10,3,radId_serverTerminalPermissionsEnter,radAction_ObjCallback,"@player_structure:permission_enter");

		 
		

	}
  
	RadialMenuPtr radialPtr(radial);
	mRadialMenu = radialPtr;

}

//=============================================================================

PlayerStructureTerminal::~PlayerStructureTerminal()
{
	
}

//=============================================================================

void PlayerStructureTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	PlayerObject* player = (PlayerObject*)srcObject;

	if ((!player) ||(!player->isConnected()))
	{
		gLogger->logMsgF("HarvesterObject::handleObjectMenuSelect::could not find player",MSG_HIGH);
		return;
	}
	
	switch(messageType)
	{
		case radId_serverTerminalPermissionsBanned:
		{
			StructureAsyncCommand command;
			command.Command = Structure_Command_PermissionBan;
			command.PlayerId = player->getId();
			command.StructureId = this->getStructure();

			gStructureManager->checkNameOnPermissionList(this->getStructure(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
		}
		break;

		case radId_serverTerminalPermissionsEnter:
		{
			StructureAsyncCommand command;
			command.Command = Structure_Command_PermissionEntry;
			command.PlayerId = player->getId();
			command.StructureId = this->getStructure();

			gStructureManager->checkNameOnPermissionList(this->getStructure(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
		}
		break;

		case radId_serverTerminalPermissionsAdmin:
		{
			StructureAsyncCommand command;
			command.Command = Structure_Command_PermissionAdmin;
			command.PlayerId = player->getId();
			command.StructureId = this->getStructure();

			gStructureManager->checkNameOnPermissionList(this->getStructure(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
		}
		break;

		case radId_serverTerminalManagementPrivacy:
		{
			StructureAsyncCommand command;
			command.Command = Structure_Command_Privacy;
			command.PlayerId = player->getId();
			command.StructureId = this->getStructure();

			gStructureManager->checkNameOnPermissionList(this->getStructure(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
		}
		break;

		case radId_serverTerminalManagementDestroy: 
		{
			StructureAsyncCommand command;
			command.Command = Structure_Command_Destroy;
			command.PlayerId = player->getId();
			command.StructureId = this->getStructure();

			gStructureManager->checkNameOnPermissionList(this->getStructure(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
			
		}
		break;

		case radId_serverTerminalManagementStatus:
		{
			StructureAsyncCommand command;
			command.Command = Structure_Command_ViewStatus;
			command.PlayerId = player->getId();
			command.StructureId = this->getStructure();

			gStructureManager->checkNameOnPermissionList(this->getStructure(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);

			
		}
		break;

		case radId_serverTerminalManagementPay:
		{
			StructureAsyncCommand command;
			command.Command = Structure_Command_PayMaintenance;
			command.PlayerId = player->getId();
			command.StructureId = this->getStructure();

			gStructureManager->checkNameOnPermissionList(this->getStructure(),player->getId(),player->getFirstName().getAnsi(),"ADMIN",command);
			
		}
		break;


		default:
			break;
	}

}


//=============================================================================

void PlayerStructureTerminal::handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window)
{
	// gLogger->logMsgF("InsuranceTerminal::handleUIEvent You are here!",MSG_NORMAL);

	if(window == NULL)
	{
		return;
	}

	PlayerObject* playerObject = window->getOwner(); // window owner

	if(playerObject == NULL || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead())
	{
		return;
	}
   /*
	switch(window->getWindowType())
	{
		
	

	

	

		default:
		{
		}
		break;
	}
	 */
	// gLogger->logMsgF("CloningTerminal::handleUIEvent You sure handled this UI-event!, Action = %d",MSG_NORMAL, action);
}
