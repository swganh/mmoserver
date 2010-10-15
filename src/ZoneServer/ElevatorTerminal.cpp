/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include "ElevatorTerminal.h"
#include "CellObject.h"
#include "PlayerObject.h"
#include "ZoneServer/WorldManager.h"
#include "MessageLib/MessageLib.h"

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


        //gMessageLib->sendDataTransformWithParent053(playerObject);

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

        //gMessageLib->sendDataTransformWithParent053(playerObject);
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

