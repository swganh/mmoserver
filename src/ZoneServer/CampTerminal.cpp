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
#include "CampTerminal.h"
#include "Camp.h"
#include "CampRegion.h"
#include "PlayerObject.h"
#include "UIManager.h"
#include "ZoneServer/WorldManager.h"
#include "MessageLib/MessageLib.h"


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

	if(!playerObject || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead()|| playerObject->states.checkState(CreatureState_Combat))
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
        sprintf(text,"Owner: %s",region->getCampOwnerName().c_str());
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

