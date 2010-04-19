/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "CampTerminal.h"
#include "Camp.h"
#include "CampRegion.h"
#include "PlayerObject.h"
#include "UIManager.h"
#include "ZoneServer/WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"




//=============================================================================

CampTerminal::CampTerminal() : Terminal ()
{

}

//=============================================================================

CampTerminal::~CampTerminal()
{
}

//=============================================================================

void CampTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject);

	if(!playerObject || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead()|| playerObject->checkState(CreatureState_Combat))
	{
		return;
	}

	if(messageType == radId_serverTerminalManagementDestroy)
	{
		CampRegion* region = dynamic_cast<CampRegion*>(gWorldManager->getObjectById(this->mCampRegionId));
		region->despawnCamp();
		return;
	}

	if(messageType == radId_serverTerminalManagementStatus)
	{
		mAttributesMenu.clear();
		//Camp* camp = dynamic_cast<Camp*>(gWorldManager->getObjectById(this->mCampId));
		CampRegion* region = dynamic_cast<CampRegion*>(gWorldManager->getObjectById(this->mCampRegionId));

		int8 text[64];
		sprintf(text,"Owner: %s",region->getCampOwnerName().getAnsi());
		mAttributesMenu.push_back(text);

		uint32 time = static_cast<uint32>(region->getUpTime());
		uint32 hours = (uint32)time/3600;

		time -=(hours*3600);
		uint32 minutes = (uint32)(time)/60;

		time -=(minutes*60);
		uint32 seconds = (uint32)time;

		sprintf(text,"Up-Time: %u hours, %u minutes and %u seconds",hours, minutes, seconds);
		mAttributesMenu.push_back(text);

		sprintf(text,"Total Visitors: %u ", region->getVisitors());
		mAttributesMenu.push_back(text);

		sprintf(text,"Current Visitors: %u ", region->getCurrentVisitors());
		mAttributesMenu.push_back(text);

		//sprintf(text,"Healing Modifier: %f ", camp->getHealingModifier());
		sprintf(text,"Healing Modifier: 0.65 ");
		mAttributesMenu.push_back(text);

		gUIManager->createNewListBox(this,"handleMainMenu","Camp status","Below is a summary of the status of the camp.", mAttributesMenu,playerObject,SUI_Window_ListBox);

		return;
	}

	gLogger->logMsgF("CampTerminal: Unhandled MenuSelect: %u",MSG_HIGH,messageType);

}

//=============================================================================

void CampTerminal::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	RadialMenu* radial = new RadialMenu();


	radial->addItem(1,0,radId_examine,radAction_Default);
	radial->addItem(2,0,radId_serverTerminalManagementStatus,radAction_ObjCallback,"Status");
	
	Camp* camp = (Camp*) gWorldManager->getObjectById(this->mCampId);
	
	if(creatureObject->getId() == camp->getOwner())
	{
		radial->addItem(3,0,radId_serverTerminalManagementDestroy,radAction_ObjCallback,"Disband");
	}

	mRadialMenu = RadialMenuPtr(radial);

}

//=============================================================================

