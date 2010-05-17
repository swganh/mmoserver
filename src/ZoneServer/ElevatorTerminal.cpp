/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "ElevatorTerminal.h"
#include "CellObject.h"
#include "PlayerObject.h"
#include "ZoneServer/WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"


//=============================================================================

ElevatorTerminal::ElevatorTerminal() : Terminal ()
{
	
}

//=============================================================================

ElevatorTerminal::~ElevatorTerminal()
{
}

//=============================================================================

void ElevatorTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject);

	if(!playerObject || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead())
	{
		return;
	}

	if(messageType == radId_elevatorUp)
	{
		gMessageLib->sendPlayClientEffectObjectMessage(gWorldManager->getClientEffect(mEffectUp),"",playerObject);

		
		// put him into new one
		playerObject->mDirection = mDstDirUp;
		playerObject->mPosition  = mDstPosUp;
		playerObject->setParentId(mDstCellUp);
		playerObject->updatePosition(mDstCellUp,mDstPosUp);

		
		gMessageLib->sendDataTransformWithParent(playerObject);

	}
	else if(messageType == radId_elevatorDown)
	{
		gMessageLib->sendPlayClientEffectObjectMessage(gWorldManager->getClientEffect(mEffectDown),"",playerObject);
	
		// remove player from current position, elevators can only be inside

		// put him into new one
		playerObject->mDirection = mDstDirDown;
		playerObject->mPosition  = mDstPosDown;
		playerObject->setParentId(mDstCellDown);

		playerObject->updatePosition(mDstCellDown,mDstPosDown);		
		
		gMessageLib->sendDataTransformWithParent(playerObject);
	}
	else
	{
		gLogger->log(LogManager::NOTICE,"ElevatorTerminal: Unhandled MenuSelect: %u",messageType);
	}
}

//=============================================================================

void ElevatorTerminal::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	RadialMenu* radial = new RadialMenu();
	
	if(mTanType == TanType_ElevatorUpTerminal)
	{
		radial->addItem(1,0,radId_examine,radAction_Default);
		radial->addItem(2,0,radId_elevatorUp,radAction_ObjCallback,"@elevator_text:up");
	}
	else if(mTanType == TanType_ElevatorDownTerminal)
	{
		radial->addItem(1,0,radId_examine,radAction_Default);
		radial->addItem(2,0,radId_elevatorDown,radAction_ObjCallback,"@elevator_text:down");
	}
	else
	{
		radial->addItem(1,0,radId_examine,radAction_Default);
		radial->addItem(2,0,radId_elevatorUp,radAction_ObjCallback,"@elevator_text:up");
		radial->addItem(3,0,radId_elevatorDown,radAction_ObjCallback,"@elevator_text:down");
	}

	mRadialMenu = RadialMenuPtr(radial);
}

//=============================================================================

