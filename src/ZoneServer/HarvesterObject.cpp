
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "HarvesterObject.h"
#include "UIManager.h"
#include "PlayerObject.h"


//=============================================================================

HarvesterObject::HarvesterObject() : PlayerStructure()
{
	mType = ObjType_Harvester;
	
}

//=============================================================================

HarvesterObject::~HarvesterObject()
{

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
			gUIManager->createNewStructureDestroyBox(this,player, this, true);
		}
	}
}

void HarvesterObject::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(creatureObject);
	if(!player)
	{	
		gLogger->logMsgF("HarvesterObject::handleObjectMenuSelect::could not find player",MSG_HIGH);
		return;
	}

	if(this->getOwner() != player->getPlayerObjId())
	{
		gLogger->logMsgF("HarvesterObject::handleObjectMenuSelect::No Owner",MSG_HIGH);
		return;
	}
	
	RadialMenu* radial	= new RadialMenu();
			
	radial->addItem(1,0,radId_examine,radAction_Default,"");
	radial->addItem(2,0,radId_serverHarvesterManage,radAction_ObjCallback,"Structure Management");
	radial->addItem(3,0,radId_serverTerminalManagement,radAction_ObjCallback,"Structure Permissions");
	
	radial->addItem(4,2,radId_serverTerminalManagementDestroy,radAction_ObjCallback,"destroy");//destroy


	RadialMenuPtr radialPtr(radial);
	mRadialMenu = radialPtr;
}


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